#ifndef MASTERQUEUE_H
#define MASTERQUEUE_H


#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <vector>
#include <queue>
#include <iostream>
#include "semaphore.h"
#include "SpottingResults.h"

using namespace std;







class MasterQueue {
private:
    sem_t semResultsQueue;
    sem_t semResults;
    
    map<unsigned long, pair<sem_t*,SpottingResults*> > results;
    map<unsigned long, pair<sem_t*,SpottingResults*> > resultsQueue;
    
    int atID;
    //map<unsigned long,unsigned long> batchToResults;
public:
    MasterQueue();
    SpottingsBatch* getBatch(unsigned int numberOfInstances, unsigned int maxWidth);
    vector<Spotting>* feedback(unsigned long id, const vector<string>& ids, const vector<int>& userClassifications);
    void addSpottingResults(SpottingResults* res);
};
#endif
