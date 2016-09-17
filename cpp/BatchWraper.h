#ifndef BATCH_WRAPER
#define BATCH_WRAPER
#include <nan.h>

using namespace Nan;
using namespace std;
using namespace v8;

struct Location
{
    string pageId, x1,y1,x2,y2;
    Location() {}
    Location(int pageId, int x1, int y1, int x2, int y2) : pageId(to_string(pageId)), 
                                                            x1(to_string(x1)), 
                                                            y1(to_string(y1)), 
                                                            x2(to_string(x2)), 
                                                            y2(to_string(y2)) {}
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
