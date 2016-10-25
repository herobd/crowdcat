
#ifndef BATCH_WRAPER_NE
#define BATCH_WRAPER_NE

#ifndef NO_NAN
#include <nan.h>
#endif
#include <iostream>
#include <assert.h>
#define BUFFERSIZE 65536
#include <b64/encode.h>
#include "opencv2/highgui/highgui.hpp"
#include "BatchWraper.h"
#include "batches.h"

#ifndef NO_NAN
using namespace Nan;
using namespace v8;
#endif
using namespace std;
class BatchWraperNewExemplars: public BatchWraper
{
    private:
        //output
        vector<string> retData;
        vector<string> retNgram;
        vector<Location> locations;
        string batchId;
        
    public:
        BatchWraperNewExemplars(NewExemplarsBatch* newExemplars);
        ~BatchWraperNewExemplars() {}
#ifndef NO_NAN
        void doCallback(Callback* callback);
#else

        virtual int getType(){return NEW_EXEMPLARS;}
        virtual void getNewExemplars(string* batchId,vector<string>* ngrams, vector<Location>* locs);
#endif
};
#endif
