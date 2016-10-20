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
#define THRESH_LEXICON_LOOKUP_COUNT 50
//#define THRESH_SCORING 1.0
#define LOW_COUNT_PRUNE_THRESH 5
#define LOW_COUNT_SCORE_THRESH 0.75
#define THRESH_SCORING_COUNT 7

#define CHAR_ASPECT_RATIO 2.45 //TODO

#define ANCHOR_CONST 500

#define PRUNED_LEXICON_MAX_SIZE 200

#define SHOW 0

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



class Word: public WordBackPointer
{
private:
    pthread_rwlock_t lock;
    int tlx, tly, brx, bry; // top y and bottom y
    string query;
    string gt;
    SearchMeta meta;
    const cv::Mat* pagePnt;
    const Spotter* const* spotter;
    const float* averageCharWidth;
    int* countCharWidth;
    int pageId;
    int spottingIndex;
   
    int topBaseline, botBaseline;

    multimap<int,Spotting> spottings;
    bool done;
    bool loose; //If the user reports and error during trans (not spotting error), we give it a second chance but loosen the regex to take in more possibilities
    unsigned long sentBatchId;

    set<pair<unsigned long,string> > harvested;

    multimap<float,string> scoreAndThresh(vector<string> match) const;
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
    const cv::Mat getWordImg() const;
    cv::Point wordCord(int r, int c)
    {
        return cv::Point(c-tlx,r-tly);
    }
    int wordIndex(int r, int c)
    {
        return (c-tlx) + (brx-tlx+1)*(r-tly);
    }

    string transcription;

    map<unsigned long, vector<Spotting> > removedSpottings;
    void reAddSpottings(unsigned long batchId, vector<Spotting*>* newExemplars);

public:
    //Word() : tlx(-1), tly(-1), brx(-1), bry(-1), pagePnt(NULL), averageCharWidth(NULL), countCharWidth(NULL), pageId(-1), query(""), gt(""), done(false), sentBatchId(0), topBaseline(-1), botBaseline(-1)
    //{
    //    pthread_rwlock_init(&lock,NULL);
    //}    
    
    Word(int tlx, int tly, int brx, int bry, const cv::Mat* pagePnt, const Spotter* const* spotter, const float* averageCharWidth, int* countCharWidth, int pageId) : tlx(tlx), tly(tly), brx(brx), bry(bry), pagePnt(pagePnt), spotter(spotter), averageCharWidth(averageCharWidth), countCharWidth(countCharWidth), pageId(pageId), query(""), gt(""), done(false), loose(false), sentBatchId(0), topBaseline(-1), botBaseline(-1)
    {
        meta = SearchMeta(THRESH_LEXICON_LOOKUP_COUNT);
        pthread_rwlock_init(&lock,NULL);
        assert(tlx>=0 && tly>=0 && brx<pagePnt->cols && bry<pagePnt->rows);
    }
    Word(int tlx, int tly, int brx, int bry, const cv::Mat* pagePnt, const Spotter* const* spotter, const float* averageCharWidth, int* countCharWidth, int pageId, string gt) : tlx(tlx), tly(tly), brx(brx), bry(bry), pagePnt(pagePnt), spotter(spotter), averageCharWidth(averageCharWidth), countCharWidth(countCharWidth), pageId(pageId), query(""), gt(gt), done(false), loose(false), sentBatchId(0), topBaseline(-1), botBaseline(-1)
    {
        meta = SearchMeta(THRESH_LEXICON_LOOKUP_COUNT);
        pthread_rwlock_init(&lock,NULL);
        assert(tlx>=0 && tly>=0 && brx<pagePnt->cols && bry<pagePnt->rows);
    }
    Word(ifstream& in, const cv::Mat* pagePnt, const Spotter* const* spotter, float* averageCharWidth, int* countCharWidth);
    void save(ofstream& out);
    
    ~Word()
    {
        pthread_rwlock_destroy(&lock);
    }
    
    TranscribeBatch* addSpotting(Spotting s,vector<Spotting*>* newExemplars, bool findBatch=true);
    TranscribeBatch* removeSpotting(unsigned long sid, unsigned long batchId, bool resend, unsigned long* sentBatchId, vector<Spotting*>* newExemplars, vector< pair<unsigned long, string> >* toRemoveExemplars);
    TranscribeBatch* removeSpotting(unsigned long sid, unsigned long batchId, bool resend, vector<Spotting*>* newExemplars, vector< pair<unsigned long, string> >* toRemoveExemplars) {return removeSpotting(sid,batchId,resend,NULL,newExemplars,toRemoveExemplars);}
    
