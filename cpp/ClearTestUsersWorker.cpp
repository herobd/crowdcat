#include <nan.h>
#include <iostream>
#include <assert.h>

#include "TestQueue.h"

using namespace Nan;
using namespace std;
using namespace v8;

class ClearTestUsersWorker : public AsyncWorker {
    public:
        ClearTestUsersWorker(Callback *callback, TestQueue* testQueue)
        : AsyncWorker(callback), testQueue(testQueue) {}

        ~ClearTestUsersWorker() {}


        void Execute () {
            
            testQueue->clearUsers();
            
        }

        // We have the results, and we're back in the event loop.
        void HandleOKCallback () {
            Nan:: HandleScope scope;
            Local<Value> argv[] = {
                Nan::Null()
            };
            //cout <<"calling back"<<endl;
            callback->Call(1, argv);

        }
    private:
        TestQueue* testQueue;
};
