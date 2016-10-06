
#include <nan.h>
#include <functional>
#include <iostream>
#include <assert.h>
#define BUFFERSIZE 65536

#include "CATTSS.h"

using namespace Nan;
using namespace std;
using namespace v8;

#include "BatchRetrieveWorker.cpp"
#include "SpecialBatchRetrieveWorker.cpp"
#include "SpottingBatchUpdateWorker.cpp"
#include "NewExemplarsBatchUpdateWorker.cpp"
#include "TranscriptionBatchUpdateWorker.cpp"
#include "MiscWorker.cpp"

#include "TrainingInstances.h"
#include "TestingInstances.h"

//test
#include "spotting.h"

CATTSS* cattss;
TrainingInstances* trainingInstances;
map<string, TestingInstances*> testingInstances;

NAN_METHOD(getNextBatch) {
    int width = To<int>(info[0]).FromJust();
    int color = To<int>(info[1]).FromJust();
    //string prevNgram = To<string>(info[2]).FromJust();
    //v8::String::Utf8Value str(args[0]->ToString());
    string prevNgram;
    if (info[2]->IsString())
    {
        v8::String::Utf8Value str(info[2]->ToString());
        prevNgram = string(*str);
        
    }
    
    //String::Utf8Value str = To<String::Utf8Value>(info[2]).FromJust();
    
    int num = To<int>(info[3]).FromJust();
    
    Callback *callback = new Callback(info[4].As<Function>());

    AsyncQueueWorker(new BatchRetrieveWorker(callback,cattss, width,color,prevNgram,num));
}
NAN_METHOD(getNextTrainingBatch) {
    int width = To<int>(info[0]).FromJust();
    int color = To<int>(info[1]).FromJust();
    string prevNgram;
    if (info[2]->IsString())
    {
        v8::String::Utf8Value str(info[2]->ToString());
        prevNgram = string(*str);
        
    }
    int num = To<int>(info[3]).FromJust();
    int trainingNum = To<int>(info[4]).FromJust();
    
    Callback *callback = new Callback(info[5].As<Function>());
    if (trainingInstances==NULL)
        trainingInstances = new TrainingInstances(); 
    AsyncQueueWorker(new SpecialBatchRetrieveWorker(callback,trainingInstances, width,color,prevNgram,num,trainingNum));
}
NAN_METHOD(getNextTestingBatch) {
    if (info[2]->IsString())
    {
        int width = To<int>(info[0]).FromJust();
        int color = To<int>(info[1]).FromJust();
        string prevNgram;
        v8::String::Utf8Value str(info[2]->ToString());
        prevNgram = string(*str);
            
        int num = To<int>(info[3]).FromJust();
        int testingNum = To<int>(info[4]).FromJust();
        
        v8::String::Utf8Value str2(info[5]->ToString());
        string datasetName = string(*str2);

        Callback *callback = new Callback(info[6].As<Function>());

        AsyncQueueWorker(new SpecialBatchRetrieveWorker(callback,testingInstances[datasetName], width,color,prevNgram,num,testingNum));
    }
}

