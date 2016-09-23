#include "BOVWSpotter.h"

BOVWSpotter::BOVWSpotter(const Dataset* corpus, string modelPrefix, int batchSize) 
{
    vector<Vec2i> spp={Vec2i(1,1),Vec2i(2,2)};
    spotter = new EnhancedBoVW(spp);
    Preprocessor pp(PP_BASELINE_CENTER | PP_BASELINE_NORMALIZE || PP_DESLANT);
    spotter->setPre(pp);
    string codebookLoc = argv[2];
    spotter.codebook = new Codebook();
    spotter.codebook->readIn(modelPrefix+"_codebook.dat");

    numBatches = corpus->size()/batchSize;

    for (int i=0; i<corpus->size(); i++)
    {
        spotter->encodeAndSaveImageBatch(


}

vector<SpottingResult> BOVWSpotter::runQuery(SpottingQuery* query) const
{
    float alpha=ALPHA;
    if (query->getImg().cols==0)
        alpha=0.0;
    vector< SubwordSpottingResult > res;
    if (query->getImg().channels()==1)
       res = spotter->subwordSpot(query->getImg(), query->getNgram(), alpha);
    else
    {
        cv::Mat gray;
        cv::cvtColor(query->getImg(),gray,CV_RGB2GRAY);
        res = spotter->subwordSpot(gray, query->getNgram(), alpha);
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

float BOVWSpotter::score(string text, const cv::Mat& image) const
{
    cv::Mat pi = BOVWDataset::preprocess(image);
    if (pi.channels() == 1)
        return spotter->compare(text,pi);
    else
    {
        cv::Mat gray;
        cv::cvtColor(pi,gray,CV_RGB2GRAY);
        return spotter->compare(text,gray);
    }
}
float BOVWSpotter::score(string text, int wordIndex) const
{
    return spotter->compare(text,wordIndex);
}
