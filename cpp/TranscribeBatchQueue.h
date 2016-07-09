#ifndef TRANS_BATCH_Q
#define TRANS_BATCH_Q

#include <deque>
#include <map>
#include <chrono>

#include <mutex>
#include <iostream>

#include "Knowledge.h"

using namespace std;

class TranscribeBatchQueue
{
    public:
        TranscribeBatchQueue();
        void enqueueAll(vector<TranscribeBatch*> batches, vector<unsigned long>* remove=NULL);

        TranscribeBatch* dequeue(unsigned int maxWidth);

        vector<Spotting*> feedback(unsigned long id, string transcription);

        void checkIncomplete();
        void lock() { mutLock.lock(); }
        void unlock() { mutLock.unlock(); }
    private:
        deque<TranscribeBatch*> queue;
        map<unsigned long, TranscribeBatch*> returnMap;
        map<unsigned long, chrono::system_clock::time_point> timeMap;
        map<unsigned long, WordBackPointer*> doneMap;
        mutex mutLock;
};
#endif
