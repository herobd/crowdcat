
#ifndef SPOTTING_H
#define SPOTTING_H

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <vector>
#include <set>
#include <map>
#include <chrono>
#include <atomic>
#include <iostream>

using namespace std;

#define FUZZY 12

#define SPOTTING_TYPE_NONE 1
#define SPOTTING_TYPE_APPROVED 2
#define SPOTTING_TYPE_THRESHED 3
#define SPOTTING_TYPE_EXEMPLAR 4

class Spotting {
public:
    Spotting() :
        tlx(-1), tly(-1), brx(-1), bry(-1), pageId(-1), pagePnt(NULL), ngram(""), score(nan("")), id(-1), type(SPOTTING_TYPE_NONE), ngramRank(-1) {}
    Spotting(int tlx, int tly, int brx, int bry) :
        tlx(tlx), tly(tly), brx(brx), bry(bry), pageId(-1), pagePnt(NULL), ngram(""), score(nan("")), id(-1), type(SPOTTING_TYPE_NONE), ngramRank(-1) {}
    Spotting(int tlx, int tly) :
        tlx(tlx), tly(tly), brx(-1), bry(-1), pageId(-1), pagePnt(NULL), ngram(""), score(nan("")), id(-1), type(SPOTTING_TYPE_NONE), ngramRank(-1) {}
    
    Spotting(int tlx, int tly, int brx, int bry, int pageId, const cv::Mat* pagePnt, string ngram, float score) : 
        tlx(tlx), tly(tly), brx(brx), bry(bry), pageId(pageId), pagePnt(pagePnt), ngram(ngram), score(score), type(SPOTTING_TYPE_NONE), ngramRank(-1)
    {
        id = ++_id;
    }
    
    Spotting(const Spotting& s) : 
        tlx(s.tlx), tly(s.tly), brx(s.brx), bry(s.bry), pageId(s.pageId), pagePnt(s.pagePnt), ngram(s.ngram), score(s.score), type(s.type), ngramRank(s.ngramRank)
    {
        id = s.id;
    }
    Spotting& operator=(const Spotting& other)
    {
        //swap(other);
        tlx = other.tlx;
        tly = other.tly;
        brx = other.brx;
        bry = other.bry;
        pageId = other.pageId;
        pagePnt = other.pagePnt;
        ngram = other.ngram;
        score = other.score;
        type = other.type;
        ngramRank = other.ngramRank;

        return *this;
    }
    virtual ~Spotting() {}
    
    int tlx, tly, brx, bry, pageId;
    const cv::Mat* pagePnt;
    string ngram;
    float score;
    unsigned long id;
    unsigned char type;
    virtual cv::Mat img()
    {
        return (*pagePnt)(cv::Rect(tlx,tly,1+brx-tlx,1+bry-tly));
    }
    virtual cv::Mat ngramImg() const
    {
        return (*pagePnt)(cv::Rect(tlx,tly,1+brx-tlx,1+bry-tly));
    }
    int ngramRank;

protected:
    static atomic_ulong _id;
};

