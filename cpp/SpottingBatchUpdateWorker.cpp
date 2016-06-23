#include <nan.h>
#include <iostream>
#include <assert.h>
#include "opencv2/highgui/highgui.hpp"

#include "MasterQueue.h"
#include "Knowledge.h"

using namespace Nan;
using namespace std;
using namespace v8;

class SpottingBatchUpdateWorker : public AsyncWorker {
    public:
        SpottingBatchUpdateWorker(Callback *callback, MasterQueue* masterQueue, Knowledge::Corpus* corpus, string resultsId, vector<string> ids, vector<int> labels, int resent)
        : AsyncWorker(callback), masterQueue(masterQueue), corpus(corpus),
          resultsId(resultsId), ids(ids), labels(labels), resent(resent) {}

        ~SpottingBatchUpdateWorker() {}


        void Execute () {
            cout <<"Recieved batch for "<<resultsId<<endl;            
            vector<Spotting>* toAdd = masterQueue->feedback(stoul(resultsId),ids,labels,resent);
            vector<TranscribeBatch*> newBatches = corpus->addSpottings(toAdd);
            masterQueue->enqueueTranscriptionBatches(newBatches);
            cout <<"Enqueued "<<newBatches.size()<<" new trans"<<endl;            
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
