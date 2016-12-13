#ifndef PAGE_REF
#define PAGE_REF

#include <map>
#include "opencv2/core/core.hpp"
#include "Location.h"

using namespace std;

//This object is purely to provide pointers when loading the systems state.
//It is created by the Page and then passed to the MasterQueue

//For debugging, it holds all word bounding boxes and can verify if a spotting object is valid
class PageRef
{
    public:
    PageRef() {}
    void addPage(int pageId, const cv::Mat* im)
    {
        pages[pageId]=im;
    }
    void addWord(const Location& loc)
    {
        words[loc.pageId].push_back(loc);
    }
    const cv::Mat* getPageImg(int pageId) const
    {
        return pages.at(pageId);
    }

    bool verify(int pageId, int tlx, int tly, int brx, int bry) const
    {
        //check against words
        bool found=false;
        for (const Location& loc : words.at(pageId))
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
    map<int, const cv::Mat*> pages;
    map<int, vector<Location> > words;
};

#endif
