#ifndef CORPUS_WORDS_H
#define CORPUS_WORDS_H

#include "dataset.h"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "Word.h"
#include <map>

using namespace std;

class CorpusDataset : public Dataset
{
    public:

    virtual const vector<string>& labels() const=0;
    virtual int size() const=0;
    virtual const cv::Mat image(unsigned int i) const=0;
    virtual unsigned int wordId(unsigned int i) const=0;
    virtual Word* word(unsigned int i)=0;
    virtual const Word* word(unsigned int i) const=0;

};
#endif
