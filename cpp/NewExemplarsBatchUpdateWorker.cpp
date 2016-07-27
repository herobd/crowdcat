#include <nan.h>
#include <iostream>
#include <assert.h>

#include "CATTSS.h"

using namespace Nan;
using namespace std;
using namespace v8;

class NewExemplarsBatchUpdateWorker : public AsyncWorker {
    public:
        NewExemplarsBatchUpdateWorker(Callback *callback, CATTSS* cattss, string resultsId, vector<int> labels, int resent)
        : AsyncWorker(callback), cattss(cattss),
          resultsId(resultsId), labels(labels), resent(resent) {}

        ~NewExemplarsBatchUpdateWorker() {}


        void Execute () 
        {
            cattss->updateNewExemplars(resultsId,labels,resent);
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
        CATTSS* cattss;
        string resultsId;
        vector<int> labels;
        int resent;
        
};
