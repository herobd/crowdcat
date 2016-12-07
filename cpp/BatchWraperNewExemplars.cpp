
#include "BatchWraperNewExemplars.h"
BatchWraperNewExemplars::BatchWraperNewExemplars(NewExemplarsBatch* newExemplars)
{
    base64::encoder E;
    vector<int> compression_params={CV_IMWRITE_PNG_COMPRESSION,9};
    

    batchId=to_string(newExemplars->getId());
    int batchSize = newExemplars->size();
    retData.resize(batchSize);
    retNgram.resize(batchSize);
    locations.resize(batchSize);
#ifdef NO_NAN
    images.resize(batchSize);
#endif
    //auto iter=newExemplars.begin();
    for (int index=0; index<batchSize; index++) 
    {
        retNgram[index]=newExemplars->at(index).ngram;
        vector<uchar> outBuf;
        cv::imencode(".png",newExemplars->at(index).ngramImg(),outBuf,compression_params);//or should we have them look at the ngram image?
        stringstream ss;
        ss.write((char*)outBuf.data(),outBuf.size());
        stringstream encoded;
        E.encode(ss, encoded);
        string dataBase64 = encoded.str();
        retData[index]=dataBase64;

        locations[index]=Location(  newExemplars->at(index).pageId,
                                    newExemplars->at(index).tlx,
                                    newExemplars->at(index).tly,
                                    newExemplars->at(index).brx,
                                    newExemplars->at(index).bry
                                 );
#ifdef NO_NAN
        images[index]=newExemplars->at(index).ngramImg();
#endif
    }

    
}
#ifndef NO_NAN
void BatchWraperNewExemplars::doCallback(Callback *callback)
{
    Nan:: HandleScope scope;
    v8::Local<v8::Array> arr = Nan::New<v8::Array>(retData.size());
    v8::Local<v8::Array> locs = Nan::New<v8::Array>(locations.size());
    for (unsigned int index=0; index<retData.size(); index++) {
	v8::Local<v8::Object> obj = Nan::New<v8::Object>();
	Nan::Set(obj, Nan::New("ngram").ToLocalChecked(), Nan::New(retNgram[index]).ToLocalChecked());
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
	Nan::New("newExemplars").ToLocalChecked(),
	Nan::New(batchId).ToLocalChecked(),
	Nan::New(arr),
	Nan::Null(),
	Nan::Null(),
        Nan::New(locs),
        Nan::New("UNKNOWN").ToLocalChecked()
    };

    callback->Call(8, argv);
}
#else
void BatchWraperNewExemplars::getNewExemplars(string* batchId,vector<string>* ngrams, vector<Location>* locs)
{
    *batchId=this->batchId;
    *ngrams=retNgram;
    *locs=locations;
}
#endif
