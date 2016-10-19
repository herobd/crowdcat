#ifndef TRANS_BATCH_Q
#define TRANS_BATCH_Q

#include <deque>
#include <map>
#include <chrono>

#include <mutex>
#include <iostream>

#include "spotting.h"
#include "batches.h"
#include "WordBackPointer.h"
#include "CorpusRef.h"

using namespace std;

class TranscribeBatchQueue
{
    public:
        TranscribeBatchQueue();
        void save(ofstream& out);
        void load(ifstream& in, CorpusRef* corpusRef);

        void enqueueAll(vector<TranscribeBatch*> batches, vector<unsigned long>* remove=NULL);

        TranscribeBatch* dequeue(unsigned int maxWidth);

        vector<Spotting*> feedback(unsigned long id, string transcription, vector<pair<unsigned long, string> >* toRemoveExemplars);

        void checkIncomplete();
    private:
        deque<TranscribeBatch*> queue;
        map<unsigned long, TranscribeBatch*> returnMap;
        map<unsigned long, chrono::system_clock::time_point> timeMap;
        map<unsigned long, WordBackPointer*> doneMap;
        mutex mutLock;
        void lock() { mutLock.lock(); }
        void unlock() { mutLock.unlock(); }
};
#endif
