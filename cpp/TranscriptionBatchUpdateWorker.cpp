
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

class TranscriptionBatchUpdateWorker : public AsyncWorker {
    public:
        TranscriptionBatchUpdateWorker(Callback *callback, CATTSS* cattss, string id, string transcription)
        : AsyncWorker(callback), cattss(cattss), id(id), transcription(transcription) {}

        ~TranscriptionBatchUpdateWorker() {}


        void Execute () 
        {
            cattss->updateTranscription(id,transcription);
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
        CATTSS* cattss;
        Spotter* spotter;
        string id;
        string transcription;
        
};
