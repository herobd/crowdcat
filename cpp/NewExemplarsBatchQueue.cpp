
#include "NewExemplarsBatchQueue.h"
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
            /*priority_queue<Spotting*, vector<Spotting*>, pcomparison> temp;
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
            queue.swap(temp);*/
            
            auto iter=queue.begin();
            while (iter!= queue.end())
            {
                bool matched=false;
                auto iter2=toRemoveExemplars->begin();
                while(iter2!=toRemoveExemplars->end())
                {
                    if ((*iter)->id == iter2->first)
                    {
                        matched=true;
                        iter=queue.erase(iter);
                        iter2=toRemoveExemplars->erase(iter2);    
                        break;
                    }
                    else
                        iter2++;
                }
                if (!matched)
                    iter++;
            }
                    
        }
        for (Spotting* s : batch) 
        {
            assert(s->ngramImg().cols>0);
            queue.insert(s);
        }
        unlock();
    }
}

void NewExemplarsBatchQueue::remove(unsigned long id)
{
    /*priority_queue<Spotting*, vector<Spotting*>, pcomparison> temp;
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
    queue.swap(temp);*/
    auto iter = queue.begin();
    while (iter!= queue.end())
        if ((*iter)->id == id)
        {
            queue.erase(iter);
            break;
        }
}

NewExemplarsBatch* NewExemplarsBatchQueue::dequeue(int batchSize, unsigned int maxWidth, int color, bool any)
{
#ifdef NO_EXEMPLARS
    return NULL;///////////////////
#endif
    lock();
    NewExemplarsBatch* ret=NULL;
    if (queue.size()>0)
    {
        vector<Spotting*> batch;
        for (int i=0; i<batchSize && queue.size()>0; i++)
        {
           auto iter = queue.begin();
           //batch.push_back(queue.top());
           //queue.pop();
           bool none=true;
           while (iter!=queue.end())
           {
               if (any || need[(*iter)->ngram])
               {
                   batch.push_back(*iter);
                   queue.erase(iter);
                   none=false;
                   break;
               }
               iter++;
           }
           if (none)
               break;
        
        }

        if (batch.size()>0)
        {
            ret = new NewExemplarsBatch(batch, maxWidth, color);
            //assert(ret->at(0).ngramImg().cols>0);
            returnMap[ret->getId()]=ret;
            timeMap[ret->getId()]=chrono::system_clock::now();
        }
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
                {
                    need[returnMap[id]->at(i).ngram]=false;
                    confirmedNgramExemplars.push_back(new SpottingExemplar(returnMap[id]->at(i)));
                }
                else if (userClassifications[i]<0) //PASS
                {
                    queue.insert(new SpottingExemplar(returnMap[id]->at(i)));
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
                queue.insert(new SpottingExemplar(returnMap[id]->at(i)));
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
                        if (userClassifications[i]>0) {
                            need[doneMap[id]->at(i).ngram]=false;
                            confirmedNgramExemplars.push_back(new SpottingExemplar(doneMap[id]->at(i)));
                        }
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
    //for (SpottingExemplar* se : confirmedNgramExemplars)
    //    need[se->ngram]=false;
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
                queue.insert(new SpottingExemplar(returnMap[id]->at(i)));
            //queue.push(returnMap[id]);
            delete returnMap[id];
            returnMap.erase(id);
            iter=timeMap.erase(iter);
            if (iter!=timeMap.begin())
                iter--;
            if (iter==timeMap.end())
                break;
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
            if (iter!=timeDoneMap.begin())
                iter--;
            if (iter==timeDoneMap.end())
                break;
        }
    }
    unlock();
}

void NewExemplarsBatchQueue::needExemplar(string ngram)
{
    lock();
    need[ngram]=true;
    unlock();
}

void NewExemplarsBatchQueue::save(ofstream& out)
{
    lock();
    int returnMapSize = 0;
    for (auto p : returnMap)
    {
        returnMapSize+=p.second->size();
    }

    out<<(queue.size()+returnMapSize)<<"\n";
    for (auto p : returnMap)
    {
        for (int i=0; i<p.second->size(); i++)
            SpottingExemplar(p.second->at(i)).save(out);
    }
    for (Spotting* s : queue)
    {
        s->save(out);
    }

    out<<need.size()<<"\n";
    for (auto p : need)
    {
        out<<p.first<<"\n"<<p.second<<"\n";
    }
    unlock();
}
void NewExemplarsBatchQueue::load(ifstream& in, PageRef* pageRef)
{
    string line;
    getline(in,line);
    int qSize = stoi(line);
    for (int i=0; i<qSize; i++)
    {
        queue.insert(new SpottingExemplar(in,pageRef));
    }
    getline(in,line);
    int nSize=stoi(line);
    for (int i=0; i<nSize; i++)
    {
        string ngram;
        getline(in, ngram);
        getline(in, line);
        bool n=stoi(line);
        need[ngram]=n;
    }
}

