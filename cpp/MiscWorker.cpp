
#include <nan.h>
#include <iostream>
#include <assert.h>
#include "opencv2/highgui/highgui.hpp"

#include "CrowdCAT.h"
#include "Knowledge.h"

using namespace Nan;
using namespace std;
using namespace v8;

class MiscWorker : public AsyncWorker {
    public:
        MiscWorker(Callback *callback, CrowdCAT* crowdcat, string task)
        : AsyncWorker(callback), task(task), crowdcat(crowdcat) {}

        ~MiscWorker() {}


        void Execute () {
            if (crowdcat!=NULL)
                crowdcat->misc(task);
        }

        // We have the results, and we're back in the event loop.
        void HandleOKCallback () {
            Nan:: HandleScope scope;
            Local<Value> argv[] = {
                Nan::Null()
            };
            //cout <<"calling back"<<endl;
            callback->Call(1, argv);

        }
    private:
        CrowdCAT* crowdcat;
        string task;
        
};
