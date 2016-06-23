//#include "BatchRetrieveWorker.h"

#include <nan.h>
#include <iostream>
#include <assert.h>
#include "opencv2/highgui/highgui.hpp"

#include "MasterQueue.h"
#include "BatchWraper.h"
#include "BatchWraperSpottings.h"
#include "BatchWraperTranscription.h"
using namespace Nan;
using namespace std;
using namespace v8;

class BatchRetrieveWorker : public AsyncWorker {
    public:
        BatchRetrieveWorker(Callback *callback, int width, int color, string prevNgram, int num, MasterQueue* masterQueue)
        : AsyncWorker(callback), width(width), color(color), prevNgram(prevNgram), num(num), masterQueue(masterQueue), batch(NULL) {}

        ~BatchRetrieveWorker() {}


        void Execute () {

            if (color==-1)
            {
                cout <<"get trans batch"<<endl;
                TranscribeBatch* b = masterQueue->getTranscriptionBatch(width);
                if (b!=NULL)
                    batch = new BatchWraperTranscription(b);
                else
                    batch = new BatchWraperBlank();

            }
            else
            {
                cout <<"get spot batch"<<endl;
                //retrieve the next batch from the priority queue
                //  This will have merely ids and subimage params
                //retireve subimages and base64 encode them
                //then
                bool hard=true;
                if (num==-1) {
                    num=5;
                    hard=false;
                }
                SpottingsBatch* b = masterQueue->getSpottingsBatch(num,hard,width,color,prevNgram);
                if (b!=NULL)
                    batch = new BatchWraperSpottings(b);
                else
                    batch = new BatchWraperBlank();
            }
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
        MasterQueue* masterQueue;
        
        
};


