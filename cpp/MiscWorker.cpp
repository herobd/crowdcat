
#include <nan.h>
#include <iostream>
#include <assert.h>
#include "opencv2/highgui/highgui.hpp"

#include "MasterQueue.h"
#include "Knowledge.h"

using namespace Nan;
using namespace std;
using namespace v8;

class MiscWorker : public AsyncWorker {
    public:
        MiscWorker(Callback *callback, string task, MasterQueue* masterQueue, Knowledge::Corpus* corpus)
        : AsyncWorker(callback), task(task), masterQueue(masterQueue), corpus(corpus) {}

        ~MiscWorker() {}


        void Execute () {
            if (task.compare("showCorpus")==0)
            {
                corpus->show();
            }    
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
        MasterQueue* masterQueue;
        Knowledge::Corpus* corpus;
        string task;
        
};
