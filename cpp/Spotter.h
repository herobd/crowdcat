#ifndef SPOTTER_H
#define SPOTTER_H

#include "MasterQueue.h"
#include "Corpus.h"
#include <omp>
#include <atomic>
#include <map>
#include <deque>
#include <semaphore.h>

Spotter class
{
    public:
    Spotter(MasterQueue* masterQueue, const Corpus* corpus, string modelDir, int numThreads);

    void run(int numThreads);

    void addQuery(vector<Spotting> exemplars);

    void enqueue(SpottingQuery* q);

    SpottingQuery* dequeue();

    private:
    MasterQueue* masterQueue;
    const Corpus* corpus;
    atomic_char cont;

    semaphor semLock;
    mutex mutLock;

    //There are two queues, the onDeck on having on instance of each ngram, thus forcing rotations
    deque<SpottingQuery*> onDeck;
    map<string,deque<SpottingQuery*> > ngramQueues;
};

#endif
