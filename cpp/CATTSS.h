#ifndef CATTSS_H
#define CATTSS_H

#include "MasterQueue.h"
#include "Spotter.h"
#include "spotting.h"
#include "FacadeSpotter.h"
#include "Knowledge.h"
#include "Lexicon.h"
#include "BatchWraper.h"
#include "opencv2/core/core.hpp"
#include <exception>
#include <pthread.h>

class CATTSS
{
    private:
    MasterQueue* masterQueue;
    Spotter* spotter;
    Knowledge::Corpus* corpus;
    thread* incompleteChecker;

    public:
    CATTSS(string lexiconFile, string pageImageDir, string segmentationFile);

    BatchWraper* getBatch(int num, int width, int color, string prevNgram);
    void updateSpottings(string resultsId, vector<string> ids, vector<int> labels, int resent);
    void updateTranscription(string id, string transcription, bool manual);
    void updateNewExemplars(string resultsId,  vector<int> labels, int resent);
    void misc(string task);

    const cv::Mat* imgForPageId(int id) const {return corpus->imgForPageId(id);}

};
#endif
