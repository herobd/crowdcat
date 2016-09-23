#ifndef MASTERQUEUE_H
#define MASTERQUEUE_H


#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <vector>
#include <queue>
#include <iostream>
#include "semaphore.h"
#include <pthread.h>
#include "spotting.h"
#include "SpottingResults.h"
#include "TranscribeBatchQueue.h"
#include "NewExemplarsBatchQueue.h"
#include <thread>
#include <chrono>
#include <atomic>
#ifndef TEST_MODE_C
#include "BatchWraper.h"
#include "BatchWraperSpottings.h"
#include "BatchWraperTranscription.h"
#include "BatchWraperNewExemplars.h"
#endif
#include <fstream>

using namespace std;

#define ROTATE 1 //this is for testing, forcing it to feed up batches from different spotting results.

#define NGRAM_Q_COUNT_THRESH_NEW 100//4
#define NGRAM_Q_COUNT_THRESH_WORD 100//6
#define TRANS_READY_THRESH 10//50


class MasterQueue {
private:
    pthread_rwlock_t semResultsQueue;
    pthread_rwlock_t semResults;
    
    map<unsigned long, pair<sem_t*,SpottingResults*> > results;
    map<unsigned long, pair<sem_t*,SpottingResults*> > resultsQueue;
    TranscribeBatchQueue transcribeBatchQueue;
    NewExemplarsBatchQueue newExemplarsBatchQueue;
    
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
    atomic_bool finish;
    int numCTrue, numCFalse;
    void updateSpottingsMix(const vector<SpottingExemplar*>* spottings);
public:
    MasterQueue();
#ifndef TEST_MODE_C
    BatchWraper* getBatch(unsigned int numberOfInstances, bool hard, unsigned int maxWidth, int color, string prevNgram);
#endif
    SpottingsBatch* getSpottingsBatch(unsigned int numberOfInstances, bool hard, unsigned int maxWidth, int color, string prevNgram, bool need=true);
    vector<Spotting>* feedback(unsigned long id, const vector<string>& ids, const vector<int>& userClassifications, int resent, vector<pair<unsigned long,string> >* remove);
    virtual unsigned long updateSpottingResults(vector<Spotting>* spottings, unsigned long id=0);//a negative id means add a new spottingresult
    void addSpottingResults(SpottingResults* res, bool hasSemResults=false, bool toQueue=true);
    
    
    //TranscribeBatch* getTranscriptionBatch(unsigned int maxWidth) {return transcribeBatchQueue.dequeue(maxWidth);}
    void transcriptionFeedback(unsigned long id, string transcription, vector<pair<unsigned long, string> >* toRemoveExemplars);
    void enqueueTranscriptionBatches(vector<TranscribeBatch*> newBatches, vector<unsigned long>* remove=NULL) {transcribeBatchQueue.enqueueAll(newBatches,remove);};
    void enqueueNewExemplars(const vector<Spotting*>& newExemplars, vector<pair<unsigned long, string> >* toRemoveExemplars);
    //NewExemplarsBatch* getNewExemplarsBatch(int batchSize, unsigned int maxWidth, int color) {return newExemplarsBatchQueue.dequeue(batchSize,maxWidth,color);}
    vector<SpottingExemplar*> newExemplarsFeedback(unsigned long id,  const vector<int>& userClassifications, vector<pair<unsigned long, string> >* toRemoveExemplars) 
    {
        vector<SpottingExemplar*> ret = newExemplarsBatchQueue.feedback(id, userClassifications, toRemoveExemplars);
        updateSpottingsMix(&ret);
        return ret;
    }
    void needExemplar(string ngram);
    virtual ~MasterQueue()
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
        pthread_rwlock_destroy(&semResultsQueue);
        pthread_rwlock_destroy(&semResults);
    }
    void checkIncomplete();
    atomic_bool kill;
    void setFinish(bool v) {finish.store(v);}

    //test
    vector<Spotting>* test_feedback(unsigned long id, const vector<string>& ids, const vector<int>& userClassifications);
    bool test_autoBatch();
};
#endif
