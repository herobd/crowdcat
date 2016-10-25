#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <vector>
#include "BatchWraper.h"
#include "batches.h"

class Simulator
{
    public:
    vector<int> spottings(string ngram, vector<Location> locs, vector<string> gt);
    vector<int> newExemplars(vector<string> ngrams, vector<Location> locs);
    string transcription(int wordIndex, vector<SpottingPoint> spottings, vector<string> poss, string gt);
    string manual(int wordIndex, vector<string> poss, string gt);
};
#endif
