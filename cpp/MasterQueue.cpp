#include "MasterQueue.h"


void MasterQueue::checkIncomplete()
{
    queueLock.lock();
    for (auto iter=timeMap.begin(); iter!=timeMap.end(); iter++)
    {
        unsigned long id = iter->first;
        chrono::system_clock::duration d = chrono::system_clock::now()-iter->second;
        chrono::minutes pass = chrono::duration_cast<chrono::minutes> (d);
        if (pass.count() > 20) //if 20 mins has passed
        {
            wordsByScore.emplace(words->word(id)->topScore(),words->word(id));

            iter = timeMap.erase(iter);
            if (iter!=timeMap.begin())
                iter--;

            if (iter==timeMap.end())
                break;
        }
    }
    queueLock.unlock();
}

MasterQueue::MasterQueue(CorpusDataset* words, int contextPad) : words(words), contextPad(contextPad)
{
    finish.store(false);

    //pruningNet = net_load (net_filename);
}


BatchWraper* MasterQueue::getBatch(string userId, unsigned int maxWidth)
{

    BatchWraper* ret=NULL;
    queueLock.lock();
    if (wordsByScore.size()>0)
    {
        auto iter = wordsByScore.begin();
        while(1)
        {
            if (iter->second->userOK(userId))
                break;
            iter++;
            if (iter==wordsByScore.end())
            {
                iter= wordsByScore.begin();
                break;
            }
        }
        Word* word = iter->second;
        wordsByScore.erase(iter);
        ret = new BatchWraperTranscription(word,maxWidth,contextPad,state==CLEAN_UP);
        timeMap[word->getId()]=chrono::system_clock::now();

        if (wordsByScore.size()==0)
        {
            if (state==TOP_RESULTS)
            {
                //???
                //Wait for spotter/transcriber to stop, enqueue undoneWords.
                state=PAUSED;
            }
            else if (state==REMAINDER)
            {
                //enqueue all not done words
                state=CLEAN_UP;
                for (int i=0; i<words->size(); i++)
                {
                    if (!words->word(i)->isDone() && timeMap.find(i)==timeMap.end())
                    {
                        wordsByScore.emplace(words->word(i)->topScore(),words->word(i));
                    }
                }


                    
            }
            else if (state==REMAINDER)
            {
                state=DONE;
            }
        }
    }
    else if (state==PAUSED)
        ret = new BatchWraperPaused();
    else if (state==DONE)
        ret = new BatchWraperDone();
    

    queueLock.unlock();
    return ret;
} 




void MasterQueue::transcriptionFeedback(string userId, unsigned long id, string transcription) 
{
    Word* word = words->word(id);
    if (word!=NULL)
    {
        if (transcription.find('\n') != string::npos)
            transcription="$PASS$";
        queueLock.lock();
        timeMap.erase(word->getId());
        if (transcription[0]=='$')
        {
            if (transcription.compare("$PASS$")==0)
            {
                word->banUser(userId);
                wordsByScore.emplace(word->topScore(), word);
                queueLock.unlock();
            }
            else if (transcription.compare("$NONE$")==0)
            {
                if (state!=CLEAN_UP)
                {
                    queueLock.unlock();
                    //pthread_rwlock_wrlock(&undoneLock);
                    //undoneWords.insert(word);
                    //pthread_rwlock_unlock(&undoneLock);
                }
                else
                {
                    word->banUser(userId);
                    wordsByScore.emplace(word->topScore(), word);
                    queueLock.unlock();
                    //??
                    //word->setTrans(userId,"$ERROR-NONE$");
                }
            }
            else if (transcription.compare("$ERROR$")==0)
            {
                    queueLock.unlock();
                    //??
                    word->setTrans(userId,"$ERROR$");
            }
        }
        else
        {
            queueLock.unlock();
            word->setTrans(userId,transcription);
        }
    }
    else
    {
        cout<<"WARNING (MasterQueue::transcriptionFeedback): Unrecognized word id("<<id<<"), trans: "<<transcription<<endl;
    }
}

void MasterQueue::setTranscriptions()
{
    queueLock.lock();
    for (int i=0; i<words->size(); i++)
    {
        Word* word = words->word(i);
        if (goodEnough(word))
        {
            wordsByScore.emplace(word->topScore(), word);
        }
    }
    state=TOP_RESULTS;
    queueLock.unlock();
}

bool MasterQueue::goodEnough(Word* word)
{
    //TODO Train a network to look at top 10 scores and predict if correct in top 5
    //network doesn't seem to work

    //float input[10];
    //float output[1];
    //vector<float> scores = word->peakTopN(10);
    //net_compute (pruningNet, &scores[0], output);
    //return output>PRUNING_THRESH;
    return true;
}


void MasterQueue::save(ofstream& out)
{

    queueLock.lock();
    out<<"MASTERQUEUE"<<endl;
    out<<wordsByScore.size()<<"\n";
    for (auto p : wordsByScore)
    {
        //out<<p.first<<"\n";
        out<<p.second->getId()<<"\n";
    }
    queueLock.unlock();


    out<<finish.load()<<"\n";
    out<<contextPad<<"\n";
    out<<(int)state<<endl;
}
MasterQueue::MasterQueue(ifstream& in, CorpusDataset* words) : words(words)
{
    finish.store(false);

    string line;
    getline(in,line);
    assert(line.compare("MASTERQUEUE")==0);
    getline(in,line);
    int wSize = stoi(line);
    for (int i=0; i<wSize; i++)
    {
        getline(in,line);
        unsigned long id = stoul(line);
        wordsByScore.emplace(words->word(id)->topScore(),words->word(id));
    }

    getline(in,line);
    finish.store(stoi(line));
 

    getline(in,line);
    contextPad = stoi(line);
    getline(in,line);
    state = (MasterQueueState) stoi(line);
}
