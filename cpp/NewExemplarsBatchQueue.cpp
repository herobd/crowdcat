
#include "NewExemplarsBatchQueue.h"
unsigned long NewExemplarsBatch::_batchId=0;
NewExemplarsBatchQueue::NewExemplarsBatchQueue()
{

}

void NewExemplarsBatchQueue::enqueue(const vector<Spotting*>& batch, vector<pair<unsigned long, string> >* toRemoveExemplars)
{
    if (batch.size()>0 || toRemoveExemplars->size()>0)
    {
        lock();
        if (toRemoveExemplars->size()>0)
        {
            priority_queue<Spotting*, vector<Spotting*>, pcomparison> temp;
            while (queue.size()>0)
            {
                Spotting* s = queue.top();
                queue.pop();
                bool matched=false;
                for (auto iter=toRemoveExemplars->begin(); iter!=toRemoveExemplars->end(); iter++)
                {
                    if (s->id == iter->first)
                    {
                        matched=true;
                        iter=toRemoveExemplars->erase(iter);
                        break;
                    }
                }
                if (!matched)
                    temp.push(s);
            }
            queue.swap(temp);

        }
        for (Spotting* s : batch)
            queue.push(s);
        unlock();
    }
}

void NewExemplarsBatchQueue::remove(unsigned long id)
{
    priority_queue<Spotting*, vector<Spotting*>, pcomparison> temp;
    while (queue.size()>0)
    {
        Spotting* s = queue.top();
        queue.pop();
        if (s->id != id)
            temp.push(s);
#ifdef TEST_MODE_LONG
        else
            cout <<"NewExemplarsQueue removed "<<id<<":"<<s->ngram<<endl;
#endif
    }
    queue.swap(temp);
}

NewExemplarsBatch* NewExemplarsBatchQueue::dequeue(int batchSize, unsigned int maxWidth, int color)
{
    lock();
    NewExemplarsBatch* ret=NULL;
    if (queue.size()>0)
    {
        vector<Spotting*> batch;
        for (int i=0; i<batchSize && queue.size()>0; i++)
        {
           batch.push_back(queue.top());
           queue.pop();
        
        }
        ret = new NewExemplarsBatch(batch, maxWidth, color);
        returnMap[ret->getId()]=ret;
        timeMap[ret->getId()]=chrono::system_clock::now();
    }
    unlock();
    return ret;
}

vector<SpottingExemplar*> NewExemplarsBatchQueue::feedback(unsigned long id, const vector<int>& userClassifications, vector<pair<unsigned long, string> >* toRemoveExemplars)
{
    vector<SpottingExemplar*> confirmedNgramExemplars;
    lock();
    if (returnMap.find(id) != returnMap.end())
    {
        if (userClassifications.size() == returnMap[id]->size())
        {
            for (int i=0; i<userClassifications.size(); i++)
            {
                returnMap[id]->at(i).classified=userClassifications[i];
                if (userClassifications[i]>0)
                    confirmedNgramExemplars.push_back(new SpottingExemplar(returnMap[id]->at(i)));
                else if (userClassifications[i]<0) //PASS
                {
                    queue.push(new SpottingExemplar(returnMap[id]->at(i)));
                }
            }
            //delete returnMap[id];
            doneMap[id]=returnMap[id];
            timeDoneMap[id]=chrono::system_clock::now();
        }
        else //Something went wrong...
        {
            cout <<"ERROR: mismatch size between user data ("<<userClassifications.size()<<") and newExemplars length ("<<returnMap[id]->size()<<"). Requeing."<<endl;
            for (int i=0; i<returnMap[id]->size(); i++)
                queue.push(new SpottingExemplar(returnMap[id]->at(i)));
            delete returnMap[id];
        }
        returnMap.erase(id);
        timeMap.erase(id);
    }
    else
    {
        //This occurs on a resend
        if (doneMap.find(id) != doneMap.end())
        {
            if (userClassifications.size() == doneMap[id]->size())
            {
                for (int i=0; i<userClassifications.size(); i++)
                {
                    if (userClassifications[i]!=-1 && doneMap[id]->at(i).classified!=userClassifications[i])
                    {
#ifdef TEST_MODE_LONG
                        cout<<"resend new exs:"<<doneMap[id]->at(i).ngram<<" was:"<<doneMap[id]->at(i).classified<<" now:"<<userClassifications[i]<<endl;
#endif
                        if (doneMap[id]->at(i).classified==-1)
                            remove(doneMap[id]->at(i).id);
                        if (userClassifications[i]>0)
                            confirmedNgramExemplars.push_back(new SpottingExemplar(doneMap[id]->at(i)));
                        else if (userClassifications[i]==0 && doneMap[id]->at(i).classified!=-1)
                            toRemoveExemplars->push_back(make_pair(doneMap[id]->at(i).id,doneMap[id]->at(i).ngram));
                        doneMap[id]->at(i).classified=userClassifications[i];
                    }
                        

                }
            }
            else
                cout <<"ERROR: Resent new ex batch has size mismatch. user data ("<<userClassifications.size()<<") and newExemplars length ("<<doneMap[id]->size()<<")"<<endl;
        }
        else
        {
            cout<<"ERROR: Getting old resend of New Ex Batch"<<endl;
        }
    }
    unlock();
    return confirmedNgramExemplars;
}

void NewExemplarsBatchQueue::checkIncomplete()
{
    lock();
    for (auto iter = timeMap.begin(); iter!=timeMap.end(); iter++)
    {
        auto start = *iter;
        unsigned long id = start.first;
        chrono::system_clock::duration d = chrono::system_clock::now()-start.second;
        chrono::minutes pass = chrono::duration_cast<chrono::minutes> (d);
        if (pass.count() > 20) //if 20 mins has passed
        {
            for (int i=0; i<returnMap[id]->size(); i++)
                queue.push(new SpottingExemplar(returnMap[id]->at(i)));
            //queue.push(returnMap[id]);
            delete returnMap[id];
            returnMap.erase(id);
            iter=timeMap.erase(iter);
            iter--;
        }
    }
    for (auto iter = timeDoneMap.begin(); iter!=timeDoneMap.end(); iter++)
    {
        auto start = *iter;
        unsigned long id = start.first;
        chrono::system_clock::duration d = chrono::system_clock::now()-start.second;
        chrono::minutes pass = chrono::duration_cast<chrono::minutes> (d);
        if (pass.count() > 20) //if 20 mins has passed
        {
            delete doneMap[id];
            doneMap.erase(id);
            iter=timeDoneMap.erase(iter);
            iter--;
        }
    }
    unlock();
}
