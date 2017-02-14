#ifndef CATTSS_H
#define CATTSS_H

#include "MasterQueue.h"
#include "Knowledge.h"
#include "SpottingQueue.h"
#include "spotting.h"
#include "Lexicon.h"
#include "BatchWraper.h"
#include "opencv2/core/core.hpp"
#include <exception>
#include <pthread.h>

#include <unistd.h>
//#include "ctpl_stl.h"
#ifdef NO_NAN
#include "tester.h"
#endif

using namespace std;

#define CHECK_SAVE_TIME 6

#define NEW_EXEMPLAR_TASK 1
#define TRANSCRIPTION_TASK 2
#define SPOTTINGS_TASK 3
struct UpdateTask
{
    int type;
    string id;
    vector<int> labels;
    int resent_manual_bool;
    vector<string> strings;
    UpdateTask(string batchId,  vector<int>& labels, int resent) : type(NEW_EXEMPLAR_TASK), id(batchId), labels(labels), resent_manual_bool(resent) {} 
    UpdateTask(string id, string transcription, bool manual) : type(TRANSCRIPTION_TASK), id(id),  resent_manual_bool(manual) {strings.push_back(transcription);}
    UpdateTask(string resultsId, vector<string>& ids, vector<int>& labels, int resent) : type(SPOTTINGS_TASK), id(resultsId), labels(labels), resent_manual_bool(resent), strings(ids) {} 

    UpdateTask(ifstream& in)
    {
        string line;
        getline(in,line);
        type = stoi(line);
        getline(in,id);
        getline(in,line);
        int size = stoi(line);
        labels.resize(size);
        for (int i=0; i<size; i++)
        {
            getline(in,line);
            labels.at(i)=stoi(line);
        }
        getline(in,line);
        resent_manual_bool = stoi(line);
        getline(in,line);
        size = stoi(line);
        strings.resize(size);
        for (int i=0; i<size; i++)
        {
            getline(in,strings.at(i));
        }
    }
    void save(ofstream& out)
    {
        out<<type<<"\n";
        out<<id<<"\n";
        out<<labels.size()<<"\n";
        for (int i : labels)
        {
            out<<i<<"\n";
        }
        out<<resent_manual_bool<<"\n";
        out<<strings.size()<<"\n";
        for (string s : strings)
        {
            out<<s<<"\n";
        }
    }
};

class CATTSS
{
    private:
    MasterQueue* masterQueue;
    SpottingQueue* spottingQueue;
    Knowledge::Corpus* corpus;
    thread* incompleteChecker;
    thread* showChecker;

    //ctpl::thread_pool* pool;
    //Thread pool stuff
    vector<thread*> taskThreads;
    atomic_char cont;
    deque<UpdateTask*> taskQueue;
    sem_t semLock;
    mutex taskQueueLock;

    string savePrefix;//file prefix for its regular saving.

    UpdateTask* dequeue();
    void enqueue(UpdateTask* task);
    void run(int numThreads);
    void stop();

    public:
    CATTSS( string lexiconFile,
            string pageImageDir, 
            string segmentationFile, 
            string spottingModelPrefix,
            string savePrefix,
            int numSpottingThreads,
            int numTaskThreads,
            int showHeight,     //Height of showProgress image
            int showWidth,      //Width of showProgress image
            int showMilli,      //How frequently to save showProgress
            int contextPad );    //how many pixels to arbitrarly pad to the bottom of images sent to users (for NAMES)
    ~CATTSS()
    {
        delete incompleteChecker;
        delete showChecker;
        delete masterQueue;
        delete corpus;
        delete spottingQueue;
        for (thread* t : taskThreads)
            delete t;
        for (UpdateTask* t : taskQueue)
            delete t;
    } 
    void save();

    BatchWraper* getBatch(int num, int width, int color, string prevNgram);
    void updateSpottings(string resultsId, vector<string> ids, vector<int> labels, int resent);
    void updateTranscription(string id, string transcription, bool manual);
    void updateNewExemplars(string resultsId,  vector<int> labels, int resent);
    void misc(string task);
    const Knowledge::Corpus* getCorpus() const {return corpus;}

    const cv::Mat* imgForPageId(int id) const {return corpus->imgForPageId(id);}
    void threadLoop();
    bool getCont() {return cont.load();}
#ifdef NO_NAN
    friend class Tester;
#endif

    //For data collection, when I deleted all my trans... :(
    void resetAllWords_();
};
#endif
