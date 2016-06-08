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

#include "SpottingResults.h"
#include "Lexicon.h"


using namespace std;

#define OVERLAP_LINE_THRESH 0.5
#define OVERLAP_WORD_THRESH 0.6
#define THRESH_UNKNOWN_EST 0.2
#define THRESH_LEXICON_LOOKUP_COUNT 20
//#define THRESH_SCORING 1.0
#define THRESH_SCORING_COUNT 6
//#define averageCharWidth 40 //GW, totally just making this up

class TranscribeBatch
{
private:
    vector<string> possibilities;
    cv::Mat wordImg;
    cv::Mat ngramLocs;
public:
    TranscribeBatch(vector<string> possibilities, cv::Mat wordImg, cv::Mat ngramLocs) : 
        possibilities(possibilities), wordImg(wordImg), ngramLocs(ngramLocs) {}
    
    const vector<string>& getPossibilities() {return possibilities;}
};

namespace Knowledge
{

int averageCharWidth=40;

//some general functions
void findPotentailWordBoundraies(Spotting s, int* tlx, int* tly, int* brx, int* bry);



class Word
{
private:
    pthread_rwlock_t lock;
    vector<Word*> _words;
    int tlx, tly, brx, bry; // top y and bottom y
    string query;
    Meta meta;
    
    multimap<int,Spotting> spottings;
    
    multimap<float,string> scoreAndThresh(vector<string> match);
    TranscribeBatch* createBatch(multimap<float,string> scored);
    string generateQuery();
    
public:
    Word() : tlx(-1), tly(-1), brx(-1), bry(-1)
    {
        pthread_rwlock_init(&lock,NULL);
    }    
    
    Word(int tlx, int tly, int brx, int bry) : tlx(tlx), tly(tly), brx(brx), bry(bry), query("")
    {
        pthread_rwlock_init(&lock,NULL);
    }
    
    ~Word()
    {
        pthread_rwlock_destroy(&lock);
    }
    
    TranscribeBatch* addSpotting(Spotting s);
    
    void getBounds(int* word_tlx, int* word_tly, int* word_brx, int* word_bry)
    {
        pthread_rwlock_rdlock(&lock);
        *word_tlx=tlx;
        *word_tly=tly;
        *word_brx=brx;
        *word_bry=bry;
        pthread_rwlock_unlock(&lock);
    }

};

class Line
{
private:
    pthread_rwlock_t lock;
    vector<Word*> _words;
    int ty, by; // top y and bottom y
public:
    Line() : ty(-1), by(-1)
    {
        pthread_rwlock_init(&lock,NULL);
    }
    
    Line(int ty, int by) : ty(ty), by(by)
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
        Word* newWord = new Word(tlx,tly,brx,bry);
        pthread_rwlock_wrlock(&lock);
         _words.push_back(newWord);
        pthread_rwlock_unlock(&lock);
        
        return newWord->addSpotting(s);
    }
};

class Page
{
private:
    pthread_rwlock_t lock;
    vector<Line*> _lines;
public:
    Page()
    {
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
        Line* newLine = new Line(s.tly, s.bry);
        
        
        pthread_rwlock_wrlock(&lock);
        _lines.push_back(newLine);
        pthread_rwlock_unlock(&lock);
        
        return newLine->addWord(s);
    }
};







class Corpus
{
private:
    pthread_rwlock_t pagesLock;
    
    int averageCharWidth;
    float threshScoring;
    
    map<unsigned long, vector<Word*> > spottingsToWords;
    vector<Page*> pages;
public:
    Corpus();
    ~Corpus()
    {
        pthread_rwlock_destroy(&pagesLock);
    }
    void addSpotting(Spotting s);
};

}
#endif
