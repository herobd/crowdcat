//#include "BatchRetrieveWorker.h"

#include <nan.h>
#include <iostream>
#include <assert.h>
#include "opencv2/highgui/highgui.hpp"

#include "CrowdCAT.h"
#include "BatchWraper.h"
using namespace Nan;
using namespace std;
using namespace v8;

class BatchRetrieveWorker : public AsyncWorker {
    public:
        BatchRetrieveWorker(Callback *callback, CrowdCAT* crowdcat, string userId, int width)
        : AsyncWorker(callback), userId(userId), width(width), crowdcat(crowdcat), batch(NULL) {}

        ~BatchRetrieveWorker() {}


        void Execute () {
            batch = crowdcat->getBatch(userId,width);
        }

        // We have the results, and we're back in the event loop.
        void HandleOKCallback () {
            batch->doCallback(callback);
            delete batch;
        }
    private:
        //output
        BatchWraper* batch;
        //input
        int width;
        string userId;
        CrowdCAT* crowdcat;
        
        
};


