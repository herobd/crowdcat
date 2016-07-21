
#include <nan.h>
#include <functional>
#include <iostream>
#include <assert.h>
#define BUFFERSIZE 65536

#include "CATTSS.h"
#include "TestQueue.h"

using namespace Nan;
using namespace std;
using namespace v8;

#include "BatchRetrieveWorker.cpp"
#include "SpottingBatchUpdateWorker.cpp"
#include "NewExemplarsBatchUpdateWorker.cpp"
#include "TranscriptionBatchUpdateWorker.cpp"
#include "TestBatchRetrieveWorker.cpp"
#include "SpottingTestBatchUpdateWorker.cpp"
#include "ClearTestUsersWorker.cpp"
#include "MiscWorker.cpp"

//test
#include "SpottingResults.h"

CATTSS* cattss;
TestQueue* testQueue;


NAN_METHOD(getNextBatch) {
    int width = To<int>(info[0]).FromJust();
    int color = To<int>(info[1]).FromJust();
    //string prevNgram = To<string>(info[2]).FromJust();
    //String::Utf8Value str(args[0]->ToString());
    string prevNgram;
    if (info[2]->IsString())
    {
        String::Utf8Value str(info[2]->ToString());
        prevNgram = string(*str);
        
    }
    
    //String::Utf8Value str = To<String::Utf8Value>(info[2]).FromJust();
    
    int num = To<int>(info[3]).FromJust();
    
    Callback *callback = new Callback(info[4].As<Function>());

    AsyncQueueWorker(new BatchRetrieveWorker(callback,cattss, width,color,prevNgram,num));
}

NAN_METHOD(spottingBatchDone) {
    //string batchId = To<string>(info[0]).FromJust();
    //string resultsId = To<string>(info[0]).FromJust();
    String::Utf8Value resultsIdNAN(info[0]);
    string resultsId = string(*resultsIdNAN);
    
    vector<string> ids;
    vector<int> labels;
    Handle<Value> val;
    if (info[1]->IsArray()) {
      Handle<Array> jsArray = Handle<Array>::Cast(info[1]);
      for (unsigned int i = 0; i < jsArray->Length(); i++) {
        val = jsArray->Get(i);
        ids.push_back(string(*String::Utf8Value(val)));
        //Nan::Set(arr, i, val);
      }
    }
    if (info[2]->IsArray()) {
      Handle<Array> jsArray = Handle<Array>::Cast(info[2]);
      for (unsigned int i = 0; i < jsArray->Length(); i++) {
        val = jsArray->Get(i);
        labels.push_back(val->Uint32Value());
        //Nan::Set(arr, i, val);
      }
    }
    int resent = To<int>(info[3]).FromJust();
    Callback *callback = new Callback(info[4].As<Function>());

    AsyncQueueWorker(new SpottingBatchUpdateWorker(callback,cattss,resultsId,ids,labels,resent));
}

NAN_METHOD(newExemplarsBatchDone) {
    //string batchId = To<string>(info[0]).FromJust();
    //string resultsId = To<string>(info[0]).FromJust();
    String::Utf8Value resultsIdNAN(info[0]);
    string resultsId = string(*resultsIdNAN);
    
    vector<int> labels;
    Handle<Value> val;
    if (info[1]->IsArray()) {
      Handle<Array> jsArray = Handle<Array>::Cast(info[1]);
      for (unsigned int i = 0; i < jsArray->Length(); i++) {
        val = jsArray->Get(i);
        labels.push_back(val->Uint32Value());
        //Nan::Set(arr, i, val);
      }
    }
    int resent = To<int>(info[2]).FromJust();
    Callback *callback = new Callback(info[3].As<Function>());

    AsyncQueueWorker(new NewExemplarsBatchUpdateWorker(callback,cattss,resultsId,labels,resent));
}

NAN_METHOD(getNextTranscriptionBatch) {
    int width = To<int>(info[0]).FromJust();
    
    Callback *callback = new Callback(info[1].As<Function>());

    AsyncQueueWorker(new BatchRetrieveWorker(callback,cattss, width,-1,"",-1));
}

NAN_METHOD(transcriptionBatchDone) {
    //string batchId = To<string>(info[0]).FromJust();
    //string resultsId = To<string>(info[0]).FromJust();
    String::Utf8Value resultsIdNAN(info[0]);
    string id = string(*resultsIdNAN);
    String::Utf8Value resultNAN(info[1]);
    string transcription = string(*resultNAN);
    
    Callback *callback = new Callback(info[2].As<Function>());

    AsyncQueueWorker(new TranscriptionBatchUpdateWorker(callback,cattss,id,transcription));
}

