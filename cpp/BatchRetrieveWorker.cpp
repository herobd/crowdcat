//#include "BatchRetrieveWorker.h"

#include <nan.h>
#include <iostream>
#include <assert.h>
#include "opencv2/highgui/highgui.hpp"

#include "CATTSS.h"
#include "BatchWraper.h"
using namespace Nan;
using namespace std;
using namespace v8;

class BatchRetrieveWorker : public AsyncWorker {
    public:
        BatchRetrieveWorker(Callback *callback, CATTSS* cattss, int width, int color, string prevNgram, int num)
        : AsyncWorker(callback), width(width), color(color), prevNgram(prevNgram), num(num), cattss(cattss), batch(NULL) {}

        ~BatchRetrieveWorker() {}


        void Execute () {
            batch = cattss->getBatch(num,width,color,prevNgram);
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
        int color;
        string prevNgram;
        int num;
        CATTSS* cattss;
        
        
};


