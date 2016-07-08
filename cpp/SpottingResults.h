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

#define FUZZY 12
#define UPDATE_OVERLAP_THRESH 0.3

using namespace std;



#define SPOTTING_TYPE_NONE 1
#define SPOTTING_TYPE_APPROVED 2
#define SPOTTING_TYPE_THRESHED 3
#define SPOTTING_TYPE_EXEMPLAR 4

class Spotting {
public:
    Spotting() :
        tlx(-1), tly(-1), brx(-1), bry(-1), pageId(-1), pagePnt(NULL), ngram(""), score(nan("")), id(-1), type(SPOTTING_TYPE_NONE), ngramRank(-1) {}
    Spotting(int tlx, int tly, int brx, int bry) :
        tlx(tlx), tly(tly), brx(brx), bry(bry), pageId(-1), pagePnt(NULL), ngram(""), score(nan("")), id(-1), type(SPOTTING_TYPE_NONE), ngramRank(-1) {}
    Spotting(int tlx, int tly) :
        tlx(tlx), tly(tly), brx(-1), bry(-1), pageId(-1), pagePnt(NULL), ngram(""), score(nan("")), id(-1), type(SPOTTING_TYPE_NONE), ngramRank(-1) {}
    
    Spotting(int tlx, int tly, int brx, int bry, int pageId, const cv::Mat* pagePnt, string ngram, float score) : 
        tlx(tlx), tly(tly), brx(brx), bry(bry), pageId(pageId), pagePnt(pagePnt), ngram(ngram), score(score), type(SPOTTING_TYPE_NONE), ngramRank(-1)
    {
        id = _id++;
    }
    
    Spotting(const Spotting& s) : 
        tlx(s.tlx), tly(s.tly), brx(s.brx), bry(s.bry), pageId(s.pageId), pagePnt(s.pagePnt), ngram(s.ngram), score(s.score), type(s.type), ngramRank(s.ngramRank)
    {
        id = s.id;
    }
    
    int tlx, tly, brx, bry, pageId;
    const cv::Mat* pagePnt;
    string ngram;
    float score;
    unsigned long id;
    unsigned char type;
    virtual cv::Mat img()
    {
        return (*pagePnt)(cv::Rect(tlx,tly,brx-tlx,bry-tly));
    }
    int ngramRank;

private:
    static unsigned long _id;
};

