#ifndef GLOBAL_HEADER
#define GLOBAL_HEADER

#include <map>
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <atomic>
#include <mutex>
#include <time.h>
#include <sys/stat.h>
#include <sstream>

#ifdef NO_NAN
#include "SubwordSpottingResult.h"
#endif

using namespace std;

#define MIN_N 2
#define MAX_N 2
#define MAX_NGRAM_RANK 300
class GlobalK
{
    private:
        GlobalK();
        ~GlobalK();
        static GlobalK* _self;

        map<int, vector<string> > ngramRanks;
        //int contextPad;
#ifdef NO_NAN
        atomic_int transSent;
        atomic_int spotSent;
        atomic_int spotAccept;
        atomic_int spotReject;
        atomic_int spotAutoAccept;
        atomic_int spotAutoReject;
        atomic_int newExemplarSpotted;
        ofstream trackFile;

        stringstream track;

        mutex spotMut;
        mutex accumResMut;
        string spottingFile;
        map<string, vector<float> > spottingAccums;
        map<string, vector<float> > spottingExemplars;
        map<string, vector<float> > spottingNormals;
        map<string, vector<float> > spottingOthers;
        
        map<string, vector<SubwordSpottingResult>*> accumRes;

        mutex xLock;
        const vector< vector<int> >* corpusXLetterStartBounds;
        const vector< vector<int> >* corpusXLetterEndBounds;
#endif

    public:
        static GlobalK* knowledge();

        int getNgramRank(string ngram);
        //int getContextPad() {return contextPad;}
        //void setContextPad(int pad) {contextPad=pad;}

        static double otsuThresh(vector<int> histogram);
        static void saveImage(const cv::Mat& im, ofstream& out);
        static void loadImage(cv::Mat& im, ifstream& in);
        static string currentDateTime();

#ifdef NO_NAN
        void setSimSave(string file);
        void sentSpottings();
        void sentTrans();
        void accepted();
        void rejected();
        void autoAccepted();
        void autoRejected();
        void newExemplar();
        void saveTrack(float accTrans, float pWordsTrans, float pWords80_100, float pWords60_80, float pWords40_60, float pWords20_40, float pWords0_20, float pWords0);
        void writeTrack();       

        void setCorpusXLetterBounds(const vector< vector<int> >* start, const vector< vector<int> >* end)
        {
            corpusXLetterStartBounds=start;
            corpusXLetterEndBounds=end;
            xLock.unlock();
        }
        const vector< vector<int> >* getCorpusXLetterStartBounds() {xLock.lock(); xLock.unlock(); return corpusXLetterStartBounds;}
        const vector< vector<int> >* getCorpusXLetterEndBounds() {xLock.lock(); xLock.unlock(); return corpusXLetterEndBounds;}
        void storeSpottingAccum(string ngram, float ap);
        void storeSpottingExemplar(string ngram, float ap);
        void storeSpottingNormal(string ngram, float ap);
        void storeSpottingOther(string ngram, float ap);
        vector<SubwordSpottingResult>* accumResFor(string ngram);
#endif
};

#endif
