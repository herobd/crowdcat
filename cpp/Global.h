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
        mutex mutLock;
        atomic_int transSent;
        atomic_int spotSent;
        atomic_int spotAccept;
        atomic_int spotReject;
        atomic_int spotAutoAccept;
        atomic_int spotAutoReject;
        atomic_int newExemplarSpotted;
        ofstream trackFile;

    public:
        static GlobalK* knowledge();

        int getNgramRank(string ngram);
        //int getContextPad() {return contextPad;}
        //void setContextPad(int pad) {contextPad=pad;}

        static double otsuThresh(vector<int> histogram);
        static void saveImage(const cv::Mat& im, ofstream& out);
        static void loadImage(cv::Mat& im, ifstream& in);

        void sentSpottings();
        void sentTrans();
        void accepted();
        void rejected();
        void autoAccepted();
        void autoRejected();
        void newExemplar();
        void saveTrack(float accTrans, float pWordsTrans, float pWords80_100, float pWords60_80, float pWords40_60, float pWords20_40, float pWords0_20, float pWords0);
};

#endif
