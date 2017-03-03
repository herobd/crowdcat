#include "BatchWraperSpottings.h"
BatchWraperSpottings::BatchWraperSpottings(SpottingsBatch* batch)
{
    base64::encoder E;
    vector<int> compression_params={CV_IMWRITE_PNG_COMPRESSION,9};
    

    batchId=to_string(batch->batchId);
    resultsId=to_string(batch->spottingResultsId);
    ngram=batch->ngram;
    int batchSize = batch->size();
    retData.resize(batchSize);
    retId.resize(batchSize);
    locations.resize(batchSize);
    gt.resize(batchSize);
#ifdef NO_NAN
    images.resize(batchSize);
#endif
    for (int index=0; index<batchSize; index++) 
    {
        retId[index]=to_string(batch->at(index).id);
        vector<uchar> outBuf;
        //cout <<"encoding..."<<endl;
        //cv::imshow("batch im",batch->at(index).img());
        //cv::waitKey();
        cv::imencode(".png",batch->at(index).img(),outBuf,compression_params);
        //cout <<"done"<<endl;
        stringstream ss;
        ss.write((char*)outBuf.data(),outBuf.size());
        stringstream encoded;
        E.encode(ss, encoded);
        string dataBase64 = encoded.str();
        retData[index]=dataBase64;

        locations[index]=Location(  batch->at(index).pageId,
                                    batch->at(index).tlx,
                                    batch->at(index).tly,
                                    batch->at(index).brx,
                                    batch->at(index).bry
                                 );
        if (batch->at(index).gt!=UNKNOWN_GT)
            gt[index]=batch->at(index).gt?"1":"0";
        else
            gt[index]="UNKNOWN";
#ifdef NO_NAN
        images[index] = batch->at(index).img();
#endif
    }
#ifdef TEST_MODE
    precAtPull = batch->precAtPull;
    precAcceptT = batch->precAcceptT;
    precRejectT = batch->precRejectT;
    precBetweenT = batch->precBetweenT;
    countAcceptT = batch->countAcceptT;
    countRejectT = batch->countRejectT;
    countBetweenT = batch->countBetweenT;
#endif
    //cout <<"readied batch of size "<<batchSize<<endl;
    delete batch;
}
#ifndef NO_NAN
void BatchWraperSpottings::doCallback(Callback *callback)
{
    Nan:: HandleScope scope;
    v8::Local<v8::Array> arr = Nan::New<v8::Array>(retId.size());
    v8::Local<v8::Array> locs = Nan::New<v8::Array>(locations.size());
    v8::Local<v8::Array> gtArr = Nan::New<v8::Array>(gt.size());
    for (unsigned int index=0; index<retId.size(); index++) {
	v8::Local<v8::Object> obj = Nan::New<v8::Object>();
	Nan::Set(obj, Nan::New("id").ToLocalChecked(), Nan::New(retId[index]).ToLocalChecked());
	Nan::Set(obj, Nan::New("data").ToLocalChecked(), Nan::New(retData[index]).ToLocalChecked());
	Nan::Set(arr, index, obj);

        Location l=locations[index];
        v8::Local<v8::Object> loc = Nan::New<v8::Object>();
        loc->Set(Nan::New("page").ToLocalChecked(), Nan::New(l.pageId));
        loc->Set(Nan::New("x1").ToLocalChecked(), Nan::New(l.x1));
        loc->Set(Nan::New("y1").ToLocalChecked(), Nan::New(l.y1));
        loc->Set(Nan::New("x2").ToLocalChecked(), Nan::New(l.x2));
        loc->Set(Nan::New("y2").ToLocalChecked(), Nan::New(l.y2));

        Nan::Set(locs, index, loc);

        Nan::Set(gtArr, index, Nan::New(gt[index]).ToLocalChecked());
    }

#ifdef TEST_MODE
    v8::Local<v8::Object> debug = Nan::New<v8::Object>();
    debug->Set(Nan::New("precAtPull").ToLocalChecked(), Nan::New(precAtPull));
    debug->Set(Nan::New("precAcceptT").ToLocalChecked(), Nan::New(precAcceptT));
    debug->Set(Nan::New("precRejectT").ToLocalChecked(), Nan::New(precRejectT));
    debug->Set(Nan::New("precBetweenT").ToLocalChecked(), Nan::New(precBetweenT));
    debug->Set(Nan::New("countAcceptT").ToLocalChecked(), Nan::New(countAcceptT));
    debug->Set(Nan::New("countRejectT").ToLocalChecked(), Nan::New(countRejectT));
    debug->Set(Nan::New("countBetweenT").ToLocalChecked(), Nan::New(countBetweenT));
#endif

    Local<Value> argv[] = {
#ifdef TEST_MODE
        debug,
#else
	Nan::Null(),
#endif
	Nan::New("spottings").ToLocalChecked(),
	Nan::New(batchId).ToLocalChecked(),
	Nan::New(resultsId).ToLocalChecked(),
	Nan::New(ngram).ToLocalChecked(),
	Nan::New(arr),
        locs,
        Nan::New(gtArr)
    };
    /*Local<Value> argv[5];
    argv[0] = Nan::Null();
    argv[1] = Nan::New("spottings").ToLocalChecked();
    argv[2] = Nan::New(batchId).ToLocalChecked();
    argv[3] = Nan::New(ngram).ToLocalChecked();
    argv[4] = Nan::New(retArr);*/
    

    callback->Call(8, argv);
}
#else
void BatchWraperSpottings::getSpottings(string* resId,string* ngram, vector<string>* ids, vector<Location>* locs, vector<string>* gt)
{
    *resId = resultsId;
    *ids=retId;
    *ngram=this->ngram;
    *locs=locations;
    *gt=this->gt;
}
#endif
