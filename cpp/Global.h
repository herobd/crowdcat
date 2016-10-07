#ifndef GLOBAL_HEADER
#define GLOBAL_HEADER

#include <map>
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace std;

#define MIN_N 2
#define MAX_N 2
#define MAX_NGRAM_RANK 300
class GlobalK
{
    private:
        GlobalK();
        static GlobalK* _self;

        map<int, vector<string> > ngramRanks;
    public:
        static GlobalK* knowledge();

        int getNgramRank(string ngram);

        static double otsuThresh(vector<int> histogram);
        static void saveImage(const cv::Mat& im, ofstream& out);
        static void loadImage(cv::Mat& im, ifstream& in);
};

#endif
