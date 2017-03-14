#ifndef MASTERQUEUE_H
#define MASTERQUEUE_H


#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <vector>
#include <queue>
#include <iostream>
#include "semaphore.h"
#include <pthread.h>
#include <thread>
#include <chrono>
#include <atomic>
#include "BatchWraper.h"
#include "BatchWraperTranscription.h"
#include <fstream>
#include <assert.h>

#include "CorpusRef.h"
#include "PageRef.h"
#include "Word.h"
#include "CorpusDataset.h"
#include "Global.h"

using namespace std;



class MasterQueue {
private:
    multimap<float,Word*> wordsByScore;
    CorpusDataset* words;
    map<unsigned long, chrono::system_clock::time_point> timeMap;
    mutex queueLock;//.lock(), .unlock()
    int contextPad;

    atomic_bool finish;

    MasterQueueState state;

    //network_t *pruningNet;

public:
    MasterQueue(CorpusDataset* words, int contextPad);
    MasterQueue(ifstream& in, CorpusDataset* words);
    void save(ofstream& out);

    BatchWraper* getBatch(string userId, unsigned int maxWidth);
    
    
    void transcriptionFeedback(string userId, unsigned long id, string transcription);
    void setTranscriptions();

    virtual ~MasterQueue()
    {
    }
    void checkIncomplete();
    void setFinish(bool v) {finish.store(v);}

    bool goodEnough(Word* word);

    MasterQueueState getState()
    {
        queueLock.lock();
        MasterQueueState ret = state;
        queueLock.unlock();
    }
};
#endif
