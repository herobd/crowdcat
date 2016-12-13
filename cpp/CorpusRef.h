#ifndef CORPUS_REF
#define CORPUS_REF

#include <map>
#include "opencv2/core/core.hpp"

using namespace std;
#include "WordBackPointer.h"
//class WordBackPointer;
#include "spotting.h"
#include "Location.h"
//class Spotting;

//This object is purely to provide pointers when loading the systems state.
//It is created by the Corpus and then passed to the MasterQueue

//It also contains all word locations for debuggin purposes
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
    void addLoc(const Location& loc)
    {
        locs[loc.pageId].push_back(loc);
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

    bool verify(int pageId, int tlx, int tly, int brx, int bry) const
    {
        //check against words
        bool found=false;
        for (const Location& loc : locs.at(pageId))
        {
            if (loc.x1<=tlx && loc.y1<=tly &&
                    loc.x2>=brx && loc.y2>=bry)
            {
                if (pageId==0 && tlx==1574 && tly==2968 && brx==1664 && bry==3126)
                    cout<<"[!] Matching word is at, page:"<<pageId<<" x1:"<<loc.x1<<" y1:"<<loc.y1<<" x2:"<<loc.x2<<" y2:"<<loc.y2<<endl;
                found=true;
                break;
            }
        }
        return found;
    }

    private:
    map<unsigned int, WordBackPointer*> backPointers;
    map<unsigned int,  const cv::Mat*> wordImgs;
    map<unsigned int, const multimap<int,Spotting>*> spottings;
    map<int,vector<Location> > locs;
};

#endif