class SpottingImage : public Spotting 
{
public:
    SpottingImage(const Spotting& s, int maxWidth, int color, string prevNgram="") : 
        Spotting(s) 
    {
        ngramImage = s.ngramImg();
        assert(ngramImage.cols>0);
        if (maxWidth<0)
            return;
        int oneSide = maxWidth/2;
        int sideFromR = (oneSide- (brx-tlx)/2);
        int left = tlx-sideFromR;
        int right = brx+sideFromR-1;
        //cout <<"getting image window... sideFromR="<<sideFromR<<", oneSide="<<oneSide<<", tlx="<<tlx<<", brx="<<brx<<", left="<<left<<", right="<<right<<endl;
        if (left>=0 && right<s.pagePnt->cols)
        {   
            //cout <<"normal: "<<left<<" "<<tly<<" "<<right-left<<" "<<bry-tly<<endl;
            image = ((*s.pagePnt)(cv::Rect(left,tly,right-left,bry-tly))).clone();
        }
        else
        {
            image = cv::Mat(bry-tly,maxWidth,s.pagePnt->type());
            if (image.channels()==1)
                image.setTo(cv::Scalar(10));
            else
                image.setTo(cv::Scalar(10,10,10));
            int leftOff = left>=0?0 : -1*(left+1);
            int newLeft=left<0?0:left;
            if (right>=s.pagePnt->cols)
                right = s.pagePnt->cols-1;
            //cout <<"adjusted from: "<<newLeft<<" "<<tly<<" "<<right-newLeft<<" "<<bry-tly<<endl;
            //cout <<"adjusted to: "<<leftOff<<" "<<0<<" "<<right-newLeft<<" "<<bry-tly<<endl;
            (*s.pagePnt)(cv::Rect(newLeft,tly,right-newLeft,bry-tly)).copyTo(image(cv::Rect(leftOff,0,right-newLeft,bry-tly)));
        }
        //cout <<"done, now coloring..."<<endl;
        if (image.channels()==1)
            cv::cvtColor(image, image, CV_GRAY2RGB);
        
        //if (left==1117 && tly==186)
        //{
         //   cv::imshow("rer", image);
          //  cv::waitKey();
        //}
        if (prevNgram.compare(ngram)!=0)
        {
            color=(color+1)%5;
        }
        for (int r=0; r<bry-tly; r++)
            for (int c=sideFromR-FUZZY; c<sideFromR+brx-tlx+FUZZY; c++)
            {
                //if (left==1117 && tly==186 && (r==10 || r==5) && c==300)
                //{
                  //  cv::imshow("rer", image);
                  //  cv::waitKey(1);
                //}
                //cout <<" ("<<r<<" "<<c;
                //cout.flush();
                if (c>=0 && c<image.cols)
                {
                    cv::Vec3b& pix = image.at<cv::Vec3b>(r,c);
                    //cout <<"):";
                    //cout.flush();
                    //cout<<(int)pix[0];
                    //cout.flush();
                    //image.at<cv::Vec3b>(r,c) = cv::Vec3b(pix[0]*0.75,min(20+(int)(pix[1]*1.05),255),pix[2]*0.75);
                    float fuzzyMult = 1;
                    if (c<sideFromR+FUZZY)
                        fuzzyMult=(c-sideFromR+FUZZY)/(2.0*FUZZY);
                    else if (c>sideFromR+brx-tlx-FUZZY)
                        fuzzyMult=(sideFromR+brx-tlx+FUZZY-c)/(2.0*FUZZY);
                    
                    if (color%3!=1)//red is a bad color for highlighting
                    {
                        pix[color%3]*=1.0-0.25*fuzzyMult;
                        pix[(color+1)%3] =min((int)(20*fuzzyMult+(pix[(color+1)%3]*(1.0+fuzzyMult*0.05))),255);
                        pix[(color+2)%3]*=1.0-0.25*fuzzyMult;
                    }
                    else
                    {
                        pix[color%3]=min((int)(20*fuzzyMult+(pix[(color)%3]*(1.0+fuzzyMult*0.05))),255);
                        pix[(color+1)%3] =min((int)(20*fuzzyMult+(pix[(color+1)%3]*(1.0+fuzzyMult*0.05))),255);
                        pix[(color+2)%3]*=1.0-0.25*fuzzyMult;
                    }
                }
            }
        //cout<<endl;
        //cout <<"done"<<endl;
    }
    
    SpottingImage(const SpottingImage& s) : Spotting(s)
    {
        image=s.image;
        ngramImage=s.ngramImage;
    }
    virtual ~SpottingImage() {}
    
    virtual cv::Mat img()
    {
        return image;
    }
    cv::Mat ngramImg() const
    {
        assert(ngramImage.cols>0);
        return ngramImage; //(*pagePnt)(cv::Rect(tlx,tly,brx-tlx,bry-tly));
    }
    int classified;
private:
    cv::Mat image;
    cv::Mat ngramImage;
};

class SpottingExemplar : public Spotting
{
public:
    SpottingExemplar(int tlx, int tly, int brx, int bry, int pageId, const cv::Mat* pagePnt, string ngram, float score, cv::Mat ngramImage) : Spotting(tlx,tly,brx,bry,pageId,pagePnt,ngram,score) , ngramImage(ngramImage)
    {
        id = _id++;
        type=SPOTTING_TYPE_EXEMPLAR;
    }
    SpottingExemplar(const SpottingImage& s) : Spotting(s) , ngramImage(s.ngramImg())
    {
        type=SPOTTING_TYPE_EXEMPLAR;
    }
    SpottingExemplar(const SpottingExemplar& s) : Spotting(s) , ngramImage(s.ngramImg())
    {
        type=SPOTTING_TYPE_EXEMPLAR;
    }
    virtual ~SpottingExemplar() {}

    cv::Mat ngramImg() const
    {
        return ngramImage; 
    }
private:
    cv::Mat ngramImage;
};

#endif
