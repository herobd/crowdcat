#ifndef BATCHES_H
#define BATCHES_H

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <vector>
#include <map>
#include <chrono>
#include <atomic>
#include <iomanip>

#include <mutex>
#include <iostream>

#include "spotting.h"
#include "WordBackPointer.h"
#include "CorpusRef.h"
#include "Global.h"

using namespace std;


class SpottingPoint
{
    public:
        SpottingPoint() : x(-1), ngram(""), x1(-1), y1(-1), x2(-1), y2(-1), page(-1), id(""), color("")
        {}
        SpottingPoint(unsigned long id, int x, string ngram, int b, int g, int r,  
                int page, int x1, int y1, int x2, int y2) : x(x), ngram(ngram), x1(x1), y1(y1), x2(x2), y2(y2), page(page)
        {
            stringstream stream;
            if (b>255)
                b=255;
            if (g>255)
                g=255;
            if (r>255)
                r=255;
            stream << setfill('0') << setw(sizeof(unsigned char)*2) << hex << r<<g<<b;
            color = stream.str();
            this->id = to_string(id);
        }
        void setPad(int pad) {this->pad=pad;}
        string getX() {return to_string(pad+x);}
        string getNgram() {return ngram;}
        string getColor() {return color;}
        string getId() {return id;}
        int page, x1,y1,x2,y2;

        SpottingPoint(ifstream& in)
        {
            string line;
            getline(in,line);
            page=stoi(line);
            getline(in,line);
            x1=stoi(line);
            getline(in,line);
            y1=stoi(line);
            getline(in,line);
            x2=stoi(line);
            getline(in,line);
            y2=stoi(line);
            getline(in,line);
            x=stoi(line);
            getline(in,line);
            page=stoi(line);
            getline(in,ngram);
            getline(in,color);
            getline(in,id);
        }
        void save(ofstream& out)
        {
            out<<page<<"\n";
            out<<x1<<"\n"<<y1<<"\n"<<x2<<"\n"<<y2<<"\n";
            out<<x<<"\n"<<pad<<"\n";
            out<<ngram<<"\n";
            out<<color<<"\n";
            out<<id<<"\n";
        }
    private:
        int x, pad;
        string ngram, color, id;
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
    SpottingImage operator [](int i) const    {return instances.at(i);}
    SpottingImage & operator [](int i) {return instances.at(i);}
    SpottingImage at(int i) const    {return instances.at(i);}
    SpottingImage & at(int i) {return instances.at(i);}
    unsigned int size() const { return instances.size();}
    
    virtual ~SpottingsBatch()
    {
    }

    //For saving and loading
    static unsigned long getIdCounter() {return _batchId.load();}
    static void setIdCounter(unsigned long id) {_batchId.store(id);}
    
private:
    static std::atomic_ulong _batchId;
    vector<SpottingImage> instances;
};

class WordBackPointer;

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
    double scale;
    string gt;
    bool manual;
    static vector< cv::Vec3f > colors;
    static cv::Vec3f wordHighlight;
    static std::atomic_ulong _id;
    static void highlightPix(cv::Vec3b &p, cv::Vec3f color);
public:
    //TranscribeBatch(vector<string> possibilities, cv::Mat wordImg, cv::Mat ngramLocs) : 
    //    possibilities(possibilities), wordImg(wordImg), ngramLocs(ngramLocs) {id = ++_id;}
    
    //A normal transcription batch
    TranscribeBatch(WordBackPointer* origin, multimap<float,string> scored, const cv::Mat* origImg, const multimap<int,Spotting>* spottings, int tlx, int tly, int brx, int bry, string gt="$UNKOWN$", unsigned long batchId=0);
    //A manual transcription batch
    TranscribeBatch(WordBackPointer* origin, vector<string> prunedDictionary, const cv::Mat* origImg, const multimap<int,Spotting>* spottings, int tlx, int tly, int brx, int bry, string gt="$UNKNOWN$", unsigned long batchId=0);
    
    void init(WordBackPointer* origin, const cv::Mat* origImg, const multimap<int,Spotting>* spottings, int tlx, int tly, int brx, int bry, string gt, unsigned long id);

    TranscribeBatch(ifstream& in, CorpusRef* corpusRef);
    void save(ofstream& out);

    const vector<string>& getPossibilities() {return possibilities;}
    cv::Mat getImage() { if (newWordImg.cols!=0) return newWordImg; return wordImg;}
    //cv::Mat getTextImage() { if (newTextImg.cols!=0) return newTextImg; return textImg;}
    unsigned long getId() {return id;}
    WordBackPointer* getBackPointer() {return origin;}
    void setWidth(unsigned int width, int contextPad);
    vector<SpottingPoint> getSpottingPoints() {return spottingPoints;}
    bool isManual() {return manual;}
    string getGT() {return gt;}
    double getScale() {return scale;}

    //For saving and loading
    static unsigned long getIdCounter() {return _id.load();}
    static void setIdCounter(unsigned long id) {_id.store(id);}
};


class NewExemplarsBatch {
public:
    
    NewExemplarsBatch(const vector<Spotting*>& exes, unsigned int maxWidth, int color)
    {
        batchId = _batchId++;
        for (Spotting* s : exes)
        {
            instances.push_back(SpottingImage(*s,maxWidth,0,color));
            delete s;
        }

    }
    
    
    SpottingImage operator [](int i) const    {return instances[i];}
    SpottingImage & operator [](int i) {return instances[i];}
    SpottingImage at(int i) const    {return instances.at(i);}
    SpottingImage & at(int i) {return instances.at(i);}
    unsigned int size() const { return instances.size();}
    unsigned long getId() {return batchId;}
    
    //For saving and loading
    static unsigned long getIdCounter() {return _batchId.load();}
    static void setIdCounter(unsigned long id) {_batchId.store(id);}
    
private:
    static std::atomic_ulong _batchId;
    unsigned long batchId;
    vector<SpottingImage> instances;
};

#endif
