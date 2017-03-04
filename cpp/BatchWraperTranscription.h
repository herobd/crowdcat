
#ifndef BATCH_WRAPER_TRANS
#define BATCH_WRAPER_TRANS
#ifndef NO_NAN
#include <nan.h>
#endif
#include <iostream>
#include <assert.h>
#define BUFFERSIZE 65536
#include <b64/encode.h>
#include "opencv2/highgui/highgui.hpp"
#include "batches.h"
#include "BatchWraper.h"

#ifndef NO_NAN
using namespace Nan;
using namespace v8;
#endif
using namespace std;
class BatchWraperTranscription: public BatchWraper
{
    protected:
        //output
        string batchId;
        string wordImgStr;
        string gt;
        double scale;
        vector<string> retPoss;
        bool manual;

    public:
        BatchWraperTranscription(Word* word, int width, int contextPad, bool allowManual);
        ~BatchWraperTranscription() {}
#ifndef NO_NAN
        virtual void doCallback(Callback* callback);
#else
        virtual int getType(){return TRANSCRIPTION;}
        virtual void getTranscription(int* batchId, vector<string>* poss, bool* manual, string* gt);
#endif
};
#endif
