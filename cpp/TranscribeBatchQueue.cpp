#include "TranscribeBatchQueue.h"

TranscribeBatchQueue::TranscribeBatchQueue()
{
    contextPad=0;
}

/*void TranscribeBatchQueue::enqueue(TranscribeBatch* batch)
{
    lock();
    queue.push_back(batch);
    unlock();
}*/
void TranscribeBatchQueue::enqueueAll(vector<TranscribeBatch*> batches, vector<unsigned long>* remove)
{
    lock();
    for (TranscribeBatch* b :  batches)
    {
#if TRANS_DONT_WAIT
        bool foundL=false;
        for (int i=0; i<lowQueue.size(); i++)
        {
            if (b->getId() == lowQueue[i]->getId())
            {
                delete lowQueue[i];
                if (b->isLowPriority())
                    lowQueue[i] = b;
                else
                    lowQueue.erase(lowQueue.begin()+i);
                foundL=true;
                break;
            }
        }
        if (!foundL && b->isLowPriority())
            lowQueue.push_back(b);
        
#endif
        bool found=false;
        for (int i=0; i<queue.size(); i++)
        {
            if (b->getId() == queue[i]->getId())
            {
                delete queue[i];
#if TRANS_DONT_WAIT
                if (!b->isLowPriority())
                    queue[i] = b;
                else    
                    queue.erase(queue.begin()+i);
#else
                queue[i] = b;
#endif
                found=true;
                break;
            }
        }
#if TRANS_DONT_WAIT
        if (!found && !b->isLowPriority())
#else
        if (!found)
#endif
            queue.push_back(b);
    }
    if (remove)
    {
        for (unsigned long id :  *remove)
        {
            for (auto i=queue.begin(); i!=queue.end(); i++)
            {
                if (id == (*i)->getId())
                {
                    delete (*i);
                    queue.erase(i);
                    break;
                }
            }
        }
#if TRANS_DONT_WAIT
        for (unsigned long id :  *remove)
        {
            for (auto i=lowQueue.begin(); i!=lowQueue.end(); i++)
            {
                if (id == (*i)->getId())
                {
                    delete (*i);
                    lowQueue.erase(i);
                    break;
                }
            }
        }
#endif
    }
    unlock();
}

TranscribeBatch* TranscribeBatchQueue::dequeue(unsigned int maxWidth, bool need)
{
    lock();
    TranscribeBatch* ret=NULL;
    if (queue.size()>0)
    {
        ret = queue.front();
        assert(ret->getPossibilities().size()<1000);
        ret->setWidth(maxWidth,contextPad);
        queue.pop_front();

        returnMap[ret->getId()]=ret;
        timeMap[ret->getId()]=chrono::system_clock::now();
    }
#if TRANS_DONT_WAIT
    if (ret==NULL && need)
    {
        if (lowQueue.size()>0)
        {
            ret = lowQueue.front();
            assert(ret->getPossibilities().size()<1000);
            ret->setWidth(maxWidth,contextPad);
            lowQueue.pop_front();

            lowReturnMap[ret->getId()]=ret;
            lowTimeMap[ret->getId()]=chrono::system_clock::now();
        }
    }
#endif
    unlock();
    return ret;
}

void TranscribeBatchQueue::feedbackProcess(unsigned long id, string transcription, vector<pair<unsigned long, string> >* toRemoveExemplars, WordBackPointer* backPointer, bool resend, deque<TranscribeBatch*>& queue, map<unsigned long, TranscribeBatch*>& returnMap, map<unsigned long, chrono::system_clock::time_point>& timeMap, map<unsigned long, WordBackPointer*>& doneMap, vector<Spotting*>* newNgramExemplars)
{
    if ((transcription[0]=='$' && transcription[transcription.length()-1]=='$') || transcription.length()==0)
    {
        if (transcription.compare("$ERROR$")==0)
        {//This may occur with bad segmentation, but primarily just bad ngram alignments
            TranscribeBatch* newBatch = backPointer->error(id,resend,newNgramExemplars,toRemoveExemplars);//change into manual batch or remove spottings?
            if (newBatch!=NULL)
                queue.push_back(newBatch);
            if (!resend)
            {
                doneMap[id] = backPointer;
                delete returnMap[id];
            }
        }
        else if (transcription.length()>9 && transcription.substr(0,8).compare("$REMOVE:")==0)
        {
            unsigned long sid; 
            //try
            //{
                sid= stoul(transcription.substr(8,transcription.length()-9));

                TranscribeBatch* newBatch = backPointer->removeSpotting(sid,id,resend,newNgramExemplars,toRemoveExemplars);
                if (newBatch!=NULL)
                    queue.push_back(newBatch);
            //}
            //catch (const invalid_argument& ia)
            //{
            //    cout << "Assumed stoul() failed with: "<<transcription.substr(8,transcription.length()-9)<< " ("<<transcription<<")"<<endl;
            //    cout << ia.what() << endl;
            //    assert(false);
            //}
            if (!resend)
            {
                doneMap[id] = backPointer;
                delete returnMap[id];
            }
        }
        else if (!resend && (transcription.length()==0 || transcription.compare("$PASS$")==0))
        {
            queue.push_front(returnMap[id]);
        }
        /*else
        {
            cout << "invalid_argument TranscribeBatchQueue::feedback(#,"<<transcription<<")"<<endl;
            if (!resend)
                queue.push_front(returnMap[id]);
        }*/
    }
}

