#include "Spotter.h"

FacadeSpotter class : public Spotter
{
    private:
    map<string, vector< vector<Spotting> > > loaded;
    void FacadeSpotter::addTestSpottings(string file);
    public:
    Spotter(MasterQueue* masterQueue, const Corpus* corpus, string modelDir, int numThreads) : Spotter(masterQueue, corpus, modelDir, numThreads);
    vector<Spotting> runQuery(SpottingQuery* query);
};

#endif
