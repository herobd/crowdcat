
#include "BatchWraperNewExemplars.h"
BatchWraperNewExemplars::BatchWraperNewExemplars(NewExemplarsBatch* newExemplars)
{
    base64::encoder E;
    vector<int> compression_params={CV_IMWRITE_PNG_COMPRESSION,9};
    

    batchId=to_string(newExemplars->getId());
    int batchSize = newExemplars->size();
    retData.resize(batchSize);
    retNgram.resize(batchSize);
    //auto iter=newExemplars.begin();
    for (int index=0; index<batchSize; index++) 
    {
        retNgram[index]=newExemplars->at(index).ngram;
        vector<uchar> outBuf;
        cv::imencode(".png",newExemplars->at(index).img(),outBuf,compression_params);//or should we have them look at the ngram image?
        stringstream ss;
        ss.write((char*)outBuf.data(),outBuf.size());
        stringstream encoded;
        E.encode(ss, encoded);
        string dataBase64 = encoded.str();
        retData[index]=dataBase64;
    }
}
void BatchWraperNewExemplars::doCallback(Callback *callback)
{
    Nan:: HandleScope scope;
    v8::Local<v8::Array> arr = Nan::New<v8::Array>(retData.size());
    for (unsigned int index=0; index<retData.size(); index++) {
	v8::Local<v8::Object> obj = Nan::New<v8::Object>();
	Nan::Set(obj, Nan::New("ngram").ToLocalChecked(), Nan::New(retNgram[index]).ToLocalChecked());
	Nan::Set(obj, Nan::New("data").ToLocalChecked(), Nan::New(retData[index]).ToLocalChecked());
	Nan::Set(arr, index, obj);
    }
    Local<Value> argv[] = {
	Nan::Null(),
	Nan::New("newExemplars").ToLocalChecked(),
	Nan::New(batchId).ToLocalChecked(),
	Nan::New(arr),
	Nan::Null(),
	Nan::Null()
    };

    callback->Call(6, argv);
}
