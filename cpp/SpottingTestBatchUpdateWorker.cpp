#include <nan.h>
#include <iostream>
#include <assert.h>

#include "TestQueue.h"

using namespace Nan;
using namespace std;
using namespace v8;

class SpottingTestBatchUpdateWorker : public AsyncWorker {
    public:
        SpottingTestBatchUpdateWorker(Callback *callback, TestQueue* testQueue, string resultsId, vector<string> ids, vector<int> labels, int userId)
        : AsyncWorker(callback), testQueue(testQueue), resultsId(resultsId), ids(ids), labels(labels), userId(userId) {}

        ~SpottingBatchUpdateWorker() {}


        void Execute () {
            
            fp=fn=-1;
            finished = testQueue->feedback(stoul(resultsId),ids,labels,userId, &fp, &fn);
            
        }

        // We have the results, and we're back in the event loop.
        void HandleOKCallback () {
            Nan:: HandleScope scope;
            Local<Value> argv[] = {
                Nan::Null(),
                Nan::New(to_string(finished)).ToLocalChecked(),
                Nan::New(to_string(fp)).ToLocalChecked(),
                Nan::New(to_string(fn)).ToLocalChecked()
            };
            //cout <<"calling back"<<endl;
            callback->Call(4, argv);

        }
    private:
        TestQueue* testQueue;
        int userId;
        string resultsId;
        vector<string> ids;
        vector<int> labels;
        
        int fp, fn;
        bool finished;
};
