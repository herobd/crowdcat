#include "Spotter.h"
class Spotter;
Spotter::Spotter(MasterQueue* masterQueue, const Knowledge::Corpus* corpus, string modelDir)
{
    this->masterQueue=masterQueue;
    this->corpus=corpus;
    //TODO init spotting implementations from modelDir
    
    //int _setId=0;
    //cont.store(1);
    numThreads=0;
    sem_init(&semLock, 0, 0);
}
Spotter::~Spotter() {sem_destroy(&semLock);}

void Spotter::stop()
{
    //cont.store(0);
    for (int i=0; i<numThreads; i++)
        sem_post(&semLock);
}
void Spotter::run(int numThreads)
{
    this->numThreads=numThreads;
    //omp_set_num_threads(numThreads);
#pragma omp parallel num_threads(numThreads)
    {
        while(1)
        {
            sem_wait(&semLock);
            mutLock.lock();
            SpottingQuery* query = dequeue();
            if (query==NULL)
                break;
            mutLock.unlock();
            vector<Spotting>* results = runQuery(query);
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
            {
#ifdef TEST_MODE
                cout<<"sending to masterQ, "<<results->size()<<endl;
#endif
                masterQueue->updateSpottingResults(results);
            }
#ifdef TEST_MODE
            else
                cout<<"Successful mid-run cancel."<<endl;
#endif
        }
    }
}

void Spotter::addQueries(vector<SpottingExemplar*>& exemplars)
{
    //int setId = ++_setId;
    mutLock.lock();
    for (SpottingExemplar* exemplar : exemplars)
    {
        if (exemplar->type != SPOTTING_TYPE_THRESHED) //It's probably best not to entirely trust the threshed ones
        {
            SpottingQuery* query = new SpottingQuery(exemplar);
            enqueue(query);
            sem_post(&semLock);
        }
    }
    mutLock.unlock();
}
void Spotter::addQueries(vector<Spotting*>& exemplars)
{
    //int setId = ++_setId;
    mutLock.lock();
    for (Spotting* exemplar : exemplars)
    {
        if (exemplar->type != SPOTTING_TYPE_THRESHED) //It's probably best not to entirely trust the threshed ones
        {
            SpottingQuery* query = new SpottingQuery(exemplar);
            enqueue(query);
            sem_post(&semLock);
        }
    }
    mutLock.unlock();
}
void Spotter::addQueries(vector<Spotting>& exemplars)
{
    //int setId = ++_setId;
    mutLock.lock();
    for (Spotting& exemplar : exemplars)
    {
        if (exemplar.type != SPOTTING_TYPE_THRESHED) //It's probably best not to entirely trust the threshed ones
        {
            SpottingQuery* query = new SpottingQuery(exemplar);
            enqueue(query);
            sem_post(&semLock);
        }
    }
    mutLock.unlock();
}

void Spotter::removeQueries(vector<pair<unsigned long,string> >* toRemove)
{
#ifdef TEST_MODE
    cout <<"spotter removing query "<<toRemove->front().first<<":"<<toRemove->front().second<<"..."<<endl;
#endif
    mutLock.lock();
    for (auto r : *toRemove)
    {
        bool found=false;
        for (auto iter = ngramQueues[r.second].begin(); iter!=ngramQueues[r.second].end(); iter++)
        {
            if ((*iter)->getId() == r.first)
            {
#ifdef TEST_MODE
                cout<<"found in ngram queue"<<endl;
#endif
                sem_wait(&semLock);
                ngramQueues[r.second].erase(iter);
                found=true;
                break;
            }
        }
        if (!found)
        {
            for (auto iter=onDeck.begin(); iter!=onDeck.end(); iter++)
            {
#ifdef TEST_MODE
                cout<<"scanning onDeck: "<<(*iter)->getId()<<":"<<(*iter)->getNgram()<<endl;
#endif
                if ((*iter)->getId() == r.first)
                {
#ifdef TEST_MODE
                    cout<<"found in onDeck"<<endl;
#endif
                    sem_wait(&semLock);
                    onDeck.erase(iter);
                    found=true;
                    if (ngramQueues[r.second].size()>0)
                    {
                        onDeck.push_back(ngramQueues[r.second].front());
                        ngramQueues[r.second].pop_front();
                    }
                    break;
                }
            }
            if (!found)
            {
#ifdef TEST_MODE
                cout<<"not found, adding to emList"<<endl;
#endif
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
    if (onDeck.size()>0) //A null return should only occur when stopping.
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
}