    vector<Spotting*> result(string selected, unsigned long batchId, bool resend, vector< pair<unsigned long, string> >* toRemoveExemplars);

    TranscribeBatch* error(unsigned long batchId, bool resend, vector<Spotting*>* newExemplars, vector< pair<unsigned long, string> >* toRemoveExemplars);

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
    void getBoundsAndDoneAndSent(int* word_tlx, int* word_tly, int* word_brx, int* word_bry, bool* isDone, bool* isSent)
    {
        pthread_rwlock_rdlock(&lock);
        *word_tlx=tlx;
        *word_tly=tly;
        *word_brx=brx;
        *word_bry=bry;
	*isDone=done;
        *isSent=sentBatchId!=0;
        pthread_rwlock_unlock(&lock);
    }
    void getBoundsAndDoneAndGT(int* word_tlx, int* word_tly, int* word_brx, int* word_bry, bool* isDone, string* gt)
    {
        pthread_rwlock_rdlock(&lock);
        *word_tlx=tlx;
        *word_tly=tly;
        *word_brx=brx;
        *word_bry=bry;
	*isDone=done;
        *gt=this->gt;
        pthread_rwlock_unlock(&lock);
    }
    void getBoundsAndDoneAndSentAndGT(int* word_tlx, int* word_tly, int* word_brx, int* word_bry, bool* isDone, bool* isSent, string* gt)
    {
        pthread_rwlock_rdlock(&lock);
        *word_tlx=tlx;
        *word_tly=tly;
        *word_brx=brx;
        *word_bry=bry;
	*isDone=done;
        *isSent=sentBatchId!=0;
        *gt=this->gt;
        pthread_rwlock_unlock(&lock);
    }


    vector<Spotting> getSpottings() 
    {
        pthread_rwlock_rdlock(&lock);
        vector<Spotting> ret;
        for (auto p : spottings)
            ret.push_back(p.second);
        pthread_rwlock_unlock(&lock);
        return ret;
    }
    void sent(unsigned long id)
    {
        pthread_rwlock_wrlock(&lock);
        sentBatchId=id;
        pthread_rwlock_unlock(&lock);
    }
    void setSpottingIndex(int index)
    {
        pthread_rwlock_wrlock(&lock);
        spottingIndex=index;
        pthread_rwlock_unlock(&lock);
    }
    int getSpottingIndex()
    {
        int ret;
        pthread_rwlock_rdlock(&lock);
        ret=spottingIndex;
        pthread_rwlock_unlock(&lock);
        return ret;
    }
    const multimap<int,Spotting>* getSpottingsPointer() {return & spottings;}
    vector<string> getRestrictedLexicon(int max);

    const cv::Mat* getPage() const {return pagePnt;}
    int getPageId() const {return pageId;}
    const cv::Mat getImg();// const;
    string getTranscription() 
    {
#ifdef TEST_MODE
        //cout<<"[read] "<<gt<<" ("<<tlx<<","<<tly<<") getTranscription"<<endl;
#endif
        pthread_rwlock_rdlock(&lock); 
        string ret;
        if (done) 
            ret= transcription; 
        else 
            ret = "$ERROR_NONE$"; 
        pthread_rwlock_unlock(&lock);
#ifdef TEST_MODE
        //cout<<"[unlock] "<<gt<<" ("<<tlx<<","<<tly<<") getTranscription"<<endl;
#endif
        return ret;
    }
    string getGT() {return gt;}
    void preapproveSpotting(Spotting* spotting);

    //For data collection, when I deleted all my trans... :(
    TranscribeBatch* reset_(vector<Spotting*>* newExemplars);
};

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







class Corpus : public Dataset
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
    vector<Word*> _words;
    vector<Mat> _wordImgs;
    bool changed;
    void recreateDatasetVectors(bool lockPages);

public:
    Corpus(int contextPad);
    Corpus(ifstream& in);
    void save(ofstream& out);
    ~Corpus()
    {
        pthread_rwlock_destroy(&pagesLock);
        pthread_rwlock_destroy(&spottingsMapLock);
        for (auto p : pages)
            delete p.second;
        delete spotter;
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
    Word* getWord(unsigned int i) const;
    CorpusRef* getCorpusRef();
    PageRef* getPageRef();

    //For data collection, when I deleted all my trans... :(
    vector<TranscribeBatch*> resetAllWords_();
};

}
#endif
