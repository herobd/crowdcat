#include "AlmazanSpotter.h"

AlamazanSpotter::AlamazanSpotter(MasterQueue* masterQueue, const Knowledge::Corpus* corpus, string modelDir) : Spotter(masterQueue, corpus, modelPrefix)
{
    spotter = new EmbAttSpotter(modelPrefix);
    spotter.setCorpus_dataset(corpus);
}

vector<Spotting>* AlamazanSpotter::runQuery(SpottingQuery* query)
{
    float alpha=ALPHA;
    if (query->getImg().cols==0)
        alpha=0.0;
    vector< SubwordSpottingResult > res = spotter->subwordSpot(query->getImg(), query->getNgram(), alpha);
    vector<Spotting>* ret = new vector<Spotting>(res.size());
    for (int i=0; i<res.size(); i++)
    {
        const Word* w = corpus->getWord(res[i].imIdx);
        int tlx, tly, brx, bry;
        bool done;
        w->getBoundsAndDone(&tlx, &tly, &brx, &bry, &done);
        ret[i] = Spotting(res[i].startX, tly, res[i].endX, bry, w->getPageId(), w->getPage(), query->getNgram(), res[i].score);
    }

float AlamazanSpotter::score(string text, const Mat& image)
{
    return spotter->compare(test,image);
}
