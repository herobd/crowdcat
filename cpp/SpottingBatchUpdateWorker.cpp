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
        SpottingBatchUpdateWorker(Callback *callback, MasterQueue* masterQueue, Knowledge::Corpus* corpus, sstring resultsId, vector<string> ids, vector<int> labels, int resent)
        : AsyncWorker(callback), masterQueue(masterQueue), resultsId(resultsId), ids(ids), labels(labels), resent(resent) {}

        ~SpottingBatchUpdateWorker() {}


        void Execute () {
            
            vector<Spotting>* toAdd = masterQueue->feedback(stoul(resultsId),ids,labels,resent);
            if (resent)
            {
                //TODO
            }
            else
            {
                corpus->addSpottings(toAdd);
            }
            delete toAdd;
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
        string resultsId;
        vector<string> ids;
        vector<int> labels;
        int resent;
        
};
