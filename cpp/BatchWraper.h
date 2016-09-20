#ifndef BATCH_WRAPER
#define BATCH_WRAPER
#include <nan.h>

using namespace Nan;
using namespace std;
using namespace v8;

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
        virtual void doCallback(Callback* callback)=0;
};

class BatchWraperBlank : public BatchWraper
{
    public:
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
        
};
#endif
