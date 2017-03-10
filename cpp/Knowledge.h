#ifndef Knowledge_H
#define Knowledge_H


#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <vector>
#include <queue>
#include <deque>
#include <chrono>
#include <iostream>
#include <semaphore.h>
#include <pthread.h>
#include <atomic>
#include "maxflow/graph.h"
#include <regex>
#include <assert.h>

#include "Word.h"
#include "spotting.h"
#include "Lexicon.h"
#include "Global.h"
#include "batches.h"
#include "WordBackPointer.h"
#include "TranscribeBatchQueue.h"
#include "dataset.h"
#include "Spotter.h"
#include "AlmazanSpotter.h"
#include "CorpusRef.h"
#include "PageRef.h"

using namespace std;

typedef Graph<float,float,float> GraphType;

#define NGRAM_GRAPH_BIAS 0.001f

#define OVERLAP_LINE_THRESH 0.9
#define OVERLAP_WORD_THRESH 0.45
#define THRESH_UNKNOWN_EST 0.3
#define THRESH_LEXICON_LOOKUP_COUNT 49
//#define THRESH_SCORING 1.0
#define LOW_COUNT_PRUNE_THRESH 5
#define LOW_COUNT_SCORE_THRESH 0.75
#define THRESH_SCORING_COUNT 7

#define CHAR_ASPECT_RATIO 2.45 //TODO

#define ANCHOR_CONST 500

#define PRUNED_LEXICON_MAX_SIZE 200

#define SHOW 0

#define AUTO_TRANS_ON_ONE 0

//#ifndef TEST_MODE_LONG
//#define averageCharWidth 40 //TODO GW, totally just making this up
//#else
//#define averageCharWidth 23 //TODO test, totally just making this up
//#endif




namespace Knowledge
{

//int averageCharWidth=40;

//some general functions
cv::Mat inpainting(const cv::Mat& src, const cv::Mat& mask, double* avg=NULL, double* std=NULL, bool show=SHOW);
int getBreakPoint(int lxBound, int ty, int rxBound, int by, const cv::Mat* pagePnt);
void findPotentailWordBoundraies(Spotting s, int* tlx, int* tly, int* brx, int* bry);




class Line
{
private:
    pthread_rwlock_t lock;
    vector<Word*> _words;
    int ty, by; // top y and bottom y
    const cv::Mat* pagePnt;
    const Spotter* const* spotter;
    float* averageCharWidth;
    int* countCharWidth;
    int pageId;
public:
    //Line() : ty(-1), by(-1), pagePnt(NULL), averageCharWidth(NULL), countCharWidth(NULL), pageId(-1)
    //{
    //    pthread_rwlock_init(&lock,NULL);
    //}
    
    Line(int ty, int by, const cv::Mat* pagePnt, const Spotter* const* spotter, float* averageCharWidth, int* countCharWidth, int pageId) : ty(ty), by(by), pagePnt(pagePnt), spotter(spotter), averageCharWidth(averageCharWidth), countCharWidth(countCharWidth), pageId(pageId)
    {
        pthread_rwlock_init(&lock,NULL);
    }
    Line(ifstream& in, const cv::Mat* pagePnt, const Spotter* const* spotter, float* averageCharWidth, int* countCharWidth);
    void save(ofstream& out);
    
    ~Line()
    {
        pthread_rwlock_destroy(&lock);
    }
    
    vector<Word*> wordsAndBounds(int* line_ty, int* line_by)
    {
        pthread_rwlock_rdlock(&lock);
        
        vector<Word*> ret = _words;
        if (line_ty!=NULL)
            *line_ty=ty;
        if (line_by!=NULL)
            *line_by=by;
        
        pthread_rwlock_unlock(&lock);
        return ret;
    }
    
