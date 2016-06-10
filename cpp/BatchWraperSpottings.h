
#include <nan.h>
#include <iostream>
#include <assert.h>
#include <b64/encode.h>
#include "opencv2/highgui/highgui.hpp"
#include "SpottingResults.h"
#include "BatchWraper.h"

class BatchWraperSpottings: BatchWraper
{
    private:
        //output
        vector<string> retData;
        vector<string> retId;
        string batchId;
        string resultsId;
        string ngram;
        
        //input
        int width;
        int color;
        string prevNgram;
        int num;
        MasterQueue* masterQueue;
    public:
        BatchWraperSpottings(SpottingsBatch* batch);
        void doCallback(Callback* callback);
};
