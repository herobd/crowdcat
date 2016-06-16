
#include "BatchWraperTranscription.h"
BatchWraperTranscription::BatchWraperTranscription(TranscribeBatch* batch)
{
    base64::encoder E;
    vector<int> compression_params={CV_IMWRITE_PNG_COMPRESSION,9};
    

    batchId=to_string(batch->getId());
    vector<uchar> outBuf;
    cv::imencode(".png",batch->getImage(),outBuf,compression_params);
    stringstream ss;
    ss.write((char*)outBuf.data(),outBuf.size());
    stringstream encoded;
    E.encode(ss, encoded);
    string dataBase64 = encoded.str();
    wordImgStr=dataBase64;
    outBuf.clear();
    cv::imencode(".png",batch->getTextImage(),outBuf,compression_params);
    stringstream ss;
    ss.write((char*)outBuf.data(),outBuf.size());
    stringstream encoded;
    E.encode(ss, encoded);
    dataBase64 = encoded.str();
    ngramImgStr=dataBase64;
    retPoss = batch->getPossiblilties();
    //delete batch;
}
void BatchWraperTranscription::doCallback(Callback *callback)
{
    Nan:: HandleScope scope;
    v8::Local<v8::Array> possibilities = Nan::New<v8::Array>(retPoss.size());
    for (unsigned int index=0; index<retId.size(); index++) {
	Nan::Set(arr, index, Nan::New(retPoss[index]).ToLocalChecked());
    }
    Local<Value> argv[] = {
	Nan::Null(),
	Nan::New("transcription").ToLocalChecked(),
	Nan::New(batchId).ToLocalChecked(),
	Nan::New(wordImgStr).ToLocalChecked(),
	Nan::New(ngramImgStr).ToLocalChecked(),
	Nan::New(possibilities)
    };
    

    callback->Call(6, argv);
}
