//#include "BatchRetrieveWorker.h"

#include <nan.h>
#include <iostream>
#include <assert.h>
#include "opencv2/highgui/highgui.hpp"

#include "TrainingInstances.h"
#include "BatchWraper.h"
using namespace Nan;
using namespace std;
using namespace v8;

class TrainingBatchRetrieveWorker : public AsyncWorker {
    public:
        TrainingBatchRetrieveWorker(Callback *callback, TrainingInstances* trainingInstances, int width, int color, string prevNgram, int num, int trainingNum)
        : AsyncWorker(callback), width(width), color(color), prevNgram(prevNgram), num(num), trainingInstances(trainingInstances), batch(NULL), trainingNum(trainingNum) {}

        ~TrainingBatchRetrieveWorker() {}


        void Execute () {
            batch = trainingInstances->getBatch(num,width,color,trainingNum);
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
        int num, trainingNum;
        TrainingInstances* trainingInstances;
        
        
};


