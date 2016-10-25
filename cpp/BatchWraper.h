#ifndef BATCH_WRAPER
#define BATCH_WRAPER

#ifndef NO_NAN
#include <nan.h>
using namespace Nan;
using namespace v8;
#else
#define SPOTTINGS 1
#define NEW_EXEMPLARS 2
#define TRANSCRIPTION 3
#endif
using namespace std;

struct Location
{
    int pageId, x1,y1,x2,y2;
    Location() {}
    Location(int pageId, int x1, int y1, int x2, int y2) : pageId(pageId), 
                                                            x1(x1), 
                                                            y1(y1), 
                                                            x2(x2), 
                                                            y2(y2) {}
};

class BatchWraper
{
    public:
        virtual ~BatchWraper() {}
        //virtual BatchWraper(Batch* batch)=0;
#ifndef NO_NAN
        virtual void doCallback(Callback* callback)=0;
#else
        virtual int getType()=0;
        virtual void getSpottings(string* resId,string* ngram, vector<string>* ids, vector<Location>* locs, vector<string>* gt) {}
        virtual void getNewExemplars(string* batchId,vector<string>* ngrams, vector<Location>* locs) {}
        virtual void getTranscription(string* batchId,int* wordIndex, vector<SpottingPoint>* spottings, vector<string>* poss, bool* manual, string* gt) {}
#endif
};

class BatchWraperBlank : public BatchWraper
{
    public:
#ifndef NO_NAN
        virtual void doCallback(Callback* callback)
        {

            Nan:: HandleScope scope;
            Local<Value> argv[] = {
                Nan::Null(),
                Nan::Null(),
                Nan::Null(),
                Nan::Null(),
                Nan::Null(),
                Nan::Null(),
                Nan::Null(),
                Nan::Null()
            };

            callback->Call(8, argv);

        }
#else
        virtual int getType(){return 0;}
#endif
        
};
#endif
