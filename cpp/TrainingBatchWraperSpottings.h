#ifndef TRN_BATCH_WRAPER_SPOTTINGS
#define TRN_BATCH_WRAPER_SPOTTINGS
#include <nan.h>
#include <iostream>
#include <assert.h>
#define BUFFERSIZE 65536
#include <b64/encode.h>
#include "opencv2/highgui/highgui.hpp"
#include "spotting.h"
#include "batches.h"
#include "BatchWraperSpottings.h"

using namespace Nan;
using namespace std;
using namespace v8;
class TrainingBatchWraperSpottings: public BatchWraperSpottings
{
    private:
        //output
        string instructions;
        string correct;
        bool last;
        
    public:
        TrainingBatchWraperSpottings(SpottingsBatch* batch, string correct, string instructions, bool last);
        void doCallback(Callback* callback);
};
#endif
