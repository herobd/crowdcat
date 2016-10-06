#ifndef PAGE_REF
#define PAGE_REF

#include <map>
#include "opencv2/core/core.hpp"

using namespace std;

//This object is purely to provide pointers when loading the systems state.
//It is created by the Page and then passed to the MasterQueue
class PageRef
{
    public:
    PageRef() {}
    void addPage(int pageId, cv::Mat* im)
    {
        pages[pageId]=im;
    }
    const cv::Mat* getPageImg(int pageId) const
    {
        return pages.at(pageId);
    }

    private:
    map<int, cv::Mat*> pages;
};

#endif
