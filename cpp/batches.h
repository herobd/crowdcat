#ifndef BATCHES_H
#define BATCHES_H

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <vector>
#include <map>
#include <chrono>
#include <atomic>

#include <mutex>
#include <iostream>

#include "spotting.h"

using namespace std;

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
    static std::atomic_ulong _batchId;
    vector<SpottingImage> instances;
};


class TranscribeBatch
{
private:
    WordBackPointer* origin;
    vector<string> possibilities;
    cv::Mat wordImg;
    cv::Mat newWordImg;
    //cv::Mat textImg;
    //cv::Mat newTextImg;
    const cv::Mat* origImg;
    const multimap<int,Spotting>* spottings;
    unsigned int imgWidth;
    unsigned long id;
    int tlx, tly, brx, bry;
    vector<SpottingPoint> spottingPoints;
    bool manual;
    static vector< cv::Vec3f > colors;
    static cv::Vec3f wordHighlight;
    static std::atomic_ulong _id;
    static void highlightPix(cv::Vec3b &p, cv::Vec3f color);
public:
    //TranscribeBatch(vector<string> possibilities, cv::Mat wordImg, cv::Mat ngramLocs) : 
    //    possibilities(possibilities), wordImg(wordImg), ngramLocs(ngramLocs) {id = ++_id;}
    
    //A normal transcription batch
    TranscribeBatch(WordBackPointer* origin, multimap<float,string> scored, const cv::Mat* origImg, const multimap<int,Spotting>* spottings, int tlx, int tly, int brx, int bry, unsigned long batchId=0);
    //A manual transcription batch
    TranscribeBatch(WordBackPointer* origin, vector<string> prunedDictionary, const cv::Mat* origImg, const multimap<int,Spotting>* spottings, int tlx, int tly, int brx, int bry, unsigned long batchId=0);
    
    const vector<string>& getPossibilities() {return possibilities;}
    cv::Mat getImage() { if (newWordImg.cols!=0) return newWordImg; return wordImg;}
    //cv::Mat getTextImage() { if (newTextImg.cols!=0) return newTextImg; return textImg;}
    unsigned long getId() {return id;}
    WordBackPointer* getBackPointer() {return origin;}
    void setWidth(unsigned int width);
    vector<SpottingPoint> getSpottingPoints() {return spottingPoints;}
};


class NewExemplarsBatch {
public:
    
    NewExemplarsBatch(const vector<Spotting*>& exes, unsigned int maxWidth, int color)
    {
        batchId = _batchId++;
        for (Spotting* s : exes)
        {
            instances.push_back(SpottingImage(*s,maxWidth,color));
            delete s;
        }

    }
    
    
    SpottingImage operator [](int i) const    {return instances[i];}
    SpottingImage & operator [](int i) {return instances[i];}
    SpottingImage at(int i) const    {return instances.at(i);}
    SpottingImage & at(int i) {return instances.at(i);}
    unsigned int size() const { return instances.size();}
    unsigned long getId() {return batchId;}
    
private:
    static std::atomic_ulong _batchId;
    unsigned long batchId;
    vector<SpottingImage> instances;
};

#endif
