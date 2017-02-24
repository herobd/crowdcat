#ifndef ALMAZAN_DATASET_H
#define ALMAZAN_DATASET_H

#include "dataset.h"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <map>

using namespace std;

class AlmazanDataset : public Dataset
{
    public:
    AlmazanDataset(const Dataset* other);


    const vector<string>& labels() const;
    int size() const;
    const cv::Mat image(unsigned int i) const;
    int backwards(int i, int x) const;
    unsigned int wordId(unsigned int i) const {return ids.at(i);}

    private:
    vector<string> _labels;
    vector<unsigned int> ids;
    vector<cv::Mat> wordImages;
    map<int,double> ratios; //for mapping back to original image space after preprocess
    cv::Mat preprocess(cv::Mat im, int i=-1);
};
#endif
