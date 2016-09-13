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

//#include "ctpl_stl.h"

class CATTSS
{
    private:
    MasterQueue* masterQueue;
    SpottingQueue* spottingQueue;
    Knowledge::Corpus* corpus;
    thread* incompleteChecker;
    thread* showChecker;

    //ctpl::thread_pool* pool;

    public:
    CATTSS( string lexiconFile,
            string pageImageDir, 
            string segmentationFile, 
            string spottingModelPrefix,
            int numSpottingThreads,
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

};
#endif
