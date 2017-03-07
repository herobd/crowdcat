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

    virtual const vector<string>& labels() const;
    virtual int size() const;
    virtual const cv::Mat image(unsigned int i) const;
    virtual unsigned int wordId(unsigned int i) const;
    virtual Word* word(unsigned int i);
    virtual const Word* word(unsigned int i) const;

};
#endif
