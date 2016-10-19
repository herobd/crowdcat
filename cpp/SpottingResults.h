
#ifndef SPOTTINGRESULTS_H
#define SPOTTINGRESULTS_H

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <vector>
#include <set>
#include <map>
#include <chrono>

#include <iostream>
#include "batches.h"
#include "Global.h"
#include "PageRef.h"

#define UPDATE_OVERLAP_THRESH 0.4
#define UPDATE_OVERLAP_THRESH_TIGHT 0.7

using namespace std;





class scoreComp
{
  bool reverse;
public:
  scoreComp(const bool& revparam=false)
    {reverse=revparam;}
  bool operator() (const Spotting* lhs, const Spotting* rhs) const
  {
    if (reverse) return (lhs->score>rhs->score);
    else return (lhs->score<rhs->score);
  }
};
class tlComp
{
  bool reverse;
public:
  tlComp(const bool& revparam=false)
    {reverse=revparam;}
  bool operator() (const Spotting* lhs, const Spotting* rhs) const
  {
      bool ret;
      if (lhs->tlx == rhs->tlx)
          ret=lhs->tly < rhs->tly;
      else
          ret = lhs->tlx < rhs->tlx;
    if (reverse) return !ret;
    else return ret;
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
    SpottingResults(string ngram, int contextPad);
    SpottingResults(ifstream& in, PageRef* pageRef);
    void save(ofstream& out);

    string ngram;
    
    ~SpottingResults() 
    {
        cout <<"results["<<id<<"]:"<<ngram<<", numberClassifiedTrue: "<<numberClassifiedTrue<<", numberClassifiedFalse: "<<numberClassifiedFalse<<", numberAccepted: "<<numberAccepted<<", numberRejected: "<<numberRejected<<endl;
        float effortR=numberAccepted/(0.0+numberClassifiedTrue+numberClassifiedFalse);
        cout<<"* effort reduction: "<<effortR<<endl;
        //assert(effortR>0);
    }
    
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
    
    //For use when creating SpottingResult
    void add(Spotting spotting);

    //For use when creating SpottingResult Adds a spotting which is a new exemplar. We just want to prevent a future redundant classification of it.
    void addTrueNoScore(const SpottingExemplar& spotting);

    //This will either replace the spottings or add new ones if it can't find any close enough. spottings is consumed.
    //The return value merely indicates whether this needs "resurrected" (Put back into the MasterQueue).
    bool updateSpottings(vector<Spotting>* spottings);
    
    //This accpets a spotting which is a new exemplar. We just want to prevent a future redundant classification of it.
    void updateSpottingTrueNoScore(const SpottingExemplar& spotting);

    SpottingsBatch* getBatch(bool* done, unsigned int num, bool hard, unsigned int maxWidth,int color,string prevNgram, bool need=true);
    
    vector<Spotting>* feedback(int* done, const vector<string>& ids, const vector<int>& userClassifications, int resent=false, vector<pair<unsigned long,string> >* retRemove=NULL);
    
    bool checkIncomplete();
    
private:
    static atomic_ulong _id;
    
    unsigned long id;
    double acceptThreshold;
    double rejectThreshold;
    //int numBatches;
    bool allBatchesSent;
    bool done;
    
    float trueMean;
    float trueVariance;
    float falseMean;
    float falseVariance;

    //float lastDifAcceptThreshold;
    //float lastDifRejectThreshold;
    float lastDifPullFromScore; //The delta for the monentum
    float momentum;
    
    float pullFromScore; //The choosen score from which batches are pulled (around the score, alternatively grt than and less than).
    
    float maxScore;
    float minScore;

    int numLeftInRange; //This actually has the count from the round previous, for efficency

    //This multiset orders the spotting results to ease the extraction of batches
    multiset<Spotting*,scoreComp> instancesByScore; //This holds Spottings yet to be classified
    multiset<Spotting*,tlComp> instancesByLocation; //This is a convienince holder of all Spottings
    map<unsigned long,Spotting> instancesById; //This is all the Spottings
    map<unsigned long,bool> classById; //This is the classifications of Spottings
    
    map<unsigned long, chrono::system_clock::time_point > starts;
   
    //This provides a mapping of ids to allow a feedback of a spotting to be properly mapped if it was updated
    map<unsigned long, unsigned long> updateMap;
#ifdef TEST_MODE
    map<unsigned long, unsigned long> testUpdateMap;
#endif

    //This acts as a pointer to where we last extracted a batch to speed up searching for the correct score area to extract a batch from
    multiset<Spotting*,scoreComp>::iterator tracer;
    
    //SpottingImage getNextSpottingImage(bool* done, int maxWidth,int color,string prevNgram);
   
    //This uses expectation maximization (one iteration each call) to produce new appect/reject thresholds. It assumes a bimodal gaussian distribution, one for positive spottings and one for negative.
    //returns (and sets allBatchesSent) whether we are now done (all spottings lie outside the thresholds)
    //It uses an Otsu threshold to be used to estimate some initail parameters on the first run.
    bool EMThresholds(int swing=0);

    //This returns the iterator of instancesByLocation for the spotting which overlaps (spatailly) the one given
    //It returns instancesByLocation.end() if none is found.
    multiset<Spotting*,tlComp>::iterator findOverlap(const Spotting& spotting) const;

    //How much to pad (top and bottom) images sent to users
    int contextPad;
};

#endif
