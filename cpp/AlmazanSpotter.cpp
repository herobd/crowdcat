#include "AlmazanSpotter.h"

AlmazanSpotter::AlmazanSpotter(const Dataset* corpus, string modelPrefix) 
{
    spotter = new EmbAttSpotter(modelPrefix,true);
    
    dataset = new AlmazanDataset(corpus);
    spotter->setCorpus_dataset(dataset,true);
}

vector<SpottingResult> AlmazanSpotter::runQuery(SpottingQuery* query) const
{
    float alpha=ALPHA;
    if (query->getImg().cols==0)
        alpha=0.0;
    vector< SubwordSpottingResult > res = spotter->subwordSpot(query->getImg(), query->getNgram(), alpha);
    vector <SpottingResult> ret(res.size());
    for (int i=0; i<res.size(); i++)
        ret[i] = SpottingResult(res[i].imIdx,
                                res[i].score,
                                dataset->backwards(res[i].imIdx,res[i].startX),
                                dataset->backwards(res[i].imIdx,res[i].endX)
                               );
    return ret;
}

float AlmazanSpotter::score(string text, const cv::Mat& image) const
{
    return spotter->compare(text,image);
}
