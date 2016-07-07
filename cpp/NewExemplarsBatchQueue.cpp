
#include "NewExemplarsBatchQueue.h"

NewExemplarsBatchQueue::NewExemplarsBatchQueue()
{

}

void NewExemplarsBatchQueue::enqueue(vector<Spotting>* batch)
{
    lock();
    for (Spotting& s : *batch)
        queue.push(s);
    unlock();
}

NewExemplarsBatch* NewExemplarsBatchQueue::dequeue(int batchSize, unsigned int maxWidth, int color)
{
    lock();
    NewExemplarsBatch* ret=NULL;
    if (queue.size()>0)
    {
        vector<Spotting> batch;
        for (int i=0; i<batchSize && queue.size()>0; i++)
        {
           batch.push_back(queue.top());
           queue.pop();
        
        }
        ret = new NewExemplarsBatch(&batch, maxWidth, color);
        returnMap[ret->getId()]=ret;
        timeMap[ret->getId()]=chrono::system_clock::now();
    }
    unlock();
    return ret;
}

vector<Spotting> NewExemplarsBatchQueue::feedback(unsigned long id, const vector<int>& userClassifications, vector<pair<unsigned long, string> >* toRemoveExemplars)
{
    vector<Spotting> confirmedNgramExemplars;
    lock();
    if (returnMap.find(id) != returnMap.end())
    {
        if (userClassifications.size() == returnMap[id]->size())
        {
            for (int i=0; i<userClassifications.size(); i++)
            {
                returnMap[id][i].classified=userClassifications[i];
                if (userClassifications[i]>0)
                    confirmedNgramExemplars.emplace(returnMap[id][i].ngram,returnMap[id][i].ngramImg());
                else if (userClassifications[i]<0) //PASS
                {
                    queue.push(Spotting(returnMap[id][i]));
                }
            }
            //delete returnMap[id];
            doneMap[id]=returnMap[id];
            timeDoneMap[id]=chrono::system_clock::now();
        }
        else //Something went wrong...
        {
            cout <<"ERROR: mismatch size between user data ("<<userClassifications.size()<<") and newExemplars length ("<<returnMap[id]->size()<<"). Requeing."<<endl;
            for (int i=0; i<returnMap[id].size(); i++)
                queue.push(Spotting(returnMap[id][i]));
        }
        returnMap.erase(id);
        timeMap.erase(id);
    }
    else
    {
        //This occurs on a resend
        if (doneMap.find(id) != doneMap.end())
        {
            if (userClassifications.size() == returnMap[id].size())
            {
                for (int i=0; i<userClassifications.size(); i++)
                {
                    if (doneMap[id][i].classified!=-1 && doneMap[id][i].classified!=userClassifications[i])
                    {
                        if (userClassifications[i]>0)
                            confirmedNgramExemplars.emplace(Spotting(returnMap[id][i]);
                        else if (userClassifications[i]==0)
                            toRemoveExemplars->push_back(make_pair(doneMap[id][i].id,doneMap[id][i].ngram));
                    }
                }
            }
            else
                cout <<"ERROR: Resent new ex batch has size mismatch. user data ("<<userClassifications.size()<<") and newExemplars length ("<<returnMap[id]->size()<<")"<<endl;
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
            queue.push(returnMap[id]);

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
