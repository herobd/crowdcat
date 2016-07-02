#include "Spotter.h"

Spotter::Spotter(MasterQueue* masterQueue, const Corpus* corpus, string modelDir, int numThreads)
{
    this->masterQueue=masterQueue;
    this->corpus=corpus;
    //TODO init spotting implementations from modelDir
    
    int _setId=0;
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
            vector<Spotting> results;
            //TODO run query
            delete query;
            masterQueue.updateSpottings(results);
        }
    }
}

void Spotter::addQuery(vector<Spotting> exemplars)
{
    string ngram = exemplars[0].ngram;
    int setId = ++_setId;
    mutLock.lock();
    for (Spotting exemplar : exemplars)
    {
        assert(ngram.compare(exemplar.ngram)==0);
        SpottingQuery query = new SpottingQuery(exemplar);
        enqueue(query);
        sem_post(semLock);

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
