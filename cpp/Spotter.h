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
    SpottingQuery(const Spotting* e) : id(e->id), ngram(e->ngram), img(e->ngramImg()) {}
    SpottingQuery(const Spotting& e) : SpottingQuery(&e) {}
    SpottingQuery(string ngram) : id(0), ngram(ngram) {}
    string getNgram() {return ngram;}
    unsigned long getId() {return id;}
    cv::Mat getImg() {return img;}

    private:
    string ngram;
    unsigned long id;
    cv::Mat img;
};

class Spotter
{
    public:
    Spotter(MasterQueue* masterQueue, const Knowledge::Corpus* corpus, string modelPrefix;
    ~Spotter();
    void run(int numThreads);
    void stop();

    void addQueries(vector<SpottingExemplar*>& exemplars);
    void addQueries(vector<Spotting*>& exemplars);
    void addQueries(vector<Spotting>& exemplars);
    void addQueries(vector<string>& ngrams);
    void removeQueries(vector<pair<unsigned long,string> >* toRemove);

    virtual vector<Spotting>* runQuery(SpottingQuery* query)=0;
    virtual float score(string text, const Mat& image)=0;

    protected:
    MasterQueue* masterQueue;
    const Knowledge::Corpus* corpus;
    atomic_char cont;
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
