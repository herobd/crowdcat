
#ifndef BATCH_WRAPER_NE
#define BATCH_WRAPER_NE

#include <nan.h>
#include <iostream>
#include <assert.h>
#define BUFFERSIZE 65536
#include <b64/encode.h>
#include "opencv2/highgui/highgui.hpp"
#include "BatchWraper.h"
#include "NewExemplarsBatchQueue.h"

using namespace Nan;
using namespace std;
using namespace v8;
class BatchWraperNewExemplars: public BatchWraper
{
    private:
        //output
        vector<string> retData;
        vector<string> retNgram;
        vecotr<Locations> locations;
        string batchId;
        
    public:
        BatchWraperNewExemplars(NewExemplarsBatch* newExemplars);
        ~BatchWraperNewExemplars() {}
        void doCallback(Callback* callback);
};
#endif
