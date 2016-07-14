#ifndef SPOTTER_H
#define SPOTTER_H

#include "MasterQueue.h"
#include "Knowledge.h"
#include <atomic>
#include <map>
#include <list>
#include <semaphore.h>

using namespace std;

class SpottingQuery 
{
    public:
    SpottingQuery(const Spotting* e) : id(e->id), ngram(e->ngram) {}//use e->ngramImg() to get correct exemplar image}
    string getNgram() {return ngram;}
    unsigned long getId() {return id;}

    private:
    string ngram;
    unsigned long id;
};

class Spotter
{
    public:
    Spotter(MasterQueue* masterQueue, const Knowledge::Corpus* corpus, string modelDir);
    ~Spotter();
    void run(int numThreads);
    void stop();

    void addQueries(vector<Spotting*>& exemplars);
    void removeQueries(vector<pair<unsigned long,string> >* toRemove);

    virtual vector<Spotting>* runQuery(SpottingQuery* query)=0;

    protected:
    MasterQueue* masterQueue;
    const Knowledge::Corpus* corpus;
    //atomic_char cont;
    int numThreads;

    sem_t semLock;
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
