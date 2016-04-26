
#include <multiset>
#include <multimap>
#include <Spotting>
#include <SpottingsBatch>

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
    SpottingResults(string ngram, double acceptThreshold, double rejectThreshold) : 
        ngram(ngram), acceptThreshold(acceptThreshold), rejectThreshold(rejectThreshold)
    {
        id = _id++;
        sem_init(&mutexSem,false,1);
        numBatches=0;
        allBatchesSent=false;
        
        numberClassifiedTrue=0;
        numberClassifiedFalse=0;
        numberAccepted=0;
        numberRejected=0;
        maxScore=-999999;
        
        
        trueMean=0;//How to initailize?
        trueVariance=0.1;
        falseMean=maxScore;
        falseVariance=0.1;
    }
    string ngram;
    
    
    //sem_t mutexSem;
    
    //stats
    int numberClassifiedTrue;
    int numberClassifiedFalse;
    int numberAccepted;
    int numberRejected;
    
    unsigned long getId()
    {
        return id;
    }
    
    void add(Spotting spotting) {
        //sem_wait(&mutexSem);
        instancesById[spotting.id]=spotting;
        instancesByScore.insert(&instancesById[spotting.id]);
        tracer = instancesByScore.begin();
        if (spotting.score>maxScore)
        {
            if (falseMean==maxScore && falseVariance==0.1)
                falseMean=spotting.score;
            maxScore=spotting.score;
        }
        //sem_post(&mutexSem);
    }
    SpottingsBatch* getBatch(bool* done, unsigned int num, unsigned int maxWidth=0) {
        SpottingsBatch ret = new SpottingsBatch(ngram,id);
        //sem_wait(&mutexSem);
        unsigned int toRet = ((((signed int)instancesByScore.size())-(signed int) num)>3)?num:instancesByScore.size();
        
        for (unsigned int i=0; i<toRet && !*done; i++) {
            ret.instances.push_back(getNextSpottingImage(done, maxWidth));
            
        }
        if (*done)
            allBatchesSent=true;
        //sem_post(&mutexSem);
        numBatches++;
        return ret;
    }
    
    vector<Spotting>* feedback(bool* done, const vector<string>& ids, const vector<int>& userClassifications)
    {
        
        
        vector<Spotting>* ret = new vector<Spotting>();
        
        for (int i=0; i< ids.size(); i++)
        {
            unsigned long id = stoul(ids[i]);
            ret->push_back(instancesById[id]);
            // adjust threshs
            if (userClassifications[i])
            {
                numberClassifiedTrue++;
                classById[id]=true;
            }
            else
            {
                numberClassifiedFalse++;
                classById[id]=false;
            }
        }
        EMThresholds();
        
        if (--numBatches==0 && allBatchesSent)
        {
            *done=true;
            
            tracer = instancesByScore.begin();
            while (tracer->score < acceptThreshold)
            {
                ret->push_back(*tracer);
                tracer++;
                numberAccepted++;
            }
            numberRejcted = instancesByScore.end()-tracer;
            
        }
        return ret;
    }
    
private:
    static unsigned long _id=0;
    
    unsigned long id;
    double acceptThreshold;
    double rejectThreshold;
    int numBatches;
    bool allBatchesSent;
    
    float trueMean;
    float trueVariance;
    float falseMean;
    float falseVariance;
    
    float maxScore;
    
    //This multiset orders the spotting results to ease the extraction of batches
    multiset<Spotting*,scoreComp> instancesByScore;
    map<unsigned long,Spotting> instancesById;
    map<unsigned long,bool> classById;
    
    //This acts as a pointer to where we last extracted a batch to speed up searching for the correct score area to extract a batch from
    multiset<Spotting,scoreComp>::iterator tracer;
    
    SpottingImage getNextSpottingImage(bool* done, int maxWidth)
    {
        float midScore = acceptThreshold + (rejectThreshold-acceptThreshold)/2.0
        if ((*tracer)->score < midScore)
            while((*tracer)->score<midScore)
                tracer++;
        else
            while((*tracer)->score>midScore)
                tracer--;
        
        //if(tracer->score<midScore)
        //    tracer++;
        
        if (tracer->score > rejectThreshold)
            tracer--;
        
        SpottingImage toRet(**tracer,maxWidth);
        tracer = instancesByScore.remove(tracer);
        if (tracer != instancesByScore.begin())
        {
            tracer--;
        }
        
        
        if (tracer == instancesByScore.end())
            {
                *done=true;
                return toRet;
            }
        
        if ((*tracer)->score < acceptThreshold)
            tracer++;
        if ((*tracer)->score > rejectThreshold)
            *done = true;
        
        
        return toRet;
    }
    
    void EMThresholds()
    {
        /*This will likely predict very narrow and ditinct distributions
         *initailly. This should be fine as we sample from the middle of
         *the thresholds outward.
         */
        //expectation
        
        //map<unsigned long, bool> expected;
        vector<float> expectedTrue;
        float sumTrue=0;
        vector<float> expectedFalse;
        flaot sumFalse=0;
        for (unsigned long id : instancesById.keys())
        {
            if (classById.hasKey(id))
            {
                if (classById[id])
                {
                    expectedTrue.push_back(instancesById[id].score);
                    sumTrue+=instancesById[id].score;
                else
                {
                    expectedFalse.push_back(instancesById[id].score);
                    sumFalse+=instancesById[id].score;
                }
            }
            else
            {
                double trueProb = exp(-1*pow(instancesById[id].score - trueMean)/(2*trueVariance));
                double falseProb = exp(-1*pow(instancesById[id].score - falseMean)/(2*falseVariance));
                if (trueProp>falseProb)
                {
                    sumTrue+=instancesById[id].score;
                    expectedTrue.push_back(instancesById[id].score);
                }
                else
                {
                    expectedFalse.push_back(instancesById[id].score);
                    sumFalse+=instancesById[id].score;
                }
            }
        }
        
        //maximization
        trueMean=sumTrue/expectedTrue.size();
        falseMean=sumFalse/expectedFalse.size();
        trueVariance=0;
        for (float score : expectedTrue)
            trueVariance = (score-trueMean)*(score-trueMean);
        trueVariance/=expectedTrue.size();
        falseVariance=0;
        for (float score : expectedFalse)
            falseVariance = (score-falseMean)*(score-falseMean);
        falseVariance/=expectedFalse.size();
        
        //set new thresholds
        acceptThreshold = falseMean-2*sqrt(falseVariance);
        rejectThreshold = trueMean+2*sqrt(trueVariance);
    }
};
