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
    SpottingQuery(string ngram) : id(-1), ngram(ngram), type(SPOTTING_TYPE_NONE) {}
    string getNgram() const {return ngram;}
    unsigned long getId() const {return id;}
    cv::Mat getImg() {return img;}
    SpottingType getType() const {return type;}

    SpottingQuery(ifstream& in)
    {
        string line;
        getline(in,ngram);
        getline(in,line);
        id = stoul(line);
        GlobalK::loadImage(img,in);
        getline(in,line);
        type = static_cast<SpottingType>(stoi(line));
    }
    void save(ofstream& out)
    {
        out<<ngram<<"\n";
        out<<id<<"\n";
        GlobalK::saveImage(img,out);
        out<<type<<"\n";
    }

    private:
    string ngram;
    unsigned long id;
    cv::Mat img;
    SpottingType type;
};

#endif
