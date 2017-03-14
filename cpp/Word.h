#ifndef WORD_H
#define WORD_H
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <vector>
#include <queue>
#include <deque>
#include <chrono>
#include <iostream>
#include <pthread.h>
#include <atomic>
#include "maxflow/graph.h"
#include <regex>
#include <assert.h>

//#include "Word.h"
#include "Global.h"
#include "batches.h"

class Word //: public WordBackPointer
{
private:
    pthread_rwlock_t lock;
    int tlx, tly, brx, bry; // top y and bottom y
    //string query;
    string gt;
    //SearchMeta meta;
    const cv::Mat* pagePnt;
    const float* averageCharWidth;
    int* countCharWidth;
    int pageId;
    //int spottingIndex;
   
    int topBaseline, botBaseline;

    bool done;
    bool loose; //If the user reports and error during trans (not spotting error), we give it a second chance but loosen the regex to take in more possibilities
    unsigned long sentBatchId;


    void findBaselines(const cv::Mat& gray, const cv::Mat& bin);
    void getWordImgAndBin(cv::Mat& wordImg, cv::Mat& b);
    const cv::Mat getWordImg() const;
    cv::Point wordCord(int r, int c)
    {
        return cv::Point(c-tlx,r-tly);
    }

    string transcription;
    string transcribedBy;

    set<string> rejectedTrans;
    multimap<float,string> sentPoss;
    multimap<float,string> notSent;

    set<string> bannedUsers;
    unsigned int id;
protected:
    static atomic_uint _id;

public:
    //Word() : tlx(-1), tly(-1), brx(-1), bry(-1), pagePnt(NULL), averageCharWidth(NULL), countCharWidth(NULL), pageId(-1),  gt(""), done(false), sentBatchId(0), topBaseline(-1), botBaseline(-1)
    //{
    //    pthread_rwlock_init(&lock,NULL);
    //}    
    
    Word(int tlx, int tly, int brx, int bry, const cv::Mat* pagePnt, const float* averageCharWidth, int* countCharWidth, int pageId) : tlx(tlx), tly(tly), brx(brx), bry(bry), pagePnt(pagePnt), averageCharWidth(averageCharWidth), countCharWidth(countCharWidth), pageId(pageId), gt(""), done(false),  sentBatchId(0), topBaseline(-1), botBaseline(-1)
    {
        //meta = SearchMeta(THRESH_LEXICON_LOOKUP_COUNT);
        pthread_rwlock_init(&lock,NULL);
        assert(tlx>=0 && tly>=0 && brx<pagePnt->cols && bry<pagePnt->rows);
        id = _id++;
    }
    Word(int tlx, int tly, int brx, int bry, const cv::Mat* pagePnt, const float* averageCharWidth, int* countCharWidth, int pageId, string gt) : tlx(tlx), tly(tly), brx(brx), bry(bry), pagePnt(pagePnt), averageCharWidth(averageCharWidth), countCharWidth(countCharWidth), pageId(pageId),  gt(gt), done(false),  sentBatchId(0), topBaseline(-1), botBaseline(-1)
    {
        //meta = SearchMeta(THRESH_LEXICON_LOOKUP_COUNT);
        pthread_rwlock_init(&lock,NULL);
        assert(tlx>=0 && tly>=0 && brx<pagePnt->cols && bry<pagePnt->rows);
        id = _id++;
    }
    Word(ifstream& in, const cv::Mat* pagePnt, float* averageCharWidth, int* countCharWidth);
    void save(ofstream& out);
    
    ~Word()
    {
        pthread_rwlock_destroy(&lock);
    }

    vector<string> popTopXPossibilities(int x)
    {
        vector<string> toRet;
        auto iter=notSent.begin();
        for (int i=0; i<min(x,(int)(notSent.size())); i++)
        {
            toRet.push_back(iter->second);
            sentPoss.insert(*iter);
            iter = notSent.erase(iter);
        }
        return toRet;
    }
    
    
    void setTrans(string userId, string trans)
    {
        pthread_rwlock_wrlock(&lock);
        transcription = trans;
        transcribedBy = userId;
        done=true;
        pthread_rwlock_unlock(&lock);
    }
    bool userOK(string userId)
    {
        pthread_rwlock_unlock(&lock);
        bool ret = bannedUsers.find(userId) == bannedUsers.end();
        pthread_rwlock_rdlock(&lock);
        return ret;
    }
    void banUser(string userId)
    {
        pthread_rwlock_wrlock(&lock);
        bannedUsers.insert(userId);
        pthread_rwlock_unlock(&lock);
    }

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
    void getDoneAndGT(bool* isDone, string* gt)
    {
        pthread_rwlock_rdlock(&lock);
	*isDone=done;
        *gt=this->gt;
        pthread_rwlock_unlock(&lock);
    }
    bool isDone()
    {
        pthread_rwlock_rdlock(&lock);
        bool ret=done;
        pthread_rwlock_unlock(&lock);
        return ret;
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


    void setScores(const multimap<float,string>& scores)
    {
        pthread_rwlock_wrlock(&lock);
        notSent = scores;
        pthread_rwlock_unlock(&lock);
    }
    float topScore()
    {
        pthread_rwlock_rdlock(&lock);
        float ret = notSent.begin()->first;
        pthread_rwlock_unlock(&lock);
        return ret;
    }
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
    unsigned int getId(){return id;}

};

#endif
