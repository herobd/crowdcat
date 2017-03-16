#ifndef TRN_INSTANCES_H
#define TRN_INSTANCES_H

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "BatchWraper.h"
#include "BatchWraperSpottings.h"
#include "BatchWraperTranscription.h"
#include "BatchWraperNewExemplars.h"
#include "spotting.h"
#include "batches.h"
#include "Knowledge.h"
#include "SpecialInstances.h"

class TrainingInstances : public SpecialInstances
{
    public:
        TrainingInstances();
        BatchWraper* getBatch(int width, int color, string prevNgram, int trainingNum);

    private:
        BatchWraper* makeInstance(int trainingNum, int width,int color);

        cv::Mat line;
        cv::Mat letsgo;
        Spotting spotting_0;
        Spotting spotting_1;
        Spotting spotting_2;
        Spotting spotting_3;
        Spotting spotting_4;
        Spotting spotting_5;
        Spotting spotting_6;
        Spotting spotting_7;
        Word* word;
        Spotting spotting_8a;
        Spotting spotting_9b;
        Spotting spotting_10b;
        Spotting spotting_11a;
        Spotting spotting_12a;
        Spotting spotting_12b;
        Spotting spotting_GO;
};

#endif 
