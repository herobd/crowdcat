#ifndef TESTQUEUE_H
#define TESTQUEUE_H


#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <vector>
#include <queue>
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
    map<USERID, vector< SpottingsBatch* > > userQueues;
    map<USERID, map<unsigned long, char> > classified;
    map<USERID, int> returned;
    
    map<string,cv::Mat> pages;
public:
    TestQueue();
    vector< SpottingsBatch* > getTestSpottings(int maxWidth);
    SpottingsBatch* getBatch(unsigned int numberOfInstances, unsigned int maxWidth, USERID userId) ;
    bool feedback(unsigned long id, const vector<string>& ids, const vector<int>& userClassifications, USERID userId, int* fp, int* fn);
    
    int numTestBatches;
};
#endif
