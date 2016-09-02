#ifndef WBP_H
#define WBP_H

#include <vector>
#include "spotting.h"
#include "batches.h"

using namespace std;

class TranscribeBatch;

class WordBackPointer
{
    public:
        virtual vector<Spotting*> result(string selected, unsigned long batchId, bool resend, vector< pair<unsigned long, string> >* toRemoveExemplars)= 0;
        virtual void error(unsigned long batchId, bool resend, vector< pair<unsigned long, string> >* toRemoveExemplars)= 0;
        virtual TranscribeBatch* removeSpotting(unsigned long sid, unsigned long batchId, bool resend, vector<Spotting*>* newExemplars, vector< pair<unsigned long, string> >* toRemoveExemplars)= 0;
};

#endif
