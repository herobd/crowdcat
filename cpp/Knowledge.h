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

#include "spotting.h"
#include "Lexicon.h"
#include "Global.h"
#include "batches.h"
#include "WordBackPointer.h"

using namespace std;

typedef Graph<float,float,float> GraphType;

#define NGRAM_GRAPH_BIAS 0.001f

#define OVERLAP_LINE_THRESH 0.45
#define OVERLAP_WORD_THRESH 0.45
#define THRESH_UNKNOWN_EST 0.2
#define THRESH_LEXICON_LOOKUP_COUNT 20
//#define THRESH_SCORING 1.0
#define THRESH_SCORING_COUNT 6

#define CHAR_ASPECT_RATIO 2.45 //TODO

#define ANCHOR_CONST 500

//#ifndef TEST_MODE_LONG
//#define averageCharWidth 40 //TODO GW, totally just making this up
//#else
//#define averageCharWidth 23 //TODO test, totally just making this up
//#endif




namespace Knowledge
{

//int averageCharWidth=40;

//some general functions
cv::Mat inpainting(const cv::Mat& src, const cv::Mat& mask, double* avg=NULL, double* std=NULL, bool show=false);
int getBreakPoint(int lxBound, int ty, int rxBound, int by, const cv::Mat* pagePnt);
void findPotentailWordBoundraies(Spotting s, int* tlx, int* tly, int* brx, int* bry);



class Word: public WordBackPointer
{
private:
    pthread_rwlock_t lock;
    vector<Word*> _words;
    int tlx, tly, brx, bry; // top y and bottom y
    string query;
    string gt;
    Meta meta;
    const cv::Mat* pagePnt;
    float* averageCharWidth;
    int* countCharWidth;
    int pageId;
   
    int topBaseline, botBaseline;

    multimap<int,Spotting> spottings;
    bool done;
    unsigned long sentBatchId;

    set<pair<unsigned long,string> > harvested;

    multimap<float,string> scoreAndThresh(vector<string> match);
    TranscribeBatch* createBatch(multimap<float,string> scored);
    string generateQuery();
    TranscribeBatch* queryForBatch(vector<Spotting*>* newExemplars);
    vector<Spotting*> harvest();
#ifdef TEST_MODE
    void emergencyAnchor(cv::Mat& b, GraphType* g,int startX, int endX, float sum_anchor, float goal_sum, bool word, cv::Mat& showA);
#else
    void emergencyAnchor(cv::Mat& b, GraphType* g,int startX, int endX, float sum_anchor, float goal_sum, bool word);
#endif
    SpottingExemplar* extractExemplar(int leftLeftBound, int rightLeftBound, int leftRightBound, int rightRightBound, string newNgram, cv::Mat& wordImg, cv::Mat& b);
    void findBaselines(const cv::Mat& gray, const cv::Mat& bin);
    void getWordImgAndBin(cv::Mat& wordImg, cv::Mat& b);
    cv::Point wordCord(int r, int c)
    {
        return cv::Point(c-tlx,r-tly);
    }
    int wordIndex(int r, int c)
    {
        return (c-tlx) + (brx-tlx+1)*(r-tly);
    }

    string transcription;
public:
    Word() : tlx(-1), tly(-1), brx(-1), bry(-1), pagePnt(NULL), averageCharWidth(NULL), countCharWidth(NULL), pageId(-1), query(""), gt(""), done(false), sentBatchId(0), topBaseline(-1), botBaseline(-1)
    {
        pthread_rwlock_init(&lock,NULL);
    }    
    
