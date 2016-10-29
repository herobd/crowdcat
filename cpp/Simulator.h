#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <vector>
#include "BatchWraper.h"
#include "batches.h"

/*
   Spotting timing ought to take into consideration:
   -Ngram
   -if prev ngram same
   -Whether word contains ngram (gt shows this)

   Transcription timing ought to consider
   -Length if word
   -position of correct
   -ngram error
   -none error

   Manual timing ought to consider
   -length of word


Spotting:
Does it overlap with an ngram?
No -> reject with X% prob
Yes ->
    If spot inside ngram:
        Overlap >75% of ngram accept with Y% prob, else reject
    else If spot consumes ngram:
        Spot <145% of ngram accept with Z% prob, else reject
    else If spot sided on ngram
        >55% overlap and spotting outside ngram is <90% of the overlap: accept with W% prob, else reject
*/

class Simulator
{
    public:
    vector<int> spottings(string ngram, vector<Location> locs, vector<string> gt, string prevNgram);
    vector<int> newExemplars(vector<string> ngrams, vector<Location> locs, string prevNgram);
    string transcription(int wordIndex, vector<SpottingPoint> spottings, vector<string> poss, string gt, bool lastWasTrans);
    string manual(int wordIndex, vector<string> poss, string gt, bool lastWasMan);

    private:
    network_t *spotNet;
};
#endif