class SpottingImage : public Spotting {
public:
    SpottingImage(const Spotting& s, int maxWidth, int color, string prevNgram="") : 
        Spotting(s)
    {
        int oneSide = maxWidth/2;
        int sideFromR = (oneSide- (brx-tlx)/2);
        int left = tlx-sideFromR;
        int right = brx+sideFromR-1;
        //cout <<"getting image window... sideFromR="<<sideFromR<<", oneSide="<<oneSide<<", tlx="<<tlx<<", brx="<<brx<<", left="<<left<<", right="<<right<<endl;
        if (left>=0 && right<s.pagePnt->cols)
        {   
            //cout <<"normal: "<<left<<" "<<tly<<" "<<right-left<<" "<<bry-tly<<endl;
            image = ((*s.pagePnt)(cv::Rect(left,tly,right-left,bry-tly))).clone();
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
            //cout <<"adjusted to: "<<leftOff<<" "<<0<<" "<<right-newLeft<<" "<<bry-tly<<endl;
            (*s.pagePnt)(cv::Rect(newLeft,tly,right-newLeft,bry-tly)).copyTo(image(cv::Rect(leftOff,0,right-newLeft,bry-tly)));
        }
        //cout <<"done, now coloring..."<<endl;
        if (image.channels()==1)
            cv::cvtColor(image, image, CV_GRAY2RGB);
        
        //if (left==1117 && tly==186)
        //{
         //   cv::imshow("rer", image);
          //  cv::waitKey();
        //}
        if (prevNgram.compare(ngram)!=0)
        {
            color++;//=(color+1)%5;
        }
        for (int r=0; r<bry-tly; r++)
            for (int c=sideFromR-FUZZY; c<sideFromR+brx-tlx+FUZZY; c++)
            {
                //if (left==1117 && tly==186 && (r==10 || r==5) && c==300)
                //{
                  //  cv::imshow("rer", image);
                  //  cv::waitKey(1);
                //}
                //cout <<" ("<<r<<" "<<c;
                //cout.flush();
                if (c>=0 && c<image.cols)
                {
                    cv::Vec3b& pix = image.at<cv::Vec3b>(r,c);
                    //cout <<"):";
                    //cout.flush();
                    //cout<<(int)pix[0];
                    //cout.flush();
                    //image.at<cv::Vec3b>(r,c) = cv::Vec3b(pix[0]*0.75,min(20+(int)(pix[1]*1.05),255),pix[2]*0.75);
                    float fuzzyMult = 1;
                    if (c<sideFromR+FUZZY)
                        fuzzyMult=(c-sideFromR+FUZZY)/(2.0*FUZZY);
                    else if (c>sideFromR+brx-tlx-FUZZY)
                        fuzzyMult=(sideFromR+brx-tlx+FUZZY-c)/(2.0*FUZZY);
                    
                    if (color%3!=1)//red is a bad color for highlighting
                    {
                        pix[color%3]*=1.0-0.25*fuzzyMult;
                        pix[(color+1)%3] =min((int)(20*fuzzyMult+(pix[(color+1)%3]*(1.0+fuzzyMult*0.05))),255);
                        pix[(color+2)%3]*=1.0-0.25*fuzzyMult;
                    }
                    else
                    {
                        pix[color%3]=min((int)(20*fuzzyMult+(pix[(color)%3]*(1.0+fuzzyMult*0.05))),255);
                        pix[(color+1)%3] =min((int)(20*fuzzyMult+(pix[(color+1)%3]*(1.0+fuzzyMult*0.05))),255);
                        pix[(color+2)%3]*=1.0-0.25*fuzzyMult;
                    }
                }
            }
        //cout<<endl;
        //cout <<"done"<<endl;
    }
    
    SpottingImage(const SpottingImage& s) : Spotting(s)
    {
        image=s.image;
    }
    
    virtual cv::Mat img()
    {
        return image;
    }
    cv::Mat ngramImg()
    {
        return (*pagePnt)(cv::Rect(tlx,tly,brx-tlx,bry-tly));
    }
    int classified;
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
    SpottingResults(string ngram);
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
    
    void add(Spotting spotting);
    void updateSpottings(vector<Spotting> spottings);
    SpottingsBatch* getBatch(bool* done, unsigned int num, bool hard, unsigned int maxWidth,int color,string prevNgram);
    
    vector<Spotting>* feedback(bool* done, const vector<string>& ids, const vector<int>& userClassifications, int resent=false, vector<pair<unsigned long,string> >* retRemove=NULL);
    
    bool checkIncomplete();
    
private:
    static unsigned long _id;
    
    unsigned long id;
    double acceptThreshold;
    double rejectThreshold;
    //int numBatches;
    bool allBatchesSent;
    
    
    float trueMean;
    float trueVariance;
    float falseMean;
    float falseVariance;
    
    float pullFromScore;
    float delta;
    
    float maxScore;
    float minScore;
    
    //This multiset orders the spotting results to ease the extraction of batches
    multiset<Spotting*,scoreComp> instancesByScore;
    multiset<Spotting*,tlComp> instancesByLocation;
    map<unsigned long,Spotting> instancesById;
    map<unsigned long,bool> classById;
    
    map<unsigned long, chrono::system_clock::time_point > starts;
    
    //This acts as a pointer to where we last extracted a batch to speed up searching for the correct score area to extract a batch from
    multiset<Spotting*,scoreComp>::iterator tracer;
    
    SpottingImage getNextSpottingImage(bool* done, int maxWidth,int color,string prevNgram);
    
    void EMThresholds(bool init=false);
};

#endif
