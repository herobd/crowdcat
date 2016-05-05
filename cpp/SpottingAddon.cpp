
#include <nan.h>
#include <functional>
#include <iostream>
#include <assert.h>
#define BUFFERSIZE 65536
#include <b64/encode.h>
#include "opencv2/highgui/highgui.hpp"

#include "MasterQueue.h"


using namespace Nan;
using namespace std;
using namespace v8;

#include "BatchRetrieveWorker.cpp"
#include "SpottingBatchUpdateWorker.cpp"

cv::Mat globalObject;
MasterQueue* masterQueue;


class ImageWorker : public AsyncWorker {
    public:
        ImageWorker(Callback *callback, int param)
        : AsyncWorker(callback), param(param) {}

        ~ImageWorker() {}


        void Execute () {
            vector<int> compression_params;
            compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
            compression_params.push_back(param);
            vector<uchar> outBuf;
            cv::imencode(".png",globalObject,outBuf,compression_params);
            
            base64::encoder E;
            stringstream ss;
            /*for (uchar c : outBuf)  {
                //retData+=c;
                ss.write(c);
            }*/
            ss.write((char*)outBuf.data(),outBuf.size());
            stringstream encoded;
            E.encode(ss, encoded);
            retData=encoded.str();
        }

        // We have the results, and we're back in the event loop.
        void HandleOKCallback () {
            Nan:: HandleScope scope;

            /*v8::Local<v8::Array> results = New<v8::Array>(outBuf.size());
            int i = 0;
            for_each(primes.begin(), primes.end(),
                [&](int value) {
                    Nan::Set(results, i, New<v8::Number>(value));
                    i++;
            });*/
            
            //v8:Local<v8:String> result = String::New( retData.c_str() );

            Local<Value> argv[] = {
                Nan::Null(),
                Nan::New(retData.c_str()).ToLocalChecked()
            };

            callback->Call(2, argv);

        }
    private:
        string retData;
        int param;
        
};

// Asynchronous access to the `getTestImage()` function
NAN_METHOD(getGlobalImage) {
    int param = To<int>(info[0]).FromJust();
    Callback *callback = new Callback(info[1].As<Function>());

    AsyncQueueWorker(new ImageWorker(callback, param));
}

NAN_METHOD(getNextBatch) {
    int width = To<int>(info[0]).FromJust();
    int num = To<int>(info[1]).FromJust();
    Callback *callback = new Callback(info[2].As<Function>());

    AsyncQueueWorker(new BatchRetrieveWorker(callback, width,num,masterQueue));
}

NAN_METHOD(spottingBatchDone) {
    //string batchId = To<string>(info[0]).FromJust();
    //? labels = To<?>(info[1]).FromJust();
    Callback *callback = new Callback(info[2].As<Function>());

    AsyncQueueWorker(new SpottingBatchUpdateWorker(callback,masterQueue));
}


NAN_MODULE_INIT(Init) {
    globalObject = cv::imread("./test.png");
    masterQueue = new MasterQueue();
    
    assert(globalObject.rows>0 && globalObject.cols>0);
    Nan::Set(target, New<String>("getTestImage").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(getGlobalImage)).ToLocalChecked());
    
    
    Nan::Set(target, New<String>("getNextBatch").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(getNextBatch)).ToLocalChecked());
    
    Nan::Set(target, New<String>("spottingBatchDone").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(spottingBatchDone)).ToLocalChecked());
}

NODE_MODULE(SpottingAddon, Init)
