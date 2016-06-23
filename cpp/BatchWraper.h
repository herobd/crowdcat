#ifndef BATCH_WRAPER
#define BATCH_WRAPER
#include <nan.h>

using namespace Nan;
using namespace std;
using namespace v8;
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
                Nan::Null()
            };

            callback->Call(6, argv);

        }
        
};
#endif
