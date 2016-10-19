#include "AlmazanSpotter.h"

AlmazanSpotter::AlmazanSpotter(const Dataset* corpus, string modelPrefix) 
{
    spotter = new EmbAttSpotter(modelPrefix+"_emb",true);
    
    dataset = new AlmazanDataset(corpus);
    spotter->setCorpus_dataset(dataset,true);
}

vector<SpottingResult> AlmazanSpotter::runQuery(SpottingQuery* query) const
{
    float alpha=ALPHA;
    float refinePortion=0.15;
    if (query->getImg().cols==0)
    {
        alpha=0.0;
        refinePortion=0.25;
    }
    vector< SubwordSpottingResult > res;
    if (query->getImg().channels()==1)
       res = spotter->subwordSpot(query->getImg(), query->getNgram(), alpha,refinePortion);
    else
    {
        cv::Mat gray;
        cv::cvtColor(query->getImg(),gray,CV_RGB2GRAY);
        res = spotter->subwordSpot(gray, query->getNgram(), alpha,refinePortion);
    }

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
    if (image.channels() == 1)
        return spotter->compare(text,image);
    else
    {
        cv::Mat gray;
        cv::cvtColor(image,gray,CV_RGB2GRAY);
        return spotter->compare(text,gray);
    }
}
float AlmazanSpotter::score(string text, int wordIndex) const
{
    return spotter->compare(text,wordIndex);
}
