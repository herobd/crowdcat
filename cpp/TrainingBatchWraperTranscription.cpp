
#include "TrainingBatchWraperTranscription.h"
TrainingBatchWraperTranscription::TrainingBatchWraperTranscription(TranscribeBatch* batch, string correct, string instructions, bool last) : BatchWraperTranscription(batch), correct(correct), instructions(instructions), last(last)
{
}
void TrainingBatchWraperTranscription::doCallback(Callback *callback)
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
	Nan::Set(obj, Nan::New("scale").ToLocalChecked(), Nan::New(scale)); //for convienence this is here
	Nan::Set(obj, Nan::New("ngram").ToLocalChecked(), Nan::New(spottings[index].getNgram()).ToLocalChecked());
	Nan::Set(obj, Nan::New("color").ToLocalChecked(), Nan::New(spottings[index].getColor()).ToLocalChecked());
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
        Nan::New(correct).ToLocalChecked(),
        Nan::New(instructions).ToLocalChecked(),
        Nan::New(last)
    };
    

    callback->Call(10, argv);
}
