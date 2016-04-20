class scoreComp
{
  bool reverse;
public:
  scoreComp(const bool& revparam=false)
    {reverse=revparam;}
  bool operator() (const Spotting& lhs, const Spotting& rhs) const
  {
    if (reverse) return (lhs.score>rhs.score);
    else return (lhs.score<rhs.score);
  }
};

/* This class holds all of the results of a single spotting query.
 * It orders these by score. It tracks an accept and reject 
 * threshold, indicating the score we are confident we can either
 * accept a score at, or reject a score at. These thresholds are
 * adjusted as user-classifications are returned to it. The
 * thresholds are initailly set by a global criteria, which this
 * returns to when it is finished.
 * It also facilitates generating a batch from the results. This
 * is done by detirming the midpoint between the accept threshold
 * and the reject threshold, and taking the X number of spottings
 * from that point in ordered results.
 */
class SpottingResults {
public:
    SpottingResults(string ngram) : 
        ngram(ngram)
    {
        tracer = instances.begin();
    }
    string ngram;
    
    
    void push_back(Spotting spotting) {
        instances.insert(spotting.score,spotting);
    }
    vector<SpottingImage> getSpottings(unsigned int num, unsigned int maxWidth=0) {
        vector<SpottingImage> ret;
        unsigned int toRet = ((((signed int)instances.size())-(signed int) num)>3)?num:instances.size();
        
        for (unsigned int i=0; i<toRet; i++) {
            SpottingImage img=getNextSpottingImage();
            if (maxWidth!=0 && img.img.cols>maxWidth) {
                int crop = (img.img.cols-maxWidth)/2;
                img.img=img.img(cv::Rect(crop,0,maxWidth,img.img.rows));
            }
            ret.push_back(img);
            instances.pop_back();
        }
        return ret;
    }
    
private:
    double acceptThreshold;
    double rejectThreshold;
    
    //This multiset orders the spotting results to ease the extraction of batches
    multiset<Spotting,scoreComp> instances;
    
    //This acts as a pointer to where we last extracted a batch to speed up searching for the correct score area to extract a batch from
    multiset<Spotting,scoreComp>::iterator tracer;
    
    SpottingImage getNextSpottingImage()
    {
        float midScore = acceptThreshold + (rejectThreshold-acceptThreshold)/2.0
        while(tracer->score<midScore)
            tracer++;
    }
};
