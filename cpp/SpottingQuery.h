#ifndef SPOTTING_QUERY_H
#define SPOTTING_QUERY_H

#include "opencv2/core/core.hpp"
#include "spotting.h"


using namespace std;

class SpottingQuery 
{
    public:
    SpottingQuery(const Spotting* e) : id(e->id), ngram(e->ngram), img(e->ngramImg()), type(e->type) {}
    SpottingQuery(const Spotting& e) : SpottingQuery(&e) {}
    SpottingQuery(string ngram) : id(-1), ngram(ngram) {}
    string getNgram() const {return ngram;}
    unsigned long getId() const {return id;}
    cv::Mat getImg() {return img;}
    SpottingType getType() const {return type;}

    private:
    string ngram;
    unsigned long id;
    cv::Mat img;
    SpottingType type;
};

#endif
