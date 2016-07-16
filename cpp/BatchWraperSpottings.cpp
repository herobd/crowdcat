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
    }
    //cout <<"readied batch of size "<<batchSize<<endl;
    delete batch;
}
void BatchWraperSpottings::doCallback(Callback *callback)
{
    Nan:: HandleScope scope;
    v8::Local<v8::Array> arr = Nan::New<v8::Array>(retId.size());
    for (unsigned int index=0; index<retId.size(); index++) {
	v8::Local<v8::Object> obj = Nan::New<v8::Object>();
	Nan::Set(obj, Nan::New("id").ToLocalChecked(), Nan::New(retId[index]).ToLocalChecked());
	Nan::Set(obj, Nan::New("data").ToLocalChecked(), Nan::New(retData[index]).ToLocalChecked());
	Nan::Set(arr, index, obj);
    }
    Local<Value> argv[] = {
	Nan::Null(),
	Nan::New("spottings").ToLocalChecked(),
	Nan::New(batchId).ToLocalChecked(),
	Nan::New(resultsId).ToLocalChecked(),
	Nan::New(ngram).ToLocalChecked(),
	Nan::New(arr)
    };
    /*Local<Value> argv[5];
    argv[0] = Nan::Null();
    argv[1] = Nan::New("spottings").ToLocalChecked();
    argv[2] = Nan::New(batchId).ToLocalChecked();
    argv[3] = Nan::New(ngram).ToLocalChecked();
    argv[4] = Nan::New(retArr);*/
    

    callback->Call(6, argv);
}