    Word(int tlx, int tly, int brx, int bry, const cv::Mat* pagePnt, float* averageCharWidth, int* countCharWidth, int pageId) : tlx(tlx), tly(tly), brx(brx), bry(bry), pagePnt(pagePnt), averageCharWidth(averageCharWidth), countCharWidth(countCharWidth), pageId(pageId), query(""), gt(""), done(false), sentBatchId(0), topBaseline(-1), botBaseline(-1)
    {
        pthread_rwlock_init(&lock,NULL);
    }
    Word(int tlx, int tly, int brx, int bry, const cv::Mat* pagePnt, float* averageCharWidth, int* countCharWidth, int pageId, string gt) : tlx(tlx), tly(tly), brx(brx), bry(bry), pagePnt(pagePnt), averageCharWidth(averageCharWidth), countCharWidth(countCharWidth), pageId(pageId), query(""), gt(gt), done(false), sentBatchId(0), topBaseline(-1), botBaseline(-1)
    {
        pthread_rwlock_init(&lock,NULL);
    }
    
    ~Word()
    {
        pthread_rwlock_destroy(&lock);
    }
    
    TranscribeBatch* addSpotting(Spotting s,vector<Spotting*>* newExemplars);
    TranscribeBatch* removeSpotting(unsigned long sid, unsigned long* sentBatchId, vector<Spotting*>* newExemplars, vector< pair<unsigned long, string> >* toRemoveExemplars);
    TranscribeBatch* removeSpotting(unsigned long sid, vector<Spotting*>* newExemplars, vector< pair<unsigned long, string> >* toRemoveExemplars) {return removeSpotting(sid,NULL,newExemplars,toRemoveExemplars);}
    
    void getBaselines(int* top, int* bot);
    void getBoundsAndDone(int* word_tlx, int* word_tly, int* word_brx, int* word_bry, bool* isDone)
    {
        pthread_rwlock_rdlock(&lock);
        *word_tlx=tlx;
        *word_tly=tly;
        *word_brx=brx;
        *word_bry=bry;
	*isDone=done;
        pthread_rwlock_unlock(&lock);
    }

    vector<Spotting*> result(string selected, vector< pair<unsigned long, string> >* toRemoveExemplars)
    {
        cout <<"recived trans: "<<selected<<endl;
        vector<Spotting*> ret;
        pthread_rwlock_wrlock(&lock);
        if (!done)
        {
            done=true;
            transcription=selected;
            ret = harvest();
        }
        else
        {
            //this is a resubmission
            if (transcription.compare(selected)!=0)
            {
                //Retract harvested ngrams
                toRemoveExemplars->insert(toRemoveExemplars->end(),harvested.begin(),harvested.end());
                transcription=selected;
                ret= harvest();
            }
        }
        pthread_rwlock_unlock(&lock);
        return ret;
        //Harvested ngrams should be approved before spotting with them
    }

    void error(vector< pair<unsigned long, string> >* toRemoveExemplars)
    {
        spottings.clear();
        toRemoveExemplars->insert(toRemoveExemplars->end(),harvested.begin(),harvested.end());
        harvested.clear();
    }

    vector<Spotting> getSpottings() 
    {
        vector<Spotting> ret;
        for (auto p : spottings)
            ret.push_back(p.second);
        return ret;
    }

    const cv::Mat* getPage() {return pagePnt;}
    string getTranscription() {pthread_rwlock_rdlock(&lock); if (done) return transcription; else return "[ERROR]"; pthread_rwlock_unlock(&lock);}
};

class Line
{
private:
    pthread_rwlock_t lock;
    vector<Word*> _words;
    int ty, by; // top y and bottom y
    cv::Mat* pagePnt;
    float* averageCharWidth;
    int* countCharWidth;
    int pageId;
public:
    Line() : ty(-1), by(-1), pagePnt(NULL), averageCharWidth(NULL), countCharWidth(NULL), pageId(-1)
    {
        pthread_rwlock_init(&lock,NULL);
    }
    