NAN_METHOD(showCorpus) {
    Callback *callback = new Callback(info[0].As<Function>());

    AsyncQueueWorker(new MiscWorker(callback,cattss, "showCorpus"));
}
NAN_METHOD(startSpotting) {
    int num = To<int>(info[0]).FromJust();
    Callback *callback = new Callback(info[1].As<Function>());

    AsyncQueueWorker(new MiscWorker(callback,cattss, "startSpotting:"+to_string(num)));
}
NAN_METHOD(stopSpotting) {
    Callback *callback = new Callback(info[0].As<Function>());

    AsyncQueueWorker(new MiscWorker(callback,cattss, "stopSpotting"));
}

NAN_METHOD(getNextTestBatch) {
    //cout<<"request for test batch"<<endl;
    int width = To<int>(info[0]).FromJust();
    int color = To<int>(info[1]).FromJust();
    int num = To<int>(info[2]).FromJust();
    int userId = To<int>(info[3]).FromJust();
    int reset = To<int>(info[4]).FromJust();
    Callback *callback = new Callback(info[5].As<Function>());

    AsyncQueueWorker(new TestBatchRetrieveWorker(callback, width,color,num,userId,reset,testQueue));
}

NAN_METHOD(spottingTestBatchDone) {
    
    String::Utf8Value resultsIdNAN(info[0]);
    string resultsId = string(*resultsIdNAN);
    
    vector<string> ids;
    vector<int> labels;
    Handle<Value> val;
    if (info[1]->IsArray()) {
      Handle<Array> jsArray = Handle<Array>::Cast(info[1]);
      for (unsigned int i = 0; i < jsArray->Length(); i++) {
        val = jsArray->Get(i);
        ids.push_back(string(*String::Utf8Value(val)));
      }
    }
    
    if (info[2]->IsArray()) {
      Handle<Array> jsArray = Handle<Array>::Cast(info[2]);
      for (unsigned int i = 0; i < jsArray->Length(); i++) {
        val = jsArray->Get(i);
        labels.push_back(val->Uint32Value());
      }
    }
    int resent = To<int>(info[3]).FromJust();
    int userId = To<int>(info[4]).FromJust();
    Callback *callback = new Callback(info[5].As<Function>());

    AsyncQueueWorker(new SpottingTestBatchUpdateWorker(callback,testQueue,resultsId,ids,labels,resent,userId));
}

NAN_METHOD(clearTestUsers) {
    
    Callback *callback = new Callback(info[0].As<Function>());
    AsyncQueueWorker(new ClearTestUsersWorker(callback,testQueue));
}

NAN_MODULE_INIT(Init) {
    
#ifndef TEST_MODE
    cattss = new CATTSS("/home/brian/intel_index/data/wordsEnWithNames.txt", 
                        "/home/brian/intel_index/data/gw_20p_wannot",
                        "data/queries.gtp");
#else
#ifdef TEST_MODE_LONG
    cattss = new CATTSS("test/lexicon.txt","test/","test/seg.gtp");
#else
    cattss = new CATTSS("/home/brian/intel_index/data/wordsEnWithNames.txt", 
                        "/home/brian/intel_index/data/gw_20p_wannot",
                        "data/onepage_queries.gtp");

#endif
#endif

    Nan::Set(target, New<String>("getNextBatch").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(getNextBatch)).ToLocalChecked());
    
    Nan::Set(target, New<String>("spottingBatchDone").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(spottingBatchDone)).ToLocalChecked());
    
    Nan::Set(target, New<String>("newExemplarsBatchDone").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(newExemplarsBatchDone)).ToLocalChecked());
    
    Nan::Set(target, New<String>("getNextTranscriptionBatch").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(getNextTranscriptionBatch)).ToLocalChecked());
    
    Nan::Set(target, New<String>("transcriptionBatchDone").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(transcriptionBatchDone)).ToLocalChecked());
    
    Nan::Set(target, New<String>("showCorpus").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(showCorpus)).ToLocalChecked());
    Nan::Set(target, New<String>("startSpotting").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(startSpotting)).ToLocalChecked());
    Nan::Set(target, New<String>("stopSpotting").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(stopSpotting)).ToLocalChecked());
    
    testQueue = new TestQueue();
    
    Nan::Set(target, New<String>("getNextTestBatch").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(getNextTestBatch)).ToLocalChecked());
    
    Nan::Set(target, New<String>("spottingTestBatchDone").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(spottingTestBatchDone)).ToLocalChecked());
    
    Nan::Set(target, New<String>("clearTestUsers").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(clearTestUsers)).ToLocalChecked());
}

NODE_MODULE(SpottingAddon, Init)

