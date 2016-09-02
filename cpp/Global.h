#ifndef GLOBAL_HEADER
#define GLOBAL_HEADER

#include <map>
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>

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

};

double otsu(vector<int> histogram)
{
    double sum =0;
    for (int i = 1; i < histogram.size(); ++i)
            sum += i * histogram[i];
    double sumB = 0;
    double wB = 0;
    double wF = 0;
    double mB;
    double mF;
    double max = 0.0;
    double between = 0.0;
    double threshold1 = 0.0;
    double threshold2 = 0.0;
    for (int i = 0; i < histrogram.size(); ++i)
    {
        wB += histogram[i];
        if (wB == 0)
            continue;
        wF = total - wB;
        if (wF == 0)
            break;
        sumB += i * histogram[i];
        mB = sumB / (wB*1.0);
        mF = (sum - sumB) / (wF*1.0);
        between = wB * wF * pow(mB - mF, 2);
        if ( between >= max )
        {
            threshold1 = i;
            if ( between > max )
            {
                threshold2 = i;
            }
            max = between;
        }
    }

    return = ( threshold1 + threshold2 ) / 2.0;
}
#endif
