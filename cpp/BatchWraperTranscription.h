
#ifndef BATCH_WRAPER_TRANS
#define BATCH_WRAPER_TRANS
#include <nan.h>
#include <iostream>
#include <assert.h>
#define BUFFERSIZE 65536
#include <b64/encode.h>
#include "opencv2/highgui/highgui.hpp"
#include "SpottingResults.h"
#include "BatchWraper.h"
#include "Knowledge.h"

using namespace Nan;
using namespace std;
using namespace v8;
class BatchWraperTranscription: public BatchWraper
{
    private:
        //output
        string batchId;
        string wordImgStr;
        string ngramImgStr;
        vector<string> retPoss;

    public:
        BatchWraperTranscription(TranscribeBatch* batch);
        ~BatchWraperTranscription() {}
        void doCallback(Callback* callback);
};
#endif
