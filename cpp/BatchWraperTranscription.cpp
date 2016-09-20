
#include "BatchWraperTranscription.h"
BatchWraperTranscription::BatchWraperTranscription(TranscribeBatch* batch)
{
    base64::encoder E;
    vector<int> compression_params={CV_IMWRITE_PNG_COMPRESSION,9};
    
    manual = batch->isManual();
    batchId=to_string(batch->getId());
    vector<uchar> outBuf;
    cv::imencode(".png",batch->getImage(),outBuf,compression_params);
    stringstream ss;
    ss.write((char*)outBuf.data(),outBuf.size());
    stringstream encoded;
    E.encode(ss, encoded);
    string dataBase64 = encoded.str();
    wordImgStr=dataBase64;
    /*outBuf.clear();
    cv::imencode(".png",batch->getTextImage(),outBuf,compression_params);
    stringstream ss2;
    ss2.write((char*)outBuf.data(),outBuf.size());
    stringstream encoded2;
    E.encode(ss2, encoded2);
    dataBase64 = encoded2.str();
    ngramImgStr=dataBase64;*/
    retPoss = batch->getPossibilities();
    spottings = batch->getSpottingPoints();
    //delete batch;
    wordIndex=to_string(batch->getBackPointer()->getSpottingIndex());
    gt=batch->getBackPointer()->getGT();
}
void BatchWraperTranscription::doCallback(Callback *callback)
{
    Nan:: HandleScope scope;
    v8::Local<v8::Array> possibilities = Nan::New<v8::Array>(retPoss.size());
    for (unsigned int index=0; index<retPoss.size(); index++) {
	Nan::Set(possibilities, index, Nan::New(retPoss[index]).ToLocalChecked());
    }
    v8::Local<v8::Array> spottingsArr = Nan::New<v8::Array>(spottings.size());
    for (unsigned int index=0; index<spottings.size(); index++) {
	v8::Local<v8::Object> obj = Nan::New<v8::Object>();
	Nan::Set(obj, Nan::New("id").ToLocalChecked(), Nan::New(spottings[index].getId()).ToLocalChecked());
	Nan::Set(obj, Nan::New("x").ToLocalChecked(), Nan::New(spottings[index].getX()).ToLocalChecked());
	Nan::Set(obj, Nan::New("ngram").ToLocalChecked(), Nan::New(spottings[index].getNgram()).ToLocalChecked());
	Nan::Set(obj, Nan::New("color").ToLocalChecked(), Nan::New(spottings[index].getColor()).ToLocalChecked());

        v8::Local<v8::Object> locNgram = Nan::New<v8::Object>();
        locNgram->Set(Nan::New("page").ToLocalChecked(), Nan::New(spottings[index].page));
        locNgram->Set(Nan::New("x1").ToLocalChecked(), Nan::New(spottings[index].x1));
        locNgram->Set(Nan::New("y1").ToLocalChecked(), Nan::New(spottings[index].y1));
        locNgram->Set(Nan::New("x2").ToLocalChecked(), Nan::New(spottings[index].x2));
        locNgram->Set(Nan::New("y2").ToLocalChecked(), Nan::New(spottings[index].y2));
        Nan::Set(obj, Nan::New("loc").ToLocalChecked(), locNgram);

	Nan::Set(spottingsArr, index, obj);
    }
    v8::Local<v8::Object> loc = Nan::New<v8::Object>();
    Nan::Set(loc, Nan::New("wordIndex").ToLocalChecked(), Nan::New(wordIndex).ToLocalChecked());
    string batchType;
    if (manual)
       batchType = "manual";
    else
       batchType = "transcription";
    Local<Value> argv[] = {
	Nan::Null(),
	Nan::New(batchType).ToLocalChecked(),
	Nan::New(batchId).ToLocalChecked(),
	Nan::New(wordImgStr).ToLocalChecked(),
	//Nan::New(ngramImgStr).ToLocalChecked(),
	Nan::New(spottingsArr),
	Nan::New(possibilities),
        Nan::New(loc),
        Nan::New(gt).ToLocalChecked()
    };
    

    callback->Call(8, argv);
}