    TranscribeBatch* addWord(Spotting s,vector<Spotting*>* newExemplars)
    {
        int tlx, tly, brx, bry;
        findPotentailWordBoundraies(s,&tlx,&tly,&brx,&bry);
        Word* newWord = new Word(tlx,tly,brx,bry,pagePnt,spotter,averageCharWidth,countCharWidth,pageId);
        pthread_rwlock_wrlock(&lock);
         _words.push_back(newWord);
        pthread_rwlock_unlock(&lock);
        
        return newWord->addSpotting(s,newExemplars);
    }
    TranscribeBatch* addWord(int tlx, int tly, int brx, int bry, string gt)
    {
        //Becuase this occurs with an absolute word boundary, we adjust the line to match the word
        if (ty>tly)
            ty=tly;
        if (by<bry)
            by=bry;
        Word* newWord = new Word(tlx,tly,brx,bry,pagePnt,spotter,averageCharWidth,countCharWidth,pageId,gt);
        pthread_rwlock_wrlock(&lock);
         _words.push_back(newWord);
        pthread_rwlock_unlock(&lock);
        
        return NULL;
    }
};

class Page
{
private:
    pthread_rwlock_t lock;
    vector<Line*> _lines;
    cv::Mat pageImg; //I am owner of this Mat
    string pageImgLoc;
    const Spotter* const* spotter;
    float* averageCharWidth;
    int* countCharWidth;
    static int _id;
    int id;
public:
    //Page() : averageCharWidth(NULL), countCharWidth(NULL)
    //{
    //    pthread_rwlock_init(&lock,NULL);
    //    id = ++_id;
    //}
    Page(cv::Mat pageImg, const Spotter* const* spotter, float* averageCharWidth, int* countCharWidth) : pageImg(pageImg), spotter(spotter), averageCharWidth(averageCharWidth), countCharWidth(countCharWidth), pageImgLoc("")
    {
        pthread_rwlock_init(&lock,NULL);
        id = ++_id;
    }
    Page(cv::Mat pageImg, const Spotter* const* spotter, float* averageCharWidth, int* countCharWidth, int id) : pageImg(pageImg), spotter(spotter), averageCharWidth(averageCharWidth), countCharWidth(countCharWidth), pageImgLoc("")
    {
        pthread_rwlock_init(&lock,NULL);
        this->id = id;
        _id = id;
    }
    Page( const Spotter* const* spotter, string imageLoc, float* averageCharWidth, int* countCharWidth) : spotter(spotter), averageCharWidth(averageCharWidth), countCharWidth(countCharWidth), pageImgLoc(imageLoc)
    {
        pageImg = cv::imread(imageLoc);//,CV_LOAD_IMAGE_GRAYSCALE
        id = ++_id;
        pthread_rwlock_init(&lock,NULL);
    }
    Page( const Spotter* const* spotter, string imageLoc, float* averageCharWidth, int* countCharWidth, int id) : spotter(spotter), averageCharWidth(averageCharWidth), countCharWidth(countCharWidth), pageImgLoc(imageLoc)
    {
        pageImg = cv::imread(imageLoc);//,CV_LOAD_IMAGE_GRAYSCALE
        if (pageImg.cols*pageImg.rows<=1)
            cout<<"ERROR: could not open image: "<<imageLoc<<endl;
        assert(pageImg.cols*pageImg.rows > 1);
        this->id = id;
        _id = id;
        pthread_rwlock_init(&lock,NULL);
    }
    Page(ifstream& in, const Spotter* const* spotter, float* averageCharWidth, int* countCharWidth);
    void save(ofstream& out);
    
    ~Page()
    {
        pthread_rwlock_destroy(&lock);
    }
    
    vector<Line*> lines()
    {
        pthread_rwlock_rdlock(&lock);
        
        vector<Line*> ret = _lines;
        
        pthread_rwlock_unlock(&lock);
        return ret;
    }
    
    TranscribeBatch* addLine(Spotting s,vector<Spotting*>* newExemplars)
    {
        Line* newLine = new Line(s.tly, s.bry, &pageImg, spotter, averageCharWidth, countCharWidth, id);
        
        
        pthread_rwlock_wrlock(&lock);
        _lines.push_back(newLine);
        pthread_rwlock_unlock(&lock);
        
        return newLine->addWord(s,newExemplars);
    }

    TranscribeBatch* addWord(int tlx, int tly, int brx, int bry, string gt)
    {
        Line* newLine = new Line(tly, bry, &pageImg, spotter, averageCharWidth, countCharWidth, id);
        
        
        pthread_rwlock_wrlock(&lock);
        _lines.push_back(newLine);
        pthread_rwlock_unlock(&lock);
        
        return newLine->addWord(tlx,tly,brx,bry,gt);
    }

