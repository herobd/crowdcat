#ifndef FACADESPOTTER_H
#define FACADESPOTTER_H

#include "Spotter.h"

class FacadeSpotter : public Spotter
{
    private:
    map<string, vector< vector<Spotting> > > loaded;
    void addTestSpottings(string file);
    public:
    FacadeSpotter(MasterQueue* masterQueue, const Knowledge::Corpus* corpus, string modelDir, int numThreads);
    vector<Spotting>* runQuery(SpottingQuery* query);
};

#endif
