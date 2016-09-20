
#include <nan.h>
#include <iostream>
#include <assert.h>
#include "opencv2/highgui/highgui.hpp"

#include "SpecialInstances.h"
#include "BatchWraper.h"
using namespace Nan;
using namespace std;
using namespace v8;

class SpecialBatchRetrieveWorker : public AsyncWorker {
    public:
        SpecialBatchRetrieveWorker(Callback *callback, SpecialInstances* specialInstances, int width, int color, string prevNgram, int num, int batchNum)
        : AsyncWorker(callback), width(width), color(color), prevNgram(prevNgram), num(num), specialInstances(specialInstances), batch(NULL), batchNum(batchNum) {}

        ~SpecialBatchRetrieveWorker() {}


        void Execute () {
            batch = specialInstances->getBatch(width,color,prevNgram,batchNum);
        }

        // We have the results, and we're back in the event loop.
        void HandleOKCallback () {
            batch->doCallback(callback);
            delete batch;
        }
    private:
        //output
        BatchWraper* batch;
        //input
        int width;
        int color;
        string prevNgram;
        int num, batchNum;
        SpecialInstances* specialInstances;
        
        
};


