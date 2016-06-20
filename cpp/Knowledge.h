#ifndef Knowledge_H
#define Knowledge_H


#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <vector>
#include <queue>
#include <iostream>
#include <semaphore.h>
#include <pthread.h>
#include <atomic>

#include "SpottingResults.h"
#include "Lexicon.h"


using namespace std;

#define OVERLAP_LINE_THRESH 0.45
#define OVERLAP_WORD_THRESH 0.45
#define THRESH_UNKNOWN_EST 0.2
#define THRESH_LEXICON_LOOKUP_COUNT 20
//#define THRESH_SCORING 1.0
#define THRESH_SCORING_COUNT 6
#define averageCharWidth 40 //GW, totally just making this up

class WordBackPointer
{
    public:
        virtual void result(string selected)= 0;
};

class TranscribeBatch
{
private:
    WordBackPointer* origin;
    vector<string> possibilities;
    cv::Mat wordImg;
    cv::Mat textImg;
    const multimap<int,Spotting>* spottings;
    unsigned int imgWidth;
    unsigned long id;
    int tlx, tly, brx, bry;
    static vector< cv::Vec3f > colors;
    static std::atomic_ulong _id;
public:
    //TranscribeBatch(vector<string> possibilities, cv::Mat wordImg, cv::Mat ngramLocs) : 
    //    possibilities(possibilities), wordImg(wordImg), ngramLocs(ngramLocs) {id = ++_id;}
    
    TranscribeBatch(WordBackPointer* origin, multimap<float,string> scored, const cv::Mat wordImg, const multimap<int,Spotting>* spottings, int tlx, int tly, int brx, int bry, unsigned long batchId=0);
    
    const vector<string>& getPossibilities() {return possibilities;}
    cv::Mat getImage() { return wordImg; }
    cv::Mat getTextImage() { return textImg; }
    unsigned long getId() {return id;}
    WordBackPointer* getBackPointer() {return origin;}
    void setWidth(unsigned int width);
};

namespace Knowledge
{

//int averageCharWidth=40;

//some general functions
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
    
    multimap<int,Spotting> spottings;
    bool done;
    unsigned long sentBatchId;
    multimap<float,string> scoreAndThresh(vector<string> match);
    TranscribeBatch* createBatch(multimap<float,string> scored);
    string generateQuery();
    TranscribeBatch* queryForBatch();
    string transcription;
public:
    Word() : tlx(-1), tly(-1), brx(-1), bry(-1), pagePnt(NULL), query(""), gt(""), done(false), sentBatchId(0)
    {
        pthread_rwlock_init(&lock,NULL);
    }    
    
    Word(int tlx, int tly, int brx, int bry, const cv::Mat* pagePnt) : tlx(tlx), tly(tly), brx(brx), bry(bry), pagePnt(pagePnt), query(""), gt(""), done(false), sentBatchId(0)
    {
        pthread_rwlock_init(&lock,NULL);
    }
    Word(int tlx, int tly, int brx, int bry, const cv::Mat* pagePnt, string gt) : tlx(tlx), tly(tly), brx(brx), bry(bry), pagePnt(pagePnt), query(""), gt(gt), done(false), sentBatchId(0)
    {
        pthread_rwlock_init(&lock,NULL);
    }
    
    ~Word()
    {
        pthread_rwlock_destroy(&lock);
    }
    
    TranscribeBatch* addSpotting(Spotting s);
    TranscribeBatch* removeSpotting(unsigned long sid, unsigned long* sentBatchId);
    
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

    void result(string selected)
    {
        pthread_rwlock_wrlock(&lock);
        transcription=selected;
        if (!done)
            done=true;
        else
        {
            //TODO this is a resubmission
        }
        pthread_rwlock_unlock(&lock);
        //TODO, harvest new ngram exemplars, undo those already harvested
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
public:
    Line() : ty(-1), by(-1), pagePnt(NULL)
    {
        pthread_rwlock_init(&lock,NULL);
    }
    
    Line(int ty, int by, cv::Mat* pagePnt) : ty(ty), by(by), pagePnt(pagePnt)
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
        *line_ty=ty;
        *line_by=by;
        
        pthread_rwlock_unlock(&lock);
        return ret;
    }
    
    TranscribeBatch* addWord(Spotting s)
    {
        int tlx, tly, brx, bry;
        findPotentailWordBoundraies(s,&tlx,&tly,&brx,&bry);
        Word* newWord = new Word(tlx,tly,brx,bry,pagePnt);
        pthread_rwlock_wrlock(&lock);
         _words.push_back(newWord);
        pthread_rwlock_unlock(&lock);
        
        return newWord->addSpotting(s);
    }
    TranscribeBatch* addWord(int tlx, int tly, int brx, int bry, string gt)
    {
        //Becuase this occurs with an absolute word boundary, we adjust the line to match the word
        if (ty>tly)
            ty=tly;
        if (by<bry)
            by=bry;
        Word* newWord = new Word(tlx,tly,brx,bry,pagePnt,gt);
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
public:
    Page()
    {
        pthread_rwlock_init(&lock,NULL);
    }
    Page(cv::Mat pageImg) : pageImg(pageImg)
    {
        pthread_rwlock_init(&lock,NULL);
    }
    Page(string imageLoc) 
    {
        pageImg = cv::imread(imageLoc);
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
    
    TranscribeBatch* addLine(Spotting s)
    {
        Line* newLine = new Line(s.tly, s.bry, &pageImg);
        
        
        pthread_rwlock_wrlock(&lock);
        _lines.push_back(newLine);
        pthread_rwlock_unlock(&lock);
        
        return newLine->addWord(s);
    }

    TranscribeBatch* addWord(int tlx, int tly, int brx, int bry, string gt)
    {
        Line* newLine = new Line(tly, bry, &pageImg);
        
        
        pthread_rwlock_wrlock(&lock);
        _lines.push_back(newLine);
        pthread_rwlock_unlock(&lock);
        
        return newLine->addWord(tlx,tly,brx,bry,gt);
    }

    cv::Mat* getImg() {return &pageImg;}
};







class Corpus
{
private:
    pthread_rwlock_t pagesLock;
    pthread_rwlock_t spottingsMapLock;
    //int averageCharWidth;
    float threshScoring;
    
    map<unsigned long, vector<Word*> > spottingsToWords;
    map<int,Page*> pages;
    void addSpottingToPage(Spotting& s, Page* page, vector<TranscribeBatch*>& ret);

public:
    Corpus();
    ~Corpus()
    {
        pthread_rwlock_destroy(&pagesLock);
    }
    vector<TranscribeBatch*> addSpotting(Spotting s);
    vector<TranscribeBatch*> addSpottings(vector<Spotting>* spottings);
    void removeSpotting(unsigned long sid);

    void addWordSegmentaionAndGT(string imageLoc, string queriesFile);
    cv::Mat* imgForPageId(int pageId);
    void show();
};

}
#endif
