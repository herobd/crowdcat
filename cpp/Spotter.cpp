#include "Spotter.h"

Spotter::Spotter(MasterQueue* masterQueue, const Corpus* corpus, string modelDir, int numThreads)
{
    this->masterQueue=masterQueue;
    this->corpus=corpus;
    //TODO init spotting implementations from modelDir
    
    //int _setId=0;
    cont.store(1);
    //run(numThreads);
}

void Spotter::run(int numThreads)
{
    omp_set_num_threads(numThreads);
#pragma omp parallel
    {
        while(cont.load())
        {
            sem_wait(semLock);
            mutLock.lock();
            SpottingQuery* query = dequeue();
            mutLock.unlock();
            vector<Spotting*> results = runQuery(query);
            bool cont=true;
            emLock.lock();
            auto iter = emList.find(query->getId());
            if (iter !=emList.end()) //has the exemplar I used been revoked?
            {
                emList.erase(iter);
                cont=false;
            }
            emLock.unlock();
            if (cont)
                masterQueue.updateSpottings(results);
            delete query;
        }
    }
}

void Spotter::addQueries(vector<Spotting*> exemplars)
{
    //int setId = ++_setId;
    mutLock.lock();
    for (Spotting* exemplar : exemplars)
    {
        if (exemplar.type != SPOTTING_TYPE_THRESHED) //It's probably best not to entirely trust the threshed ones
        {
            SpottingQuery query = new SpottingQuery(exemplar);
            enqueue(query);
            sem_post(semLock);
        }
    }
    mutLock.unlock();
}

void Spotter::removeQueries(vector<pair<unsigned long,string> >* toRemove)
{
    mutLock.lock();
    for (auto r : *toRemove)
    {
        bool found=false;
        for (auto iter = ngramQueues[r.second].begin(); iter!=ngramQueues[r.second].end(); iter++)
        {
            if ((*iter)->getId() == r.first)
            {
                ngramQueues[r.second].erase(iter);
                found=true;
                break;
            }
        }
        if (!found)
        {
            for (auto iter=onDeck.begin(); iter!=onDeck.end(); iter++)
            {
                if ((*iter)->getId() == r.first)
                {
                    onDeck.erase(iter);
                    found=true;
                    if (ngramQueues[r.second].size()>0)
                    {
                        onDeck.push_back(ngramQueues[r.second()].front());
                        ngramQueues[r.second()].pop_front();
                    }
                    break;
                }
            }
            if (!found)
            {
                //Oh, dear! We've spotted or are spotting it.
                emLock.lock();
                emList.insert(r.first);
                emLock.unlock();
            }
        }

    }
    mutLock.unlock();
}


void Spotter::enqueue(SpottingQuery* q)
{
    bool isOther=false;
    for (SpottingQuery* oq : onDeck)
    {
        if (oq->getNgram().compare(q->getNgram())==0)
        {
            isOther=true;
            break;
        }
    }
    if (isOther)
    {
        ngramQueues[q->getNgram()].push_back(q);
    }
    else
    {
        onDeck.push_back(q);
    }
}

SpottingQuery* Spotter::dequeue()
{
    SpottingQuery* ret = NULL;
    if (onDeck.size>0) //This shouldn't ever be false due to the semaphore.
    {
        ret = onDeck.front();
        onDeck.pop_front();

        if (ngramQueues[ret->getNgram()].size()>0)
        {
            onDeck.push_back(ngramQueues[ret->getNgram()].front());
            ngramQueues[ret->getNgram()].pop_front();
        }
    }
    return ret;
