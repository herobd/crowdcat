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
#ifdef NO_NAN
    float ap, accumAP;
#endif
    if (query->getImg().channels()==1)
    {
#ifdef NO_NAN
       res = spotter->subwordSpot_eval(query->getImg(), query->getNgram(), alpha,refinePortion, GlobalK::knowledge()->accumResFor(query->getNgram()), GlobalK::knowledge()->getCorpusXLetterStartBounds(), GlobalK::knowledge()->getCorpusXLetterEndBounds(), &ap, &accumAP);
       
#else
       res = spotter->subwordSpot(query->getImg(), query->getNgram(), alpha,refinePortion);
#endif
    }
    else
    {
        cv::Mat gray;
        cv::cvtColor(query->getImg(),gray,CV_RGB2GRAY);
#ifdef NO_NAN
        res = spotter->subwordSpot_eval(gray, query->getNgram(), alpha,refinePortion, GlobalK::knowledge()->accumResFor(query->getNgram()), GlobalK::knowledge()->getCorpusXLetterStartBounds(), GlobalK::knowledge()->getCorpusXLetterEndBounds(), &ap, &accumAP);
        
#else
        res = spotter->subwordSpot(gray, query->getNgram(), alpha,refinePortion);
#endif
    }
#ifdef NO_NAN
    GlobalK::knowledge()->storeSpottingAccum(query->getNgram(),accumAP);
    if (query->getType()==SPOTTING_TYPE_EXEMPLAR)
        GlobalK::knowledge()->storeSpottingExemplar(query->getNgram(),ap);
    else if (query->getType()==SPOTTING_TYPE_APPROVED)
        GlobalK::knowledge()->storeSpottingNormal(query->getNgram(),ap);
    else 
        GlobalK::knowledge()->storeSpottingOther(query->getNgram(),ap);
#endif

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
