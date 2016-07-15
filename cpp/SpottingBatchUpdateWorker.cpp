#include <nan.h>
#include <iostream>
#include <assert.h>
#include "opencv2/highgui/highgui.hpp"

#include "CATTSS.h"

using namespace Nan;
using namespace std;
using namespace v8;

class SpottingBatchUpdateWorker : public AsyncWorker {
    public:
        SpottingBatchUpdateWorker(Callback *callback, CATTSS* cattss, string resultsId, vector<string> ids, vector<int> labels, int resent)
        : AsyncWorker(callback), cattss(cattss),
          resultsId(resultsId), ids(ids), labels(labels), resent(resent) {}

        ~SpottingBatchUpdateWorker() {}


        void Execute () 
        {
            cattss->updateSpottings(resultsId,ids,labels,resent);
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
        vector<string> ids;
        vector<int> labels;
        int resent;
        
};
