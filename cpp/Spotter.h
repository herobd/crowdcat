#ifndef SPOTTER_H
#define SPOTTER_H

#include "SpottingQuery.h"
#include "opencv2/core/core.hpp"

struct SpottingResult {
    int imIdx;
    float score;
    int startX;
    int endX;
    SpottingResult(int imIdx, float score, int startX, int endX) : 
        imIdx(imIdx), score(score), startX(startX), endX(endX)
    {
    }
    SpottingResult() : 
        imIdx(-1), score(0), startX(-1), endX(-1)
    {
    }
};

class Spotter
{
    public:
    virtual vector<SpottingResult> runQuery(SpottingQuery* query) const =0;
    virtual float score(string text, const cv::Mat& image) const =0;
};
#endif