    const cv::Mat* getImg() const {return &pageImg;}
    string getPageImgLoc() const {return pageImgLoc;}
    int getId() const {return id;}
};







class Corpus : public CorpusDataset
{
private:
    pthread_rwlock_t pagesLock;
    pthread_rwlock_t spottingsMapLock;
    float averageCharWidth;
    int countCharWidth;
    float threshScoring;
    
    map<int,Page*> pages;
    map<string,int> pageIdMap;

    int numWordsReadIn;

    map<unsigned long, vector<Word*> > spottingsToWords;
    Spotter* spotter;
    TranscribeBatchQueue manQueue;
    TranscribeBatch* makeManualBatch(int maxWidth, bool noSpottings);

    void addSpottingToPage(Spotting& s, Page* page, vector<TranscribeBatch*>& ret,vector<Spotting*>* newExemplars);

    vector<string> _gt;
    map<unsigned long, Word*> _words;
    mat<unsigned long, Mat> _wordImgs;
    bool changed;
    void recreateDatasetVectors(bool lockPages);

public:
    Corpus(int contextPad, int averageCharWidth);
    Corpus(ifstream& in);
    void save(ofstream& out);
    ~Corpus()
    {
        pthread_rwlock_wrlock(&pagesLock);
        pthread_rwlock_wrlock(&spottingsMapLock);
        
        for (auto p : pages)
            delete p.second;
        delete spotter;
        pthread_rwlock_destroy(&pagesLock);
        pthread_rwlock_destroy(&spottingsMapLock);
    }
    void loadSpotter(string modelPrefix);
    vector<TranscribeBatch*> addSpotting(Spotting s,vector<Spotting*>* newExemplars);
    //vector<TranscribeBatch*> addSpottings(vector<Spotting> spottings);
    vector<TranscribeBatch*> updateSpottings(vector<Spotting>* spottings, vector<pair<unsigned long, string> >* removeSpottings, vector<unsigned long>* toRemoveBatches,vector<Spotting*>* newExemplars, vector< pair<unsigned long, string> >* toRemoveExemplars);
    //void removeSpotting(unsigned long sid);
    TranscribeBatch* getManualBatch(int maxWidth);
    vector<Spotting*> transcriptionFeedback(unsigned long id, string transcription, vector<pair<unsigned long, string> >* toRemoveExemplars);
    void addWordSegmentaionAndGT(string imageLoc, string queriesFile);
    const cv::Mat* imgForPageId(int pageId) const;
    int addPage(string imagePath) 
    {
        Page* p = new Page(&spotter, imagePath,&averageCharWidth,&countCharWidth);
        pages[p->getId()]=p;
        return p->getId();
    }

    vector<Spotting>* runQuery(SpottingQuery* query);// const;

    void checkIncomplete();
    void show();
    void showProgress(int height, int width);

    const vector<string>& labels() const;
    int size() const;
    const cv::Mat image(unsigned int i) const;
    unsigned int wordId(unsigned int i) const;
    Word* getWord(unsigned int i) const;
    Word* word(unsigned int i);
    const Word* word(unsigned int i) const;
    
    CorpusRef* getCorpusRef();
    PageRef* getPageRef();

    //For data collection, when I deleted all my trans... :(
    vector<TranscribeBatch*> resetAllWords_();
    void getStats(float* accTrans, float* pWordsTrans, float* pWords80_100, float* pWords60_80, float* pWords40_60, float* pWords20_40, float* pWords0_20, float* pWords0, string* misTrans,
                          float* accTrans_IV, float* pWordsTrans_IV, float* pWords80_100_IV, float* pWords60_80_IV, float* pWords40_60_IV, float* pWords20_40_IV, float* pWords0_20_IV, float* pWords0_IV, string* misTrans_IV);

    static void mouseCallBackFunc(int event, int x, int y, int flags, void* page_p);
    void showInteractive(int pageId);
    void writeTranscribed(string retrainFile);
};

}
#endif
