#ifndef CORPUS_REF
#define CORPUS_REF

#include <map>
#include "opencv2/core/core.hpp"

using namespace std;
#include "WordBackPointer.h"
//class WordBackPointer;
#include "spotting.h"
//class Spotting;

//This object is purely to provide pointers when loading the systems state.
//It is created by the Corpus and then passed to the MasterQueue
class CorpusRef
{
    public:
    CorpusRef() {}
    void addWord(unsigned int i, WordBackPointer* bp, const cv::Mat* im, const multimap<int,Spotting>* sp)
    {
        backPointers[i]=bp;
        wordImgs[i]=im;
        spottings[i]=sp;
    }
    WordBackPointer* getBackPointer(unsigned int i) const
    {
        return backPointers.at(i);
    }
    const cv::Mat* getWordImg(unsigned int i) const
    {
        return wordImgs.at(i);
    }
    const multimap<int,Spotting>* getSpottingsPointer(unsigned int i) const
    {
        return spottings.at(i);
    }

    private:
    map<unsigned int, WordBackPointer*> backPointers;
    map<unsigned int,  const cv::Mat*> wordImgs;
    map<unsigned int, const multimap<int,Spotting>*> spottings;
};

#endif
