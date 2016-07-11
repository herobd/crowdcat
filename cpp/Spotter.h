#ifndef SPOTTER_H
#define SPOTTER_H

#include "MasterQueue.h"
#include "Corpus.h"
#include <omp>
#include <atomic>
#include <map>
#include <list>
#include <semaphore.h>

SpottingQuery class
{
    public:
    SpottingQuery(const Spotting* e) : id(e->id), ngram(e->ngram) {}//use e->ngramImg() to get correct exemplar image}

    private:
    string ngram;
    unsigned long id;
};

Spotter class
{
    public:
    Spotter(MasterQueue* masterQueue, const Corpus* corpus, string modelDir, int numThreads);

    void run(int numThreads);

    void addQueries(vector<Spotting*>& exemplars);

    virtual vector<Spotting> runQuery(SpottingQuery* query);

    private:
    MasterQueue* masterQueue;
    const Corpus* corpus;
    atomic_char cont;

    semaphor semLock;
    mutex mutLock;
    mutex emLock;//"emergency" lock

    //There are two queues, the onDeck on having on instance of each ngram, thus forcing rotations
    list<SpottingQuery*> onDeck;
    map<string,list<SpottingQuery*> > ngramQueues;
    set<unsigned long> emList; //List of "emergency" halts to cancel a spotting that's in progress
    
    void enqueue(SpottingQuery* q);

    SpottingQuery* dequeue();
};

#endif
