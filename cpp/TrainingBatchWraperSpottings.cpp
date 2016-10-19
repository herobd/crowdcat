#include "TrainingBatchWraperSpottings.h"
TrainingBatchWraperSpottings::TrainingBatchWraperSpottings(SpottingsBatch* batch, string correct, string instructions, bool last) : BatchWraperSpottings(batch), correct(correct), instructions(instructions), last(last)
{
}
void TrainingBatchWraperSpottings::doCallback(Callback *callback)
{
    Nan:: HandleScope scope;
    v8::Local<v8::Array> arr = Nan::New<v8::Array>(retId.size());
    v8::Local<v8::Array> locs = Nan::New<v8::Array>(locations.size());
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
    }
    Local<Value> argv[] = {
	Nan::Null(),
	Nan::New("spottings").ToLocalChecked(),
	Nan::New(batchId).ToLocalChecked(),
	Nan::New(resultsId).ToLocalChecked(),
	Nan::New(ngram).ToLocalChecked(),
	Nan::New(arr),
        Nan::New(locs),
        Nan::New(correct).ToLocalChecked(),
        Nan::New(instructions).ToLocalChecked(),
        Nan::New(last)
    };
    /*Local<Value> argv[5];
    argv[0] = Nan::Null();
    argv[1] = Nan::New("spottings").ToLocalChecked();
    argv[2] = Nan::New(batchId).ToLocalChecked();
    argv[3] = Nan::New(ngram).ToLocalChecked();
    argv[4] = Nan::New(retArr);*/
    

    callback->Call(10, argv);
}
