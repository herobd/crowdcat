
#include <nan.h>
#include <iostream>
#include <assert.h>
#define BUFFERSIZE 65536
#include <b64/encode.h>
#include "opencv2/highgui/highgui.hpp"

#include "CrowdCAT.h"

using namespace Nan;
using namespace std;
using namespace v8;

class TranscriptionBatchUpdateWorker : public AsyncWorker {
    public:
        TranscriptionBatchUpdateWorker(Callback *callback, CrowdCAT* crowdcat, string userId, string id, string transcription, bool manual)
        : AsyncWorker(callback), crowdcat(crowdcat), userId(userId), id(id), transcription(transcription), manual(manual) {}

        ~TranscriptionBatchUpdateWorker() {}


        void Execute () 
        {
            crowdcat->updateTranscription(userId,id,transcription,manual);
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
        CrowdCAT* crowdcat;
        string userId;
        string id;
        string transcription;
        bool manual;
        
};
