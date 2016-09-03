
#ifndef ALMAZANSPOTTER_H
#define ALMAZANSPOTTER_H

#include "Spotter.h"
#include "EmbAttSpotter.h"

#define ALPHA 1.0

class AlmazanSpotter : public Spotter
{
    private:
    EmbAttSpotter* spotter;

    public:
    AlmazanSpotter(MasterQueue* masterQueue, const Knowledge::Corpus* corpus, string modelDir);
    ~AlmazanSpotter()
    {
        delete spotter;
    }
    vector<Spotting>* runQuery(SpottingQuery* query);
    float score(string text, const Mat& image);
};

#endif
