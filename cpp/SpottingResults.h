#ifndef SPOTTINGRESULTS_H
#define SPOTTINGRESULTS_H

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <vector>
#include <set>
#include <map>

#include <iostream>

using namespace std;

class Spotting {
public:
    Spotting() :
        tlx(-1), tly(-1), brx(-1), bry(-1), pageId(-1), pagePnt(NULL), ngram(""), score(nan("")), id(-1) {}
    
    Spotting(int tlx, int tly, int brx, int bry, int pageId, const cv::Mat* pagePnt, string ngram, float score) : 
        tlx(tlx), tly(tly), brx(brx), bry(bry), pageId(pageId), pagePnt(pagePnt), ngram(ngram), score(score)
    {
        id = _id++;
    }
    
    Spotting(const Spotting& s) : 
        tlx(s.tlx), tly(s.tly), brx(s.brx), bry(s.bry), pageId(s.pageId), pagePnt(s.pagePnt), ngram(s.ngram), score(s.score)
    {
        id = s.id;
    }
    
    int tlx, tly, brx, bry, pageId;
    const cv::Mat* pagePnt;
    string ngram;
    float score;
    unsigned long id;
    virtual cv::Mat img()
    {
        return (*pagePnt)(cv::Rect(tlx,tly,brx-tlx,bry-tly));
    }
private:
    static unsigned long _id;
};

class SpottingImage : public Spotting {
public:
    SpottingImage(const Spotting& s, int maxWidth) : 
        Spotting(s)
    {
        int oneSide = maxWidth/2;
        int sideFromR = (oneSide- (brx-tlx)/2);
        int left = tlx-sideFromR;
        int right = brx+sideFromR;
        //cout <<"getting image window..."<<endl;
        if (left>=0 && right<s.pagePnt->cols)
        {   
            //cout <<"normal: "<<left<<" "<<tly<<" "<<right-left<<" "<<bry-tly<<endl;
            image = (*s.pagePnt)(cv::Rect(left,tly,right-left,bry-tly));
        }
        else
        {
            image = cv::Mat(bry-tly,maxWidth,s.pagePnt->type());
            if (image.channels()==1)
                image.setTo(cv::Scalar(10));
            else
                image.setTo(cv::Scalar(10,10,10));
            int leftOff = left>=0?0 : -1*(left+1);
            int newLeft=left<0?0:left;
            if (right>=s.pagePnt->cols)
                right = s.pagePnt->cols-1;
            //cout <<"adjusted from: "<<newLeft<<" "<<tly<<" "<<right-newLeft<<" "<<bry-tly<<endl;
            //cout <<"adjusted to: "<<leftOff<<" "<<tly<<" "<<right-newLeft<<" "<<bry-tly<<endl;
            (*s.pagePnt)(cv::Rect(newLeft,tly,right-newLeft,bry-tly)).copyTo(image(cv::Rect(leftOff,0,right-newLeft,bry-tly)));
        }
        //cout <<"done, now coloring..."<<endl;
        if (image.channels()==1)
            cv::cvtColor(image, image, CV_GRAY2RGB);
        for (int r=0; r<=bry-tly; r++)
            for (int c=sideFromR; c<=sideFromR+brx-tlx; c++)
            {
                cv::Vec3b pix = image.at<cv::Vec3b>(r,c);
                image.at<cv::Vec3b>(r,c) = cv::Vec3b(pix[0]*0.75,min(30+(int)(pix[1]*1.05),255),pix[2]*0.75);
            }
        //cout <<"done"<<endl;
    }
    virtual cv::Mat img()
    {
        return image;
    }
private:
    cv::Mat image;
};




class SpottingsBatch {
public:
    
    SpottingsBatch(string ngram, unsigned long spottingResultsId) : 
        ngram(ngram), spottingResultsId(spottingResultsId)
    {
        batchId = _batchId++;
    }
    string ngram;
    unsigned long batchId;
    unsigned long spottingResultsId;
    
    
    void push_back(SpottingImage im) {
        if (im.img().cols>0)
		instances.push_back(im);
    }
    SpottingImage operator [](int i) const    {return instances[i];}
    SpottingImage & operator [](int i) {return instances[i];}
    SpottingImage at(int i) const    {return instances.at(i);}
    SpottingImage & at(int i) {return instances.at(i);}
    unsigned int size() const { return instances.size();}
private:
    static unsigned long _batchId;
    vector<SpottingImage> instances;
};


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
    SpottingResults(string ngram, double acceptThreshold, double rejectThreshold);
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
    
    void add(Spotting spotting);
    SpottingsBatch* getBatch(bool* done, unsigned int num, unsigned int maxWidth);
    
    vector<Spotting>* feedback(bool* done, const vector<string>& ids, const vector<int>& userClassifications);
    
private:
    static unsigned long _id;
    
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
    float minScore;
    
    //This multiset orders the spotting results to ease the extraction of batches
    multiset<Spotting*,scoreComp> instancesByScore;
    map<unsigned long,Spotting> instancesById;
    map<unsigned long,bool> classById;
    
    //This acts as a pointer to where we last extracted a batch to speed up searching for the correct score area to extract a batch from
    multiset<Spotting*,scoreComp>::iterator tracer;
    
    SpottingImage getNextSpottingImage(bool* done, int maxWidth);
    
    void EMThresholds();
};

#endif
