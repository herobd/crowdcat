#include "NetSpotter.h"

NetSpotter::NetSpotter(const Dataset* corpus, string modelPrefix) 
{
    //spotter = new EmbAttSpotter(modelPrefix+"_emb",true);
    spooter = new CNNSPPSpotter(modelPrefix+"_featurizer.prototxt", modelPrefix+"_embedder.prototxt", modelPrefix+"_wieghts.caffemodel", true, 0.25, windowWidth, 4, modelPrefix+"_cnnsppspotter");
    
    //dataset = new AlmazanDataset(corpus);
    spotter->setCorpus_dataset(corpus);
}

vector<SpottingResult> NetSpotter::runQuery(SpottingQuery* query) const
{
    vector< SubwordSpottingResult > res;
    float refinePortion=0.20;
    if (query->getImg().cols==0)
    {
        refinePortion=0.25;
#ifdef NO_NAN
           res = spotter->subwordSpot_eval(query->getNgram(),refinePortion, GlobalK::knowledge()->accumResFor(query->getNgram()), GlobalK::knowledge()->getCorpusXLetterStartBounds(), GlobalK::knowledge()->getCorpusXLetterEndBounds(), &ap, &accumAP);
           
#else
           res = spotter->subwordSpot(query->getNgram(), refinePortion);
#endif
    }
    else
    {
#ifdef NO_NAN
        float ap, accumAP;
#endif
        if (query->getImg().channels()==1)
        {
#ifdef NO_NAN
           res = spotter->subwordSpot_eval(query->getImg(), refinePortion, GlobalK::knowledge()->accumResFor(query->getNgram()), GlobalK::knowledge()->getCorpusXLetterStartBounds(), GlobalK::knowledge()->getCorpusXLetterEndBounds(), &ap, &accumAP);
           
#else
           res = spotter->subwordSpot(query->getImg(), refinePortion);
#endif
        }
        else
        {
            cv::Mat gray;
            cv::cvtColor(query->getImg(),gray,CV_RGB2GRAY);
#ifdef NO_NAN
            res = spotter->subwordSpot_eval(gray, refinePortion, GlobalK::knowledge()->accumResFor(query->getNgram()), GlobalK::knowledge()->getCorpusXLetterStartBounds(), GlobalK::knowledge()->getCorpusXLetterEndBounds(), &ap, &accumAP);
            
#else
            res = spotter->subwordSpot(gray, refinePortion);
#endif
        }
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
                                res[i].startX,
                                res[i].endX
                               );
    return ret;
}

float NetSpotter::score(string text, const cv::Mat& image) const
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
float NetSpotter::score(string text, int wordIndex) const
{
    return spotter->compare(text,wordIndex);
}
