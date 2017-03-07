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

using namespace std;


enum MasterQueueState {EMPTY, TOP_RESULTS, PAUSED, REMAINDER, CLEAN_UP, DONE};

class MasterQueue {
private:
    multimap<float,Word*> wordsByScore;
    CorpusDataset* words;
    map<unsigned long, Word*> returnMap;
    map<unsigned long, chrono::system_clock::time_point> timeMap;
    //map<unsigned long, WordBackPointer*> doneMap;
    mutex otherLock;
    mutex queueLock;//.lock(), .unlock()
    int contextPad;

    atomic_bool finish;

    MasterQueueState state;

public:
    MasterQueue(CorpusDataset* words, int contextPad);
    MasterQueue(ifstream& in, CorpusRef* corpusRef, PageRef* pageRef);
    void save(ofstream& out);

    BatchWraper* getBatch(unsigned int maxWidth);
    
    
    void transcriptionFeedback(unsigned long id, string transcription);
    void enqueueTranscriptionBatches(vector<TranscribeBatch*> newBatches, vector<unsigned long>* remove=NULL) {transcribeBatchQueue.enqueueAll(newBatches,remove);};
    virtual ~MasterQueue()
    {
    }
    void checkIncomplete();
    void setFinish(bool v) {finish.store(v);}

};
#endif
