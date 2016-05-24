#include <nan.h>
#include <iostream>
#include <assert.h>

#include "TestQueue.h"

using namespace Nan;
using namespace std;
using namespace v8;

class SpottingTestBatchUpdateWorker : public AsyncWorker {
    public:
        SpottingTestBatchUpdateWorker(Callback *callback, TestQueue* testQueue, string resultsId, vector<string> ids, vector<int> labels, int resent, int userId)
        : AsyncWorker(callback), testQueue(testQueue), resultsId(resultsId), ids(ids), labels(labels), resent(resent), userId(userId) {}

        ~SpottingTestBatchUpdateWorker() {}


        void Execute () {
            
            fp=fn=-1;
            finished = testQueue->feedback(stoul(resultsId),ids,labels,resent,userId, &fp, &fn);
        }

        // We have the results, and we're back in the event loop.
        void HandleOKCallback () {
            Nan:: HandleScope scope;
            
            Local<Value> argv[] = {
                Nan::Null(),
                Nan::New(finished?"true":"false").ToLocalChecked(),
                Nan::New(to_string(fp)).ToLocalChecked(),
                Nan::New(to_string(fn)).ToLocalChecked()
            };
            callback->Call(4, argv);
        }
    private:
        TestQueue* testQueue;
        int userId;
        int resent;
        string resultsId;
        vector<string> ids;
        vector<int> labels;
        
        int fp, fn;
        bool finished;
};