    Line(int ty, int by, cv::Mat* pagePnt, float* averageCharWidth, int* countCharWidth, int pageId) : ty(ty), by(by), pagePnt(pagePnt), averageCharWidth(averageCharWidth), countCharWidth(countCharWidth), pageId(pageId)
    {
        pthread_rwlock_init(&lock,NULL);
    }
    
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
        Word* newWord = new Word(tlx,tly,brx,bry,pagePnt,averageCharWidth,countCharWidth,pageId);
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
        Word* newWord = new Word(tlx,tly,brx,bry,pagePnt,averageCharWidth,countCharWidth,pageId,gt);
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
    float* averageCharWidth;
    int* countCharWidth;
    static int _id;
    int id;
public:
    Page() : averageCharWidth(NULL), countCharWidth(NULL)
    {
        pthread_rwlock_init(&lock,NULL);
        id = ++_id;
    }
    Page(cv::Mat pageImg, float* averageCharWidth, int* countCharWidth) : pageImg(pageImg), averageCharWidth(averageCharWidth), countCharWidth(countCharWidth)
    {
        pthread_rwlock_init(&lock,NULL);
        id = ++_id;
    }
    Page(string imageLoc, float* averageCharWidth, int* countCharWidth) : averageCharWidth(averageCharWidth), countCharWidth(countCharWidth) 
    {
        pageImg = cv::imread(imageLoc);//,CV_LOAD_IMAGE_GRAYSCALE
        id = ++_id;
        pthread_rwlock_init(&lock,NULL);
    }
    
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
        Line* newLine = new Line(s.tly, s.bry, &pageImg, averageCharWidth, countCharWidth, id);
        
        
        pthread_rwlock_wrlock(&lock);
        _lines.push_back(newLine);
        pthread_rwlock_unlock(&lock);
        
        return newLine->addWord(s,newExemplars);
    }

    TranscribeBatch* addWord(int tlx, int tly, int brx, int bry, string gt)
    {
        Line* newLine = new Line(tly, bry, &pageImg, averageCharWidth, countCharWidth, id);
        
        
        pthread_rwlock_wrlock(&lock);
        _lines.push_back(newLine);
        pthread_rwlock_unlock(&lock);
        
        return newLine->addWord(tlx,tly,brx,bry,gt);
    }

    const cv::Mat* getImg() const {return &pageImg;}
    int getId() const {return id;}
};







class Corpus
{
private:
    pthread_rwlock_t pagesLock;
    pthread_rwlock_t spottingsMapLock;
    pthread_rwlock_t batchLock;
    float averageCharWidth;
    int countCharWidth;
    float threshScoring;
    
    map<unsigned long, vector<Word*> > spottingsToWords;
    map<int,Page*> pages;
    deque<TranscribeBatch*> leftoverQueue;
    map<unsigned long, TranscribeBatch*> returnMap;
    map<unsigned long, chrono::system_clock::time_point> timeMap;

    void addSpottingToPage(Spotting& s, Page* page, vector<TranscribeBatch*>& ret,vector<Spotting*>* newExemplars);
    void checkIncomplete();

public:
    Corpus();
    ~Corpus()
    {
        pthread_rwlock_destroy(&pagesLock);
        for (auto p : pages)
            delete p.second;
    }
    vector<TranscribeBatch*> addSpotting(Spotting s,vector<Spotting*>* newExemplars);
    //vector<TranscribeBatch*> addSpottings(vector<Spotting> spottings);
    vector<TranscribeBatch*> updateSpottings(vector<Spotting>* spottings, vector<pair<unsigned long, string> >* removeSpottings, vector<unsigned long>* toRemoveBatches,vector<Spotting*>* newExemplars, vector< pair<unsigned long, string> >* toRemoveExemplars);
    //void removeSpotting(unsigned long sid);
    TranscribeBatch* getManualBatch(int maxWidth);
    void transcriptionFeedback(unsigned id, string transcription);
    void addWordSegmentaionAndGT(string imageLoc, string queriesFile);
    const cv::Mat* imgForPageId(int pageId) const;
    int addPage(string imagePath) 
    {
        Page* p = new Page(imagePath,&averageCharWidth,&countCharWidth);
        pages[p->getId()]=p;
        return p->getId();
    }
    void show();
    void showProgress(int height, int width);
};

}
#endif