vector<Spotting*> TranscribeBatchQueue::feedback(unsigned long id, string transcription, vector<pair<unsigned long, string> >* toRemoveExemplars)
{
    vector<Spotting*> newNgramExemplars;
    lock();
    WordBackPointer* backPointer=NULL;
    bool resend=false;
    if (returnMap.find(id) != returnMap.end())
    {
        backPointer=returnMap[id]->getBackPointer();
    }
    else if (doneMap.find(id) != doneMap.end())
    {
        //This occurs on a resend
        backPointer=doneMap[id];
        resend=true;
        //if (transcription.length()!=0 && transcription.compare("$PASS$")!=0)
        //    newNgramExemplars=doneMap[id]->result(transcription,toRemoveExemplars);
    }

    if (backPointer!=NULL)
    {
        feedbackProcess(id,transcription,toRemoveExemplars, backPointer, resend, queue, returnMap, timeMap, doneMap, &newNgramExemplars);
    }
#if TRANS_DONT_WAIT
    else
    {
        if (lowReturnMap.find(id) != lowReturnMap.end())
        {
            backPointer=lowReturnMap[id]->getBackPointer();
        }
        else if (lowDoneMap.find(id) != lowDoneMap.end())
        {
            //This occurs on a resend
            backPointer=lowDoneMap[id];
            resend=true;
            //if (transcription.length()!=0 && transcription.compare("$PASS$")!=0)
            //    newNgramExemplars=doneMap[id]->result(transcription,toRemoveExemplars);
        }

        if (backPointer!=NULL)
        {
            feedbackProcess(id,transcription,toRemoveExemplars, backPointer, resend, lowQueue, lowReturnMap, lowTimeMap, lowDoneMap, &newNgramExemplars);
        }
        else
            cout <<"ERROR: TranscribeBatchQueue::feedback unrecogized id: "<<id<<"   for trans: "<<transcription<<endl;
    }
#else
    else
    {
        cout <<"ERROR: TranscribeBatchQueue::feedback unrecogized id: "<<id<<"   for trans: "<<transcription<<endl;
#ifdef TEST_MODE
//        assert(false);
#endif
    }
#endif
    unlock();
    return newNgramExemplars;
}

void TranscribeBatchQueue::checkIncomplete()
{
    lock();
    for (auto iter=timeMap.begin(); iter!=timeMap.end(); iter++)
    {
        unsigned long id = iter->first;
        chrono::system_clock::duration d = chrono::system_clock::now()-iter->second;
        chrono::minutes pass = chrono::duration_cast<chrono::minutes> (d);
        if (pass.count() > 20) //if 20 mins has passed
        {
            queue.push_front(returnMap[id]);

            returnMap.erase(id);
            iter = timeMap.erase(iter);
            if (iter!=timeMap.begin())
                iter--;

            if (iter==timeMap.end())
                break;
        }
    }
#if TRANS_DONT_WAIT
    for (auto iter=lowTimeMap.begin(); iter!=lowTimeMap.end(); iter++)
    {
        unsigned long id = iter->first;
        chrono::system_clock::duration d = chrono::system_clock::now()-iter->second;
        chrono::minutes pass = chrono::duration_cast<chrono::minutes> (d);
        if (pass.count() > 20) //if 20 mins has passed
        {
            lowQueue.push_front(lowReturnMap[id]);

            lowReturnMap.erase(id);
            iter = lowTimeMap.erase(iter);
            if (iter!=lowTimeMap.begin())
                iter--;

            if (iter==lowTimeMap.end())
                break;
        }
    }
#endif
    unlock();
}

void TranscribeBatchQueue::save(ofstream& out)
{
    out<<"TRANSCRIBEBATCHQUEUE"<<endl;
    lock();
    //shortcut by saving the returnMap as part of the queue
    out<<(queue.size()+returnMap.size())<<"\n";
    for (auto p : returnMap) //returnmap goes first
    {
        p.second->save(out);
    }
    for (TranscribeBatch* t : queue)
    {
        t->save(out);
    }
    out<<contextPad<<"\n";
#if TRANS_DONT_WAIT
    out<<"LOWPRIORITY"<<endl;
    out<<(lowQueue.size()+lowReturnMap.size())<<"\n";
    for (auto p : lowReturnMap) //returnmap goes first
    {
        p.second->save(out);
    }
    for (TranscribeBatch* t : lowQueue)
    {
        t->save(out);
    }
#endif
    //we skip the doneMap as it is only used on resends
    unlock();
}
void TranscribeBatchQueue::load(ifstream& in, CorpusRef* corpusRef)
{
    string line;
    getline(in,line);
    assert(line.compare("TRANSCRIBEBATCHQUEUE")==0);
    getline(in,line);
    int qSize = stoi(line);
    for (int i=0; i<qSize; i++)
    {
        queue.push_back(new TranscribeBatch(in,corpusRef));
    }
    getline(in,line);
    contextPad = stoi(line);
#if TRANS_DONT_WAIT
    getline(in,line);
    assert(line.compare("LOWPRIORITY")==0);
    getline(in,line);
    qSize = stoi(line);
    for (int i=0; i<qSize; i++)
    {
        lowQueue.push_back(new TranscribeBatch(in,corpusRef));
    }
#endif
}
