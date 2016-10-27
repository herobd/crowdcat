#ifndef BATCH_WRAPER_SPOTTINGS
#define BATCH_WRAPER_SPOTTINGS
#ifndef NO_NAN
#include <nan.h>
#endif
#include <iostream>
#include <assert.h>
#define BUFFERSIZE 65536
#include <b64/encode.h>
#include "opencv2/highgui/highgui.hpp"
#include "spotting.h"
#include "batches.h"
#include "BatchWraper.h"

#ifndef NO_NAN
using namespace Nan;
using namespace v8;
#endif
using namespace std;
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
        vector<string> gt;
        
    public:
        BatchWraperSpottings(SpottingsBatch* batch);
        ~BatchWraperSpottings() {}
#ifndef NO_NAN
        virtual void doCallback(Callback* callback);
#else
        virtual int getType(){return SPOTTINGS;};
        virtual void getSpottings(string* resId,string* ngram, vector<string>* ids, vector<Location>* locs, vector<string>* gt);
#endif
};
#endif
