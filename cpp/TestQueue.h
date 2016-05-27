#ifndef TESTQUEUE_H
#define TESTQUEUE_H


#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <vector>
#include <deque>
#include <iostream>
#include "semaphore.h"
#include <pthread.h>
#include "SpottingResults.h"

#include <fstream>

#define USERID int

using namespace std;







class TestQueue {
private:
    pthread_rwlock_t userQueuesLock;
    pthread_rwlock_t userResLock;
    
    
    
    map<unsigned long, bool> testGroundTruth;
    vector< vector<Spotting> > testBatches;
    vector<string> ngrams;
    vector<int> colors;
    map<USERID, deque< SpottingsBatch* > > userQueues;
    map<USERID, map<unsigned long, char> > classified;
    map<USERID, int> returned;
    map<USERID, int> numTestBatches;
    
    map<string,cv::Mat> pages;
public:
    TestQueue();
    deque< SpottingsBatch* > getTestSpottings(unsigned int numberOfInstances, unsigned int maxWidth, int color);
    SpottingsBatch* getBatch(unsigned int numberOfInstances, unsigned int maxWidth, int color, USERID userId, int reset) ;
    bool feedback(unsigned long id, const vector<string>& ids, const vector<int>& userClassifications, int resent, USERID userId, int* fp, int* fn);
    void clearUsers()//This is a reset, a hackish way to be sure we don't overflow
    {
        userQueues.clear();
        classified.clear();
        returned.clear();
        numTestBatches.clear();
    }
};
#endif
