#include <nan.h>
#include <iostream>
#include <assert.h>
#define BUFFERSIZE 65536
#include <b64/encode.h>
#include "opencv2/highgui/highgui.hpp"

#include "MasterQueue.h"

using namespace Nan;
using namespace std;
using namespace v8;

class SpottingBatchUpdateWorker : public AsyncWorker {
    public:
        SpottingBatchUpdateWorker(Callback *callback, MasterQueue* masterQueue, string resultsId, vector<string> ids, vector<int> labels)
        : AsyncWorker(callback), masterQueue(masterQueue), resultsId(resultsId), ids(ids), labels(labels) {}

        ~SpottingBatchUpdateWorker() {}


        void Execute () {
            
            vector<Spotting>* toAdd = masterQueue->feedback(stoul(resultsId),ids,labels);
            //TODO update global spottings
            delete toAdd;
        }

        // We have the results, and we're back in the event loop.
        void HandleOKCallback () {
            Nan:: HandleScope scope;
            Local<Value> argv[] = {
                Nan::Null()
            };

            callback->Call(1, argv);

        }
    private:
        MasterQueue* masterQueue;
        string resultsId;
        vector<string> ids;
        vector<int> labels;
        
};
