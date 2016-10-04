
#ifndef BATCH_WRAPER_TRANS
#define BATCH_WRAPER_TRANS
#include <nan.h>
#include <iostream>
#include <assert.h>
#define BUFFERSIZE 65536
#include <b64/encode.h>
#include "opencv2/highgui/highgui.hpp"
#include "batches.h"
#include "spotting.h"
#include "BatchWraper.h"

using namespace Nan;
using namespace std;
using namespace v8;
class BatchWraperTranscription: public BatchWraper
{
    protected:
        //output
        string batchId;
        string wordImgStr;
        string ngramImgStr;
        string wordIndex;
        string gt;
        double scale;
        vector<string> retPoss;
        vector<SpottingPoint> spottings;
        bool manual;

    public:
        BatchWraperTranscription(TranscribeBatch* batch);
        ~BatchWraperTranscription() {}
        virtual void doCallback(Callback* callback);
};
#endif
