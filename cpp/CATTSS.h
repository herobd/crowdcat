#ifndef CATTSS_H
#define CATTSS_H

#include "MasterQueue.h"
#include "Knowledge.h"
#include "SpottingQueue.h"
#include "spotting.h"
#include "Lexicon.h"
#include "BatchWraper.h"
#include "opencv2/core/core.hpp"
#include <exception>
#include <pthread.h>

#include <unistd.h>
//#include "ctpl_stl.h"

#define NEW_EXEMPLAR_TASK 1
#define TRANSCRIPTION_TASK 2
#define SPOTTINGS_TASK 3
struct UpdateTask
{
    int type;
    string id;
    vector<int> labels;
    int resent_manual_bool;
    vector<string> strings;
    UpdateTask(string batchId,  vector<int>& labels, int resent) : type(NEW_EXEMPLAR_TASK), id(batchId), labels(labels), resent_manual_bool(resent) {} 
    UpdateTask(string id, string transcription, bool manual) : type(TRANSCRIPTION_TASK), id(id),  resent_manual_bool(manual) {strings.push_back(transcription);}
    UpdateTask(string resultsId, vector<string>& ids, vector<int>& labels, int resent) : type(SPOTTINGS_TASK), id(resultsId), labels(labels), resent_manual_bool(resent), strings(ids) {} 
};

class CATTSS
{
    private:
    MasterQueue* masterQueue;
    SpottingQueue* spottingQueue;
    Knowledge::Corpus* corpus;
    thread* incompleteChecker;
    thread* showChecker;

    //ctpl::thread_pool* pool;
    //Thread pool stuff
    vector<thread*> taskThreads;
    atomic_char cont;
    deque<UpdateTask*> taskQueue;
    sem_t semLock;
    mutex taskQueueLock;

    UpdateTask* dequeue();
    void enqueue(UpdateTask* task);
    void run(int numThreads);
    void stop();

    public:
    CATTSS( string lexiconFile,
            string pageImageDir, 
            string segmentationFile, 
            string spottingModelPrefix,
            int numSpottingThreads,
            int numTaskThreads,
            int showHeight,
            int showWidth,
            int showMilli );
    ~CATTSS()
    {
        delete incompleteChecker;
        delete showChecker;
        delete masterQueue;
        delete corpus;
        delete spottingQueue;
    } 

    BatchWraper* getBatch(int num, int width, int color, string prevNgram);
    void updateSpottings(string resultsId, vector<string> ids, vector<int> labels, int resent);
    void updateTranscription(string id, string transcription, bool manual);
    void updateNewExemplars(string resultsId,  vector<int> labels, int resent);
    void misc(string task);

    const cv::Mat* imgForPageId(int id) const {return corpus->imgForPageId(id);}
    void threadLoop();

};
#endif
