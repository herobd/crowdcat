#include "MasterQueue.h"


void MasterQueue::checkIncomplete()
{
    queueLock.lock();
    pthread_rwlock_rdlock(&wordsLockLock);
    for (auto iter=timeMap.begin(); iter!=timeMap.end(); iter++)
    {
        unsigned long id = iter->first;
        chrono::system_clock::duration d = chrono::system_clock::now()-iter->second;
        chrono::minutes pass = chrono::duration_cast<chrono::minutes> (d);
        if (pass.count() > 20) //if 20 mins has passed
        {
            wordsByScore.insert(words.at(id)->score,words.at(id));

            iter = timeMap.erase(iter);
            if (iter!=timeMap.begin())
                iter--;

            if (iter==timeMap.end())
                break;
        }
    }
    pthread_rwlock_unlock(&wordsLockLock);
    queueLock.unlock();
}

MasterQueue::MasterQueue(int contextPad) : contextPad(contextPad)
{
    finish.store(false);
    pthread_rwlock_init(&wordsLockLock,NULL);


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
        }
    }
    queueLock.unlock();
    return ret;
} 




void MasterQueue::transcriptionFeedback(string userId, unsigned long id, string transcription, bool manual) 
{
    pthread_rwlock_rdlock(&wordsLock);
    auto iii = words.find(id);
    if (iii != words.end())
    {
        if (transcription.find('\n') != string::npos)
            transcription="$PASS$";
        Word* word = iii->second;
        queueLock.lock();
        timeMap.erase(word->getId());
        if (transcription[0]=='$')
        {
            if (transcription.compare("$PASS$")==0)
            {
                word->banUser(userId);
                wordsByScore.insert(word->topScore(), word);
                queueLock.unlock();
            }
            else if (transcription.compare("$ERROR$")==0)
            {
                if (state!=CLEAN_UP)
                {
                    queueLock.unlock();
                    pthread_rwlock_unlock(&wordsLock);
                    pthread_rwlock_wrlock(&wordsLock);
                    undoneWords.insert(word);
                }
                else
                {
                    queueLock.unlock();
                    //??
                    word->setTrans(userId,"$ERROR$");
                }
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
    pthread_rwlock_unlock(&wordsLock);
}


void MasterQueue::save(ofstream& out)
{
    //ofstream out(savePrefix);

    //This is a costly lockdown, but bad things might happen if the queue is changed between writing the two
    pthread_rwlock_rdlock(&semResults);
    pthread_rwlock_rdlock(&semResultsQueue);
    out<<"MASTERQUEUE"<<endl;
    out<<results.size()<<"\n";
    for (auto p : results)
    {
        out<<p.first<<"\n";
        sem_wait(p.second.first);
        p.second.second->save(out);
        sem_post(p.second.first);
    }

    out<<resultsQueue.size()<<"\n";
    for (auto p : resultsQueue)
    {
        out<<p.first<<"\n";
    }
    pthread_rwlock_unlock(&semResultsQueue);
    pthread_rwlock_unlock(&semResults);
        
    transcribeBatchQueue.save(out);
    newExemplarsBatchQueue.save(out);

    out<<finish.load()<<"\n";
    out<<numCTrue<<"\n"<<numCFalse<<"\n";
    out<<contextPad<<"\n";
    //out.close();    
}
MasterQueue::MasterQueue(ifstream& in, CorpusRef* corpusRef, PageRef* pageRef)
{
    finish.store(false);
    pthread_rwlock_init(&semResultsQueue,NULL);
    pthread_rwlock_init(&semResults,NULL);
#if ROTATE
    pthread_rwlock_init(&semRotate,NULL);
    testIter=0;	
    test_rotate=0;
#endif
    kill.store(false);
    
    //ifstream in(loadPrefix);

    string line;
    getline(in,line);
    assert(line.compare("MASTERQUEUE")==0);
    getline(in,line);
    int rSize = stoi(line);
    for (int i=0; i<rSize; i++)
    {
        getline(in,line);
        unsigned long id = stoul(line);
        //assert(id==i+1);
        SpottingResults* res = new SpottingResults(in,pageRef);
        assert(id==res->getId());
        sem_t* sem = new sem_t();
        sem_init(sem,false,1);
        auto p = make_pair(sem,res);
        results[res->getId()] = p;
    }
    getline(in,line);
    rSize = stoi(line);
    for (int i=0; i<rSize; i++)
    {
        getline(in,line);
        unsigned long id = stoul(line);
        resultsQueue[results[id].second->getId()] = results[id];
    }
    transcribeBatchQueue.load(in,corpusRef);
    newExemplarsBatchQueue.load(in,pageRef);

    getline(in,line);
    finish.store(stoi(line));
 
    getline(in,line);
    numCTrue = stoi(line);
    getline(in,line);
    numCFalse = stoi(line);

    getline(in,line);
    contextPad = stoi(line);
    //in.close();
#ifdef TEST_MODE
    forceNgram="";
#endif
}
