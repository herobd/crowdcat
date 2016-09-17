#ifndef BATCH_WRAPER_SPOTTINGS
#define BATCH_WRAPER_SPOTTINGS
#include <nan.h>
#include <iostream>
#include <assert.h>
#define BUFFERSIZE 65536
#include <b64/encode.h>
#include "opencv2/highgui/highgui.hpp"
#include "spotting.h"
#include "batches.h"
#include "BatchWraper.h"

using namespace Nan;
using namespace std;
using namespace v8;
class BatchWraperSpottings: public BatchWraper
{
    protected:
        //output
        vector<string> retData;
        vector<string> retId;
        vector<Location> locations;
        string batchId;
        string resultsId;
        string ngram;
        
    public:
        BatchWraperSpottings(SpottingsBatch* batch);
        ~BatchWraperSpottings() {}
        virtual void doCallback(Callback* callback);
};
#endif
