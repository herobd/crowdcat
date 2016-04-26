#ifndef MASTERQUEUE_H
#define MASTERQUEUE_H


#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <vector>
#include <queue>
#include <iostream>
#include "semaphore.h"


using namespace std;

class Spotting {
public:
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
    const cv::Mat* pagePnt
    string ngram;
    float score;
    unsigned long id;
    virtual cv::Mat img()
    {
        return (*pagePnt)(Rect(tlx,tly,brx-tlx,bry-tly));
    }
private
    static unsigned long _id;
};

class SpottingImage : public Spotting {
public:
    SpottingImage(const Spotting& s, int maxWidth) : 
        Spotting(s)
    {
        int oneSide = maxWidth/2;
        int center = tlx + (brx-tlx)/2;
        int left = tlx-(oneSide- (brx-tlx)/2);
        int right = brx+(oneSide- (brx-tlx)/2);
        if (left>=0 && right<s.pagePnt->cols)
            image = (*s.pagePnt)(Rect(left,tly,right-left,bry-tly));
        else
        {
            image = Mat(bry-tly,maxWidth,s.pagePnt->type());
            leftOff = left>=0?0 : -1*left;
            if (left<0)
                left=0;
            if (right>=s.pagePnt->cols)
                right = s.pagePnt->cols-1;
            image(Rect(leftOff,tly,right-left,bry-tly)) = (*s.pagePnt)(Rect(left,tly,right-left,bry-tly));
        }
    }
    virtual cv::Mat img()
    {
        return image;
    }
private
    static unsigned long _id;
    cv::Mat image;
};




class SpottingsBatch {
public:
    
    Spottings(string ngram, unsigned long spottingResultsId) : 
        ngram(ngram), spottingResultsId(spottingResultsId)
    {
        batchId = _batchId++;
    }
    string ngram;
    unsigned long batchId;
    unsigned long spottingResultsId;
    
    
    void push_back(SpottingImage im) {
        if (im.img.cols>0)
		instances.push_back(im);
    }
    SpottingImage operator [](int i) const    {return instances[i];}
    SpottingImage & operator [](int i) {return instances[i];}
    SpottingImage at(int i) const    {return instances.at(i);}
    SpottingImage & at(int i) {return instances.at(i);}
    unsigned int size() const { return instance.size();}
private:
    static unsigned long _batchId=0;
    vector<SpottingImage> instances;
};





class MasterQueue {
private:
    sem_t semResultsQueue;
    sem_t semResults;
    
    map<unsigned long, pair<sem_t*,SpottingsResults*> > results;
    
    int atID;
    //map<unsigned long,unsigned long> batchToResults;
public:
    MasterQueue();
    SpottingsBatch* getBatch(unsigned int numberOfInstances, unsigned int maxWidth=0);
    vector<Spotting>* feedback(unsigned long id, const multimap<unsigned long,bool>& userClassifications);
    void addSpottingsResults(SpottingsResults* res);
};
#endif
