#ifndef TRN_BATCH_WRAPER_TRANS_H
#define TRN_BATCH_WRAPER_TRANS_H
#include <nan.h>
#include <iostream>
#include <assert.h>
#define BUFFERSIZE 65536
#include <b64/encode.h>
#include "opencv2/highgui/highgui.hpp"
#include "spotting.h"
#include "batches.h"
#include "BatchWraperTranscription.h"

using namespace Nan;
using namespace std;
using namespace v8;
class TrainingBatchWraperTranscription: public BatchWraperTranscription
{
    private:
        //output
        string instructions;
        string correct;
        bool last;
        
    public:
        TrainingBatchWraperTranscription(TranscribeBatch* batch, string correct, string instructions, bool last);
        void doCallback(Callback* callback);
};
#endif