NAN_METHOD(spottingBatchDone) {
    //string batchId = To<string>(info[0]).FromJust();
    //string resultsId = To<string>(info[0]).FromJust();
    v8::String::Utf8Value resultsIdNAN(info[0]);
    string resultsId = string(*resultsIdNAN);
    
    vector<string> ids;
    vector<int> labels;
    Handle<Value> val;
    if (info[1]->IsArray()) {
      Handle<Array> jsArray = Handle<Array>::Cast(info[1]);
      for (unsigned int i = 0; i < jsArray->Length(); i++) {
        val = jsArray->Get(i);
        ids.push_back(string(*v8::String::Utf8Value(val)));
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

    //Which way is better, synchronous (enqueue), or async?
    //Will waiting at queue lock or spawning worker  thread be more detrimental?

    //cattss->updateSpottings(resultsId,ids,labels,resent);
    //OR
    /**/
    Callback *callback = new Callback(info[4].As<Function>());

    AsyncQueueWorker(new SpottingBatchUpdateWorker(callback,cattss,resultsId,ids,labels,resent));
    /**/
}

NAN_METHOD(newExemplarsBatchDone) {
    //string batchId = To<string>(info[0]).FromJust();
    //string resultsId = To<string>(info[0]).FromJust();
    v8::String::Utf8Value resultsIdNAN(info[0]);
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
    v8::String::Utf8Value resultsIdNAN(info[0]);
    string id = string(*resultsIdNAN);
    v8::String::Utf8Value resultNAN(info[1]);
    string transcription = string(*resultNAN);
    
    Callback *callback = new Callback(info[2].As<Function>());

    AsyncQueueWorker(new TranscriptionBatchUpdateWorker(callback,cattss,id,transcription));
}
NAN_METHOD(manualBatchDone) {
    //string batchId = To<string>(info[0]).FromJust();
    //string resultsId = To<string>(info[0]).FromJust();
    v8::String::Utf8Value resultsIdNAN(info[0]);
    string id = string(*resultsIdNAN);
    v8::String::Utf8Value resultNAN(info[1]);
    string transcription = string(*resultNAN);
    
    Callback *callback = new Callback(info[2].As<Function>());

    AsyncQueueWorker(new TranscriptionBatchUpdateWorker(callback,cattss,id,transcription,true));
}

NAN_METHOD(manualFinish) {
    //bool restrictWords = To<bool>(info[0]).FromJust();
    Callback *callback = new Callback(info[0].As<Function>());

    AsyncQueueWorker(new MiscWorker(callback,cattss, "manualFinish"));
}
NAN_METHOD(showCorpus) {
    Callback *callback = new Callback(info[0].As<Function>());

    AsyncQueueWorker(new MiscWorker(callback,cattss, "showCorpus"));
}
/*NAN_METHOD(showProgress) {
    int height = To<int>(info[0]).FromJust();
    int width = To<int>(info[1]).FromJust();
    int milli = To<int>(info[2]).FromJust();
    Callback *callback = new Callback(info[3].As<Function>());

    AsyncQueueWorker(new MiscWorker(callback,cattss, "showProgress:"+to_string(height)+","+to_string(width)+","+to_string(milli)));
}
NAN_METHOD(startSpotting) {
    int num = To<int>(info[0]).FromJust();
    //cout<<"startSpotting("<<num<<")"<<endl;
    Callback *callback = new Callback(info[1].As<Function>());

    AsyncQueueWorker(new MiscWorker(callback,cattss, "startSpotting:"+to_string(num)));
}*/
NAN_METHOD(start) {
    v8::String::Utf8Value lexiconFileNAN(info[0]);
    string lexiconFile = string(*lexiconFileNAN);
    v8::String::Utf8Value pageImageDirNAN(info[1]);
    string pageImageDir = string(*pageImageDirNAN);
    v8::String::Utf8Value segmentationFileNAN(info[2]);
    string segmentationFile = string(*segmentationFileNAN);
    v8::String::Utf8Value spottingModelPrefixNAN(info[3]);
    string spottingModelPrefix = string(*spottingModelPrefixNAN);
    v8::String::Utf8Value savePrefixNAN(info[4]);
    string savePrefix = string(*savePrefixNAN);
    int numSpottingThreads = To<int>(info[5]).FromJust();
    int numTaskThreads = To<int>(info[6]).FromJust();
    int height = To<int>(info[7]).FromJust();
    int width = To<int>(info[8]).FromJust();
    int milli = To<int>(info[9]).FromJust();
    cattss = new CATTSS(lexiconFile,
                        pageImageDir,
                        segmentationFile,
                        spottingModelPrefix,
                        savePrefix,
                        numSpottingThreads,
                        numTaskThreads,
                        height,
                        width,
                        milli);
                        
}
NAN_METHOD(stopSpotting) {
    Callback *callback = new Callback(info[0].As<Function>());

    AsyncQueueWorker(new MiscWorker(callback,cattss, "stopSpotting"));
}

NAN_METHOD(loadLabeledSpotting) {
    if (info[0]->IsString() && info[1]->IsString())
    {
        v8::String::Utf8Value str0(info[0]->ToString());
        string datasetName = string(*str0);
        v8::String::Utf8Value str1(info[1]->ToString());
        string ngram = string(*str1);
        bool label = To<bool>(info[2]).FromJust();
        int pageId = To<int>(info[3]).FromJust();
        int x1 = To<int>(info[4]).FromJust();
        int y1 = To<int>(info[5]).FromJust();
        int x2 = To<int>(info[6]).FromJust();
        int y2 = To<int>(info[7]).FromJust();
        
        if (testingInstances[datasetName]==NULL)
        {
            assert(cattss!=NULL);
            testingInstances[datasetName]=new TestingInstances(cattss->getCorpus());
        }
        testingInstances[datasetName]->addSpotting(ngram,label,pageId,x1,y1,x2,y2);    
    }
}
NAN_METHOD(loadLabeledTrans) {
    if (info[0]->IsString() && info[1]->IsString())
    {
        v8::String::Utf8Value str0(info[0]->ToString());
        string datasetName = string(*str0);
        v8::String::Utf8Value str1(info[1]->ToString());
        string label = string(*str1);
        vector<string> poss;
        Handle<Value> val;
        Handle<Value> val2;
        if (info[2]->IsArray()) {
          Handle<Array> jsArray = Handle<Array>::Cast(info[2]);
          for (unsigned int i = 0; i < jsArray->Length(); i++) {
            val = jsArray->Get(i);
            poss.push_back(string(*v8::String::Utf8Value(val)));
          }
        }
        vector<string> ngramSpots;
        if (info[3]->IsArray()) {
          Handle<Array> jsArray = Handle<Array>::Cast(info[3]);
          for (unsigned int i = 0; i < jsArray->Length(); i++) {
            val = jsArray->Get(i);
            ngramSpots.push_back(string(*v8::String::Utf8Value(val)));
          }
        }


        //multimap<string,Location> spots;//fill
        vector<Spotting> spots;//fill
        if (info[4]->IsArray()) {
          Handle<Array> jsArray = Handle<Array>::Cast(info[4]);
          assert(jsArray->Length() == ngramSpots.size());
          for (unsigned int i = 0; i < jsArray->Length(); i++) {
            val = jsArray->Get(i);
            if (val->IsArray()) {
              Handle<Array> jsArray2 = Handle<Array>::Cast(val);
              assert(jsArray2->Length()==5);
              int sx1,sy1,sx2,sy2,sid;
              val2 = jsArray2->Get(0);
              sx1=val2->Uint32Value();
              val2 = jsArray2->Get(1);
              sy1=val2->Uint32Value();
              val2 = jsArray2->Get(2);
              sx2=val2->Uint32Value();
              val2 = jsArray2->Get(3);
              sy2=val2->Uint32Value();
              val2 = jsArray2->Get(4);
              sid=val2->Uint32Value();
              //spots.insert( make_pair(ngramSpots[i],Location(-1,sx1,sy1,sx2,sy2)) );
              Spotting spotting (sx1,sy1,sx2,sy2,-1,NULL,ngramSpots[i],0);
              spotting.id=sid;
              spots.push_back(spotting);
            }
            else assert(false);
          }
        }
            else assert(false);

        int wordIdx = To<int>(info[5]).FromJust();
        //int x1 = To<int>(info[5]).FromJust();
        //int y1 = To<int>(info[6]).FromJust();
        //int x2 = To<int>(info[7]).FromJust();
        //int y2 = To<int>(info[8]).FromJust();
        bool manual = To<bool>(info[6]).FromJust();
        
        if (testingInstances[datasetName]==NULL)
        {
            assert(cattss!=NULL);
            testingInstances[datasetName]=new TestingInstances(cattss->getCorpus());
        }
        testingInstances[datasetName]->addTrans(label,poss,spots,wordIdx,manual);    
    }
}
NAN_METHOD(testingLabelsAllLoaded) {
    if (info[0]->IsString())
    {
        v8::String::Utf8Value str0(info[0]->ToString());
        string datasetName = string(*str0);
        testingInstances[datasetName]->allLoaded();
    }
}
/*NAN_METHOD(getNextTestBatch) {
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
    
    v8::String::Utf8Value resultsIdNAN(info[0]);
    string resultsId = string(*resultsIdNAN);
    
    vector<string> ids;
    vector<int> labels;
    Handle<Value> val;
    if (info[1]->IsArray()) {
      Handle<Array> jsArray = Handle<Array>::Cast(info[1]);
      for (unsigned int i = 0; i < jsArray->Length(); i++) {
        val = jsArray->Get(i);
        ids.push_back(string(*v8::String::Utf8Value(val)));
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
}*/

NAN_MODULE_INIT(Init) {
    signal(SIGPIPE, SIG_IGN);    
    cattss=NULL;
    trainingInstances=NULL;
//#ifndef TEST_MODE
    //cattss = new CATTSS("/home/brian/intel_index/data/wordsEnWithNames.txt", 
    //                    "/home/brian/intel_index/data/gw_20p_wannot",
    //                    "/home/brian/intel_index/EmbAttSpotter/test/queries_test.gtp");
                        //"data/queries.gtp");
/*#else
#ifdef TEST_MODE_LONG
    cattss = new CATTSS("test/lexicon.txt","test/","test/seg.gtp");
#else
    cattss = new CATTSS("/home/brian/intel_index/data/wordsEnWithNames.txt", 
                        "/home/brian/intel_index/data/gw_20p_wannot",
                        "data/twopage_queries.gtp");

#endif
#endif*/

    Nan::Set(target, New<v8::String>("getNextBatch").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(getNextBatch)).ToLocalChecked());
    
    Nan::Set(target, New<v8::String>("spottingBatchDone").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(spottingBatchDone)).ToLocalChecked());
    
    Nan::Set(target, New<v8::String>("newExemplarsBatchDone").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(newExemplarsBatchDone)).ToLocalChecked());
    
    Nan::Set(target, New<v8::String>("getNextTranscriptionBatch").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(getNextTranscriptionBatch)).ToLocalChecked());
    
    Nan::Set(target, New<v8::String>("transcriptionBatchDone").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(transcriptionBatchDone)).ToLocalChecked());
    
    Nan::Set(target, New<v8::String>("manualBatchDone").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(manualBatchDone)).ToLocalChecked());
    
    Nan::Set(target, New<v8::String>("manualFinish").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(manualFinish)).ToLocalChecked());
    Nan::Set(target, New<v8::String>("showCorpus").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(showCorpus)).ToLocalChecked());
    //Nan::Set(target, New<v8::String>("showProgress").ToLocalChecked(),
    //    GetFunction(New<FunctionTemplate>(showProgress)).ToLocalChecked());

    Nan::Set(target, New<v8::String>("loadLabeledSpotting").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(loadLabeledSpotting)).ToLocalChecked());
    Nan::Set(target, New<v8::String>("loadLabeledTrans").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(loadLabeledTrans)).ToLocalChecked());
    Nan::Set(target, New<v8::String>("testingLabelsAllLoaded").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(testingLabelsAllLoaded)).ToLocalChecked());
    Nan::Set(target, New<v8::String>("getNextTestingBatch").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(getNextTestingBatch)).ToLocalChecked());

    //Nan::Set(target, New<v8::String>("startSpotting").ToLocalChecked(),
    //    GetFunction(New<FunctionTemplate>(startSpotting)).ToLocalChecked());
    Nan::Set(target, New<v8::String>("stopSpotting").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(stopSpotting)).ToLocalChecked());
    
    Nan::Set(target, New<v8::String>("start").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(start)).ToLocalChecked());
    /*testQueue = new TestQueue();
    
    Nan::Set(target, New<v8::String>("getNextTestBatch").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(getNextTestBatch)).ToLocalChecked());
    
    Nan::Set(target, New<v8::String>("spottingTestBatchDone").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(spottingTestBatchDone)).ToLocalChecked());
    
    Nan::Set(target, New<v8::String>("clearTestUsers").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(clearTestUsers)).ToLocalChecked());*/
    Nan::Set(target, New<v8::String>("getNextTrainingBatch").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(getNextTrainingBatch)).ToLocalChecked());
}

NODE_MODULE(SpottingAddon, Init)

