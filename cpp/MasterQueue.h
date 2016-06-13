#ifndef MASTERQUEUE_H
#define MASTERQUEUE_H


#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <vector>
#include <queue>
#include <iostream>
#include "semaphore.h"
#include <pthread.h>
#include "SpottingResults.h"
#include "TranscribeBatchQueue.h"
#include <thread>
#include <chrono>
#include <atomic>

#include <fstream>

using namespace std;







class MasterQueue {
private:
    pthread_rwlock_t semResultsQueue;
    pthread_rwlock_t semResults;
    
    map<unsigned long, pair<sem_t*,SpottingResults*> > results;
    map<unsigned long, pair<sem_t*,SpottingResults*> > resultsQueue;
    TranscribeBatchQueue transcribeBatchQueue;
    thread* incompleteChecker;
    
    //int atID;
    //map<unsigned long,unsigned long> batchToResults;
    
    //testing stuff
    //cv::Mat page;
    int testIter;
    int test_rotate;
    map<string,cv::Mat> pages;
    void addTestSpottings();
    void test_showResults(unsigned long id,string ngram);
    void test_finish();
    
    map<unsigned long, map<unsigned long,bool> > test_groundTruth;
    map<unsigned long, int> test_total;
    map<unsigned long, int> test_numDone;
    map<unsigned long, int> test_totalPos;
    map<unsigned long, int> test_numTruePos;
    map<unsigned long, int> test_numFalsePos;
    double accuracyAvg, recallAvg, manualAvg, effortAvg;
    int done;
    int numCTrue, numCFalse;
public:
    MasterQueue();
    SpottingsBatch* getSpottingsBatch(unsigned int numberOfInstances, bool hard, unsigned int maxWidth, int color, string prevNgram);
    vector<Spotting>* feedback(unsigned long id, const vector<string>& ids, const vector<int>& userClassifications, int resent);
    void addSpottingResults(SpottingResults* res);
    
    TranscribeBatch* getTranscriptionBatch() {return transcribeBatchQueue.dequeue();}
    void transcriptionFeedback(unsigned long id, string transcription) {transcribeBatchQueue.feedback(id, transcription);}
    //test
    vector<Spotting>* test_feedback(unsigned long id, const vector<string>& ids, const vector<int>& userClassifications);
    bool test_autoBatch();
    ~MasterQueue()
    {
        kill.store(true);
        //~(*incompleteChecker)();
        
        cout << "***********"<<endl;
        cout << "* accuracy: "<<accuracyAvg/done<<endl;
        cout << "* recall: "<<recallAvg/done<<endl;
        cout << "* manual: "<<manualAvg/done<<endl;
        cout << "* effort: "<<effortAvg/done<<endl;
        cout << "* true/false: "<<numCTrue/(0.0+numCFalse)<<endl;
        cout << "***********"<<endl;
        delete incompleteChecker;
        pthread_rwlock_destroy(&semResultsQueue);
        pthread_rwlock_destroy(&semResults);
    }
    void checkIncomplete();
    atomic_bool kill;
};
#endif
