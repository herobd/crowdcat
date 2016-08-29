#ifndef WBP_H
#define WBP_H

#include <vector>
#include "spotting.h"
#include "batches.h"

using namespace std;

class WordBackPointer
{
    public:
        virtual vector<Spotting*> result(string selected, vector< pair<unsigned long, string> >* toRemoveExemplars)= 0;
        virtual void error(vector< pair<unsigned long, string> >* toRemoveExemplars)= 0;
        virtual TranscribeBatch* removeSpotting(unsigned long sid, vector<Spotting*>* newExemplars, vector< pair<unsigned long, string> >* toRemoveExemplars)= 0;
};

#endif
