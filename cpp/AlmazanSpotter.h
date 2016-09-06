
#ifndef ALMAZANSPOTTER_H
#define ALMAZANSPOTTER_H

#include "opencv2/core/core.hpp"
#include "Spotter.h"
#include "embattspotter.h"
#include "dataset.h"
#include "AlmazanDataset.h"

#define ALPHA 1.0

class AlmazanSpotter : public Spotter
{
    private:
    EmbAttSpotter* spotter;
    AlmazanDataset* dataset;

    public:
    AlmazanSpotter(const Dataset* corpus, string modelDir);
    ~AlmazanSpotter()
    {
        delete spotter;
        delete dataset;
    }
    vector<SpottingResult> runQuery(SpottingQuery* query) const;
    float score(string text, const cv::Mat& image) const;
};

#endif
