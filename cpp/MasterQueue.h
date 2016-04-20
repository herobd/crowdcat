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
    Spotting(int tlx, int tly, int brx, int bry, string ngram, float score) : 
        tlx(tlx), tly(tly), brx(brx), bry(bry), ngram(ngram), score(score) {}
    int tlx, tly, brx, bry;
    string ngram;
    float score;
};



class SpottingImage {
public:
    SpottingImage(string id, string ngram, cv::Mat img) : 
        id(id), ngram(ngram), img(img) {}
    SpottingImage(const SpottingImage& o) : 
        id(o.id), ngram(o.ngram), img(o.img) {}
        
    
    
    string id;
    string ngram;
    cv::Mat img;
};

class Spottings {
public:
    Spottings(string ngram) : 
        ngram(ngram),  batchId("") {}
    Spottings(string ngram, string batchId) : 
        ngram(ngram),  batchId(batchId) {}
    string ngram;
    string batchId;
    
    vector<SpottingImage> instances;
    void push_back(SpottingImage im) {
        if (im.img.cols>0)
		instances.push_back(im);
    }
    vector<SpottingImage> getSpottings(unsigned int num, unsigned int maxWidth=0) {
        vector<SpottingImage> ret;
        //cout << "num:"<<num << endl;
        //cout << "inst:"<<instances.size() << endl;
        //cout << "exp:"<<((((signed int)instances.size())-num)>3) << " :: "<<(instances.size()-num)<< endl;
        unsigned int toRet = ((((signed int)instances.size())-(signed int) num)>3)?num:instances.size();
        //cout << "toRet:"<<toRet << endl;
        
        for (unsigned int i=0; i<toRet; i++) {
            SpottingImage img=instances.back();
            if (maxWidth!=0 && img.img.cols>maxWidth) {
                int crop = (img.img.cols-maxWidth)/2;
                img.img=img.img(cv::Rect(crop,0,maxWidth,img.img.rows));
            }
            ret.push_back(img);
            _done.push(instances.back());
            instances.pop_back();
            //cout << "popped inst:"<<instances.size() << endl;
        }
        return ret;
    }
    void _reset() {
        while (_done.size()>0) {
            instances.push_back(_done.front());
            _done.pop();
        }
    }
private:
    queue<SpottingImage> _done;
};

/*class scoreComp
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
};*/

class SpottingResults {
public:
    SpottingResults(string ngram) : 
        ngram(ngram) {}
    string ngram;
    
    multimap<float,Spotting,vector<Spotting>> instances;
    void push_back(Spotting spotting) {
        instances.insert(spotting.score,spotting);
    }
    vector<SpottingImage> getSpottings(unsigned int num, unsigned int maxWidth=0) {
        vector<SpottingImage> ret;
        //cout << "num:"<<num << endl;
        //cout << "inst:"<<instances.size() << endl;
        //cout << "exp:"<<((((signed int)instances.size())-num)>3) << " :: "<<(instances.size()-num)<< endl;
        unsigned int toRet = ((((signed int)instances.size())-(signed int) num)>3)?num:instances.size();
        //cout << "toRet:"<<toRet << endl;
        
        for (unsigned int i=0; i<toRet; i++) {
            SpottingImage img=instances.back();
            if (maxWidth!=0 && img.img.cols>maxWidth) {
                int crop = (img.img.cols-maxWidth)/2;
                img.img=img.img(cv::Rect(crop,0,maxWidth,img.img.rows));
            }
            ret.push_back(img);
            _done.push(instances.back());
            instances.pop_back();
            //cout << "popped inst:"<<instances.size() << endl;
        }
        return ret;
    }
    void _reset() {
        while (_done.size()>0) {
            instances.push_back(_done.front());
            _done.pop();
        }
    }
private:
    queue<SpottingImage> _done;
};

class MasterQueue {
private:
    sem_t mutexSem;
    queue< Spottings > spottingsQueue;
    int atID;
public:
    MasterQueue();
    Spottings getBatch(unsigned int numberOfInstances, unsigned int maxWidth=0);
    
};
#endif
