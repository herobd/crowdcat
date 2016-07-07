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
            //cout <<"Recieved batch for "<<resultsId<<endl;
            vector<pair<unsigned long,string> > toRemoveSpottings;        
            vector<unsigned long> toRemoveBatches;        
            vector<Spotting>* toAdd = masterQueue->feedback(stoul(resultsId),ids,labels,resent,&toRemoveSpottings);
            vector<Spotting> newExemplars;
            vector<TranscribeBatch*> newBatches = corpus->updateSpottings(toAdd,&toRemoveSpottings,&toRemoveBatches,&newExemplars);
            masterQueue->enqueueTranscriptionBatches(newBatches,&toRemoveBatches);
            masterQueue->enqueueNewExemplars(&newExemplars);
            //cout <<"Enqueued "<<newBatches.size()<<" new trans batches"<<endl;            
            //if (toRemoveBatches.size()>0)
            //    cout <<"Removed "<<toRemoveBatches.size()<<" trans batches"<<endl;     
            spotter->removeQueries(&toRemoveSpottings);
            spotter->addQueries(toAdd);
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
