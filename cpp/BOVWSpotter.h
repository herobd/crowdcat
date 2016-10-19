
#ifndef ALMAZANSPOTTER_H
#define ALMAZANSPOTTER_H

#include "opencv2/core/core.hpp"
#include "Spotter.h"
#include "embattspotter.h"
#include "dataset.h"


class BOVWSpotter : public Spotter
{
    private:
    EnhancedBoVW* spotter;
    const Dataset* dataset;

    public:
    BOVWSpotter(const Dataset* corpus, string modelDir);
    ~BOVWSpotter()
    {
        delete spotter;
        //delete dataset;
    }
    vector<SpottingResult> runQuery(SpottingQuery* query) const;
    float score(string text, const cv::Mat& image) const;
    float score(string text, int wordIndex) const;
};

#endif
