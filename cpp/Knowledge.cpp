#include "Knowledge.h"
#include <time.h>

int Knowledge::Page::_id=0;

Knowledge::Corpus::Corpus(int contextPad)//, int averageCharWidth) 
{
    pthread_rwlock_init(&pagesLock,NULL);
    //pthread_rwlock_init(&spottingsMapLock,NULL);
    averageCharWidth=-1;//averageCharWidth;
    countCharWidth=0;
    threshScoring= 1.0;
    //manQueue.setContextPad(contextPad);
}
//void Knowledge::Corpus::loadSpotter(string modelPrefix)
//{
    //spotter = new AlmazanSpotter(this,modelPrefix);

    //This is bad, it shouldn't be coming from here, but it prevents code dup.
    //averageCharWidth = spotter->getAverageCharWidth();

//}
/*vector<TranscribeBatch*> Knowledge::Corpus::addSpotting(Spotting s,vector<Spotting*>* newExemplars)
{
    vector<TranscribeBatch*> ret;
    pthread_rwlock_rdlock(&pagesLock);
    Page* page = pages[s.pageId];
    pthread_rwlock_unlock(&pagesLock);
    if (page==NULL)
    {
        assert(false && "ERROR, page not present");
    }
    
    addSpottingToPage(s,page,ret,newExemplars);
    
    return ret;
}
vector<TranscribeBatch*> Knowledge::Corpus::updateSpottings(vector<Spotting>* spottings, vector<pair<unsigned long,string> >* removeSpottings, vector<unsigned long>* toRemoveBatches, vector<Spotting*>* newExemplars, vector<pair<unsigned long, string> >* toRemoveExemplars)
{
    //cout <<"addSpottings"<<endl;
    vector<TranscribeBatch*> ret;
    vector<Page*> thesePages;
    pthread_rwlock_rdlock(&pagesLock);
    //cout <<"addSpottings: got lock"<<endl;
    bool writing=false;
    for (const Spotting& s : *spottings)
    {
        Page* page;
        if (pages.find(s.pageId)==pages.end())
        {

            assert(false && "ERROR, page not present");
        }
        else
        {
            page = pages[s.pageId];
        }
        thesePages.push_back(page);
    }
    pthread_rwlock_unlock(&pagesLock);
    //cout <<"addSpottings: release lock"<<endl;
    
    for (int i=0; i<spottings->size(); i++)
        addSpottingToPage(spottings->at(i),thesePages[i],ret,newExemplars);

    //Removing spottings
    map<unsigned long, vector<Word*> > wordsForIds;
    if (removeSpottings)
    {
        pthread_rwlock_wrlock(&spottingsMapLock);
        for (auto sid : *removeSpottings)
        {
            wordsForIds[sid.first] = spottingsToWords[sid.first];
            spottingsToWords[sid.first].clear(); 
        }
        pthread_rwlock_unlock(&spottingsMapLock);
        for (auto sid : *removeSpottings)
        {
            vector<Word*> wordsForId = wordsForIds[sid.first];
            for (Word* word : wordsForId)
            {
                
                unsigned long retractId=0;  
                TranscribeBatch* newBatch = word->removeSpotting(sid.first,0,false,&retractId,newExemplars,toRemoveExemplars);
                if (retractId!=0 && newBatch==NULL)
                {
                    //retract the batch
                    if (toRemoveBatches)
                        toRemoveBatches->push_back(retractId);
                }
                else if (newBatch != NULL)
                {
                    //modify batch
                    ret.push_back(newBatch);
                }
            }
        }
    }
    //cout <<"END addSpottings"<<endl;
    return ret;
}


void Knowledge::Corpus::addSpottingToPage(Spotting& s, Page* page, vector<TranscribeBatch*>& ret, vector<Spotting*>* newExemplars)
{
    vector<Line*> possibleLines;
    //pthread_rwlock_rdlock(&page->linesLock);
    vector<Line*> lines = page->lines();
    bool oneLine=false;
    for (Line* line : lines)
    {
        int line_ty, line_by;
        vector<Word*> wordsForLine = line->wordsAndBounds(&line_ty,&line_by);
        int overlap = min(s.bry,line_by) - max(s.tly,line_ty);
        float overlapPortion = overlap/(0.0+s.bry-s.tly);
        if (overlapPortion > OVERLAP_LINE_THRESH)
        {
            oneLine=true;
            //pthread_rwlock_rdlock(&line->wordsLock);
            bool oneWord=false;
            for (Word* word : wordsForLine)
            {
                int word_tlx, word_tly, word_brx, word_bry;
                bool isDone;
                word->getBoundsAndDone(&word_tlx,&word_tly,&word_brx,&word_bry,&isDone);
                if (!isDone)
                {
                    int overlap = (max(0,min(s.bry,word_bry) - max(s.tly,word_tly))) * (max(0,min(s.brx,word_brx) - max(s.tlx,word_tlx)));
                    float overlapPortion = overlap/(0.0+(s.bry-s.tly)*(s.brx-s.tlx));
                    //cout <<"Overlap for word "<<word<<": overlapPortion="<<overlapPortion<<" ="<<overlap<<"/"<<(0.0+(s.bry-s.tly)*(s.brx-s.tlx))<<endl;
                    TranscribeBatch* newBatch=NULL;
                    if (overlapPortion > OVERLAP_WORD_THRESH)
                    {
                        oneWord=true;
                        //possibleWords.push_back(word);
                        newBatch = word->addSpotting(s,newExemplars);
                        pthread_rwlock_wrlock(&spottingsMapLock);
                        spottingsToWords[s.id].push_back(word);
                        pthread_rwlock_unlock(&spottingsMapLock);
                    }
                    
                    if (newBatch != NULL)
                    {
                        ret.push_back(newBatch);
                    }
                }
            }
            
            if (!oneWord)
            {
                //for now, do nothing as we know where all words are
                //TODO
                //make a new word?
                //assert(false);
                //line->addWord(s);
            }
            
            //pthread_rwlock_unlock(&line->wordsLock);
            
        }
    }
    if (!oneLine)
    {
        //Addline
        //I'm assumming most lines are added prior with a preprocessing step
        page->addLine(s,newExemplars);
        recreateDatasetVectors(true);
    }
}







int Knowledge::getBreakPoint(int lxBound, int ty, int rxBound, int by, const cv::Mat* pagePnt)
{
    int retX;
    cv::Mat b;// = GlobalK::otsuThresh(*t.pagePnt);
    int blockSize = max((rxBound-lxBound)/2,(by-ty)/2);
    if (blockSize%2==0)
        blockSize++;
    cv::Mat orig = (*pagePnt)(cv::Rect(lxBound,ty,rxBound-lxBound+1,by-ty+1));
    if (orig.type()==CV_8UC3)
        cv::cvtColor(orig,orig,CV_RGB2GRAY);
    cv::adaptiveThreshold(orig, b, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY_INV, blockSize, 10);
    
    cv::Mat hist(b.cols,1,CV_32F);
    cv::Mat tb(b.cols,1,CV_32F);
    //vector<int> top(b.cols,9999);
    //vector<int> bot(b.cols,-9999);
    for (int c=0; c<b.cols; c++)
    {
        int top=9999;
        int bot=-9999;
        bool hitTop=false;
        for (int r=0; r<b.rows; r++)
        {
            assert(c<hist.rows && r<orig.rows && c<orig.cols);
            hist.at<float>(c,0)+=orig.at<unsigned char>(r,c);
            if (b.at<unsigned char>(r,c))
            {
                if (!hitTop)
                {
                    top=r;
                    hitTop=true;
                }
                bot=r;
            }
        }
        
        tb.at<float>(c,0)=bot-top;
    }
    float kernelData[5] = {.1,.5,.7,.5,.1};
    cv::Mat kernel = cv::Mat(5,1,CV_32F,kernelData);
    cv::filter2D(hist, hist, -1 , kernel );
    cv::filter2D(tb, tb, -1 , kernel );
    float minHist=99999;
    float maxHist=-99999;
    float minTb=99999;
    float maxTb=-99999;
    for (int i=2; i<b.cols-2; i++)
    {
        float vHist = hist.at<float>(i,0);
        float vTb = tb.at<float>(i,0);
        if (vHist>maxHist)
            maxHist=vHist;
        if (vHist<minHist)
            minHist=vHist;
        if (vTb>maxTb)
            maxTb=vTb;
        if (vTb<minTb)
            minTb=vTb;
    }
    hist = (hist-minHist)/maxHist;
    tb = (tb-minTb)/maxTb;
    
    float min=99999;
    for (int i=2; i<b.cols-2; i++)
    {
        float v = hist.at<float>(i,0) + tb.at<float>(i,0);
        if (v<min)
        {
            min=v;
            retX=i;
        }
    }

#ifdef TEST_MODE
#if SHOW
    orig=orig.clone();
    for (int r=0; r<orig.rows; r++)
        orig.at<unsigned char>(r,retX)=0;
    cv::imshow("break point",orig);
    cv::waitKey();
#endif
#endif

    return retX+lxBound;
}
cv::Mat Knowledge::inpainting(const cv::Mat& src, const cv::Mat& mask, double* avg, double* std, bool show)
{
    assert(src.rows == mask.rows && src.cols==mask.cols);
    int x_start[4] = {0,0,mask.cols-1,mask.cols-1};
    int x_end[4] = {mask.cols,mask.cols,-1,-1};
    int y_start[4] = {0,mask.rows-1,0,mask.rows-1};
    int y_end[4] = {mask.rows,-1,mask.rows,-1};
    cv::Mat dst = src.clone();
    cv::Mat P[4];
    cv::Mat I = src.clone();
    for (int i=0; i<4; i++)
    {
        P[i] = (cv::Mat_<unsigned char>(mask.rows,mask.cols));
        cv::Mat M = mask.clone();
        int yStep = y_end[i]>y_start[i]?1:-1;
        int xStep = x_end[i]>x_start[i]?1:-1;
        for (int y=y_start[i]; y!=y_end[i]; y+=yStep)
            for (int x=x_start[i]; x!=x_end[i]; x+=xStep)
            {
                
                if (M.at<unsigned char>(y,x) == 0)
                {
                    
                    int denom = (x-1>=0?M.at<unsigned char>(y,x-1):0) + 
                                (y-1>=0?M.at<unsigned char>(y-1,x):0) +
                                (x+1<I.cols?M.at<unsigned char>(y,x+1):0) + 
                                (y+1<I.rows?M.at<unsigned char>(y+1,x):0);
                    if (denom !=0)
                    {
                        P[i].at<unsigned char>(y,x) = (
                                                        (x-1>=0?I.at<unsigned char>(y,x-1)*M.at<unsigned char>(y,x-1):0) +
                                                        (y-1>=0?I.at<unsigned char>(y-1,x)*M.at<unsigned char>(y-1,x):0) + 
                                                        (x+1<I.cols?I.at<unsigned char>(y,x+1)*M.at<unsigned char>(y,x+1):0) +
                                                        (y+1<I.rows?I.at<unsigned char>(y+1,x)*M.at<unsigned char>(y+1,x):0)
                                                      )/denom;
                        //if (P[i].at<unsigned char>(y,x)!=0)
                        //{
                            if (avg!=NULL && std!=NULL)
                            {
                                if (P[i].at<unsigned char>(y,x)>*avg)
                                {
                                    double dif = std::min(P[i].at<unsigned char>(y,x)-(*avg), 3*(*std));
                                    P[i].at<unsigned char>(y,x) -= 8.5*dif/(*std);
                                }
                                else if (P[i].at<unsigned char>(y,x)<*avg)
                                {
                                    double dif = std::min((*avg)-P[i].at<unsigned char>(y,x), 3*(*std));
                                    P[i].at<unsigned char>(y,x) += 8.5*dif/(*std);
                                }
                            }
                            I.at<unsigned char>(y,x) = P[i].at<unsigned char>(y,x);
                            M.at<unsigned char>(y,x) = 1;
                        //}
                        //else
                        //    P[i].at<unsigned char>(y,x) = 255;
                    }
                    else
                        P[i].at<unsigned char>(y,x) = 255;
                }
                else
                    P[i].at<unsigned char>(y,x) = I.at<unsigned char>(y,x);
            }
    }
    if (show)
    {
        imshow("P[0]",P[0]); cv::waitKey();
        imshow("P[1]",P[1]); cv::waitKey();
        imshow("P[2]",P[2]); cv::waitKey();
        imshow("P[3]",P[3]); cv::waitKey();
    }
    
    for (int y=0; y<mask.rows; y++)
        for (int x=0; x<mask.cols; x++)
        {
            if (mask.at<unsigned char>(y,x) == 0)
            {
                dst.at<unsigned char>(y,x) = std::min( std::min(P[0].at<unsigned char>(y,x),
                                                                P[1].at<unsigned char>(y,x))
                                                       ,
                                                       std::min(P[2].at<unsigned char>(y,x),
                                                                P[3].at<unsigned char>(y,x))
                                                     );
            }
                                                 
        }
    return dst;
}

void Knowledge::findPotentailWordBoundraies(Spotting s, int* tlx, int* tly, int* brx, int* bry)
{
    //Set it to bounding box of connected component which intersects the given spottings  
    
    //int centerY = s.tly+(s.bry-s.tly)/2.0;
    cv::Mat b;// = GlobalK::otsuThresh(*t.pagePnt);
    int blockSize = max(s.brx-s.tlx,s.bry-s.tly);
    if (blockSize%2==0)
        blockSize++;
    cv::Mat orig = *s.pagePnt;
    if (orig.type()==CV_8UC3)
        cv::cvtColor(orig,orig,CV_RGB2GRAY);
    cv::adaptiveThreshold(orig, b, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY_INV, blockSize, 10);
    //cv::imshow("ff", b);
    //cv::waitKey();
    cv::Mat ele = cv::Mat::ones(3,5,CV_32F);//(cv::Mat_<float>(3,5) << 1, 1, 1, 1, 1, 1, 1, 1, 1);
    cv::dilate(b, b, ele);
    //cv::imshow("ff", b);
    //cv::waitKey();
    vector<cv::Point> ps;
    for (int r=s.tly; r<=s.bry; r++)
        for (int c=s.tlx; c<=s.brx; c++)
        {
            if (b.at<unsigned char>(r,c) >0)
            {
                //startX=c;
                //startY=r;
                ps.push_back(cv::Point(c,r));
                b.at<unsigned char>(r,c)=0;
                //r= s.bry+1;
                //break;
            }
        }
    if (ps.size()>0)
    {
        int minX = ps[0].x;
        int maxX = ps[0].x;
        int minY = ps[0].y;
        int maxY = ps[0].y;
        vector<cv::Point> rel = {cv::Point(0,1),cv::Point(0,-1),cv::Point(1,0),cv::Point(-1,0)};
        while (ps.size()>0)
        {
            cv::Point c = ps.back();
            ps.pop_back();
            for (cv::Point r : rel)
            {
                cv::Point n(r.x+c.x,r.y+c.y);
                if (b.at<unsigned char>(n) >0)
                {
                    b.at<unsigned char>(n)=0;
                    if (n.x<minX)
                        minX=n.x;
                    if (n.x>maxX)
                        maxX=n.x;
                    if (n.y<minY)
                        minY=n.y;
                    if (n.y>maxY)
                        maxY=n.y;
                    ps.push_back(n);
                }
            }
        }
       // cout<<"findPotentailWordBoundraies: "<<minX<<", "<<minY<<", "<<maxX<<", "<<maxY<<endl;
        *tlx=min(s.tlx,minX);
        *tly=min(s.tly,minY);
        *brx=max(s.brx,maxX);
        *bry=max(s.bry,maxY);
    }
    else
    {   //no word?
        cout<<"findPotentailWordBoundraies: No word?"<<endl;
        *tlx=s.tlx;
        *tly=s.tly;
        *brx=s.brx;
        *bry=s.bry;
    }
}*/

void Knowledge::Corpus::show()
{
    map<const cv::Mat*,cv::Mat> draw;
    pthread_rwlock_rdlock(&pagesLock);
    
    for (auto p : pages)
    {
        Page* page = p.second;
        vector<Line*> lines = page->lines();
        for (Line* line : lines)
        {
            int line_ty, line_by;
            vector<Word*> wordsForLine = line->wordsAndBounds(&line_ty,&line_by);
            for (Word* word : wordsForLine)
            {
                int tlx,tly,brx,bry;
                bool done;
                word->getBoundsAndDone(&tlx, &tly, &brx, &bry, &done);
                if (done)
                {
                    if (draw.find(word->getPage()) == draw.end())
                    {
                        if (word->getPage()->type() == CV_8UC3)
                            draw[word->getPage()] = word->getPage()->clone();
                        else
                        {
                            draw[word->getPage()] = cv::Mat();
                            cv::cvtColor(*word->getPage(),draw[word->getPage()],CV_GRAY2BGR);
                        }
                    }
                    cv::putText(draw[word->getPage()],word->getTranscription(),cv::Point(tlx+(brx-tlx)/2,tly+(bry-tly)/2),cv::FONT_HERSHEY_TRIPLEX,4.0,cv::Scalar(50,50,255));
                }
                //else
                //    cout<<"word not done at "<<tlx<<", "<<tly<<endl;
            }
        }
    }
    for (auto p : draw)
    {
        int h =1500;
        int w=((h+0.0)/p.second.rows)*p.second.cols;
        cv::resize(p.second, p.second, cv::Size(w,h));
        cv::imshow("a page",p.second);
        cv::waitKey();
    }
    cout<<"All pages."<<endl;
    cv::waitKey();

    pthread_rwlock_unlock(&pagesLock);
}


void Knowledge::Corpus::mouseCallBackFunc(int event, int x, int y, int flags, void* page_p)
{
    x*=2;
    y*=2;
     Page* page = (Page*) page_p;
     if  ( event == cv::EVENT_LBUTTONDOWN )
     {
          //cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;

        vector<Line*> lines = page->lines();
       /* for (Line* line : lines)
        {
            int line_ty, line_by;
            vector<Word*> wordsForLine = line->wordsAndBounds(&line_ty,&line_by);
            for (Word* word : wordsForLine)
            {
                int tlx,tly,brx,bry;
                bool done;
                word->getBoundsAndDone(&tlx, &tly, &brx, &bry, &done);
                if (tlx<=x && x<=brx && tly<=y && y<=bry)
                {
                    string query, gt;
                    word->getDoneAndGTAndQuery(&done, &gt, &query);
                    cout<<"WORD: "<<gt<<"  query: "<<query<<endl;
                    vector<Spotting> spots = word->getSpottings();
                    cout<<"Spots: ";
                    for (const Spotting & s : spots)
                    {
                        cout<<s.ngram<<", ";
                    }
                    cout<<endl<<"Poss trans: ";
                    vector<string> poss = word->getRestrictedLexicon(100);
                    for (string p : poss)
                    {
                        cout<<p<<", ";
                    }
                    cout<<endl;
                }
            }
        }*/
     }
     else if  ( event == cv::EVENT_RBUTTONDOWN )
     {
          //cout << "Right button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
     }
     else if  ( event == cv::EVENT_MBUTTONDOWN )
     {
          //cout << "Middle button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
     }
     else if ( event == cv::EVENT_MOUSEMOVE )
     {
          //cout << "Mouse move over the window - position (" << x << ", " << y << ")" << endl;

     }
}

void Knowledge::Corpus::showInteractive(int pageId)
{
    cv::Mat draw;
    pthread_rwlock_rdlock(&pagesLock);
    
    Page* page = pages[pageId];
    vector<Line*> lines = page->lines();
    for (Line* line : lines)
    {
        int line_ty, line_by;
        vector<Word*> wordsForLine = line->wordsAndBounds(&line_ty,&line_by);
        for (Word* word : wordsForLine)
        {
            if (draw.cols<2)
            {
                if (word->getPage()->type() == CV_8UC3)
                    draw = word->getPage()->clone();
                else
                {
                    cv::cvtColor(*word->getPage(),draw,CV_GRAY2BGR);
                }
            }
            int tlx,tly,brx,bry;
            bool done;
            word->getBoundsAndDone(&tlx, &tly, &brx, &bry, &done);
            if (done)
            {
                cv::putText(draw,word->getTranscription(),cv::Point(tlx+(brx-tlx)/2,tly+(bry-tly)/2),cv::FONT_HERSHEY_TRIPLEX,4.0,cv::Scalar(50,50,255));
            }
            //else
            //    cout<<"word not done at "<<tlx<<", "<<tly<<endl;
        }
    }
    //int h =1500;
    //int w=((h+0.0)/p.second.rows)*p.second.cols;
    cv::resize(draw,draw, cv::Size(), 0.5,0.5);
    cv::namedWindow("page");
    cv::setMouseCallback("page", mouseCallBackFunc, page);
    cv::imshow("page",draw);
    cv::waitKey();

    pthread_rwlock_unlock(&pagesLock);
}


void Knowledge::Corpus::showProgress(int height, int width)
{
    pthread_rwlock_rdlock(&pagesLock);
    int pageH=0;//pages.begin()->second->getImg()->rows;
    int pageW=0;//pages.begin()->second->getImg()->cols;
    for (auto p : pages)
    {
        Page* page = p.second;
        pageH = max(pageH,page->getImg()->rows);
        pageW = max(pageW,page->getImg()->cols);
    }
    float resizeScale=.001;
    int nAcross, nDown;
    for (float scale=0.5; scale>.001; scale-=.001)
    {
        nAcross=floor(width/(pageW*scale));
        nDown=floor(height/(pageH*scale));
        if (nAcross*nDown>=pages.size())
        {
            resizeScale=scale;
            break;
        }
    }
    cv::Mat draw = cv::Mat::zeros(height,width,CV_8UC3);
    draw = cv::Scalar(100,100,100);
    int xPos=0;
    int yPos=0;
    int across=0;
    for (auto p : pages)
    {
        Page* page = p.second;
        cv::Mat workingIm;
        if (page->getImg()->channels()==1)
            cv::cvtColor(*(page->getImg()), workingIm, CV_GRAY2RGB);
        else if (page->getImg()->channels()==3)
            workingIm = page->getImg()->clone();
        else
            assert(false);
        vector<Line*> lines = page->lines();
        for (Line* line : lines)
        {
            int line_ty, line_by;
            vector<Word*> wordsForLine = line->wordsAndBounds(&line_ty,&line_by);
            for (Word* word : wordsForLine)
            {
                int tlx,tly,brx,bry;
                bool done;
                word->getBoundsAndDone(&tlx, &tly, &brx, &bry, &done);
                if (done)
                {
                    for (int x=tlx; x<=brx; x++)
                        for (int y=tly; y<=bry; y++)
                        {
                            assert(x<workingIm.cols && y<workingIm.rows);
                            workingIm.at<cv::Vec3b>(y,x)[0] = 0.5*workingIm.at<cv::Vec3b>(y,x)[0];
                            workingIm.at<cv::Vec3b>(y,x)[1] = min(255,workingIm.at<cv::Vec3b>(y,x)[1]+120);
                            workingIm.at<cv::Vec3b>(y,x)[2] = 0.5*workingIm.at<cv::Vec3b>(y,x)[2];
                        }
                }
                /*else
                {
                    vector<Spotting> sps = word->getSpottings();
                    for (Spotting& s : sps)
                        for (int x=s.tlx; x<=s.brx; x++)
                            for (int y=s.tly; y<=s.bry; y++)
                            {
                                assert(x<workingIm.cols && y<workingIm.rows);
                                workingIm.at<cv::Vec3b>(y,x)[0] = 0.5*workingIm.at<cv::Vec3b>(y,x)[0];
                                workingIm.at<cv::Vec3b>(y,x)[2] = min(255,workingIm.at<cv::Vec3b>(y,x)[2]+120);
                                workingIm.at<cv::Vec3b>(y,x)[1] = 0.5*workingIm.at<cv::Vec3b>(y,x)[1];
                            }
                }*/
            }
        }
        cv::resize(workingIm,workingIm,cv::Size(),resizeScale,resizeScale);
        //cout <<"page dims: "<<workingIm.rows<<", "<<workingIm.cols<<"  at: "<<xPos<<", "<<yPos<<endl;
        //cv::imshow("test",workingIm);
        //cv::waitKey();
        assert(xPos>=0 && yPos>=0 && xPos+workingIm.cols<draw.cols && yPos+workingIm.rows<draw.rows);
        workingIm.copyTo(draw(cv::Rect(xPos,yPos,workingIm.cols,workingIm.rows)));
        xPos+=workingIm.cols;
        if (++across >= nAcross)
        {
            xPos=0;
            yPos+=workingIm.rows;
            across=0;
        }
    }
    //cv::imshow("progress",draw);
    //cv::waitKey(2000);
    cv::imwrite("progress/show.jpg",draw);

    pthread_rwlock_unlock(&pagesLock);
}



void Knowledge::Corpus::addWordSegmentaionAndGT(string imageLoc, string queriesFile)
{
    ifstream in(queriesFile);

    assert(in.is_open());
    string line;
    
    //std::getline(in,line);
    //float initSplit=0;//stof(line);//-0.52284769;
    //regex nonNum ("[^\\d]");
    numWordsReadIn=0;
    pthread_rwlock_wrlock(&pagesLock);
    while(std::getline(in,line))
    {
        if (line.length()==0)
            continue;
        vector<string> strV;
        //split(line,',',strV);
        std::stringstream ss(line);
        std::string item;
        while (std::getline(ss, item, ' ')) {
            strV.push_back(item);
        }
        
        
        string imageFile = strV[0];
        string pageName = (strV[0]);
        string gt = strV[5];
#ifdef NO_NAN
        assert(gt.compare(GlobalK::knowledge()->getSegWord(numWordsReadIn))==0);
#endif
        int tlx=stoi(strV[1]);
        int tly=stoi(strV[2]);
        int brx=stoi(strV[3]);
        int bry=stoi(strV[4]);
        
        //pageName = regex_replace (pageName,nonNum,"");
        //int pageId = stoi(pageName);
        if (pageIdMap.find(pageName)==pageIdMap.end())
        {
            int newId = pageIdMap.size()+1;
            pageIdMap[pageName]=newId;
        }
        int pageId = pageIdMap.at(pageName);
        Page* page;
        if (pages.find(pageId)==pages.end())
        {
            /*if (!writing)
            {
                pthread_rwlock_unlock(&pagesLock);
                pthread_rwlock_wrlock(&pagesLock);
                writing=true;
            }*/
            page = new Page(/*&spotter,*/imageLoc+"/"+imageFile,&averageCharWidth,&countCharWidth,pageId);
            pages[page->getId()] = page;
            //cout<<"new page "<<pageId<<endl;
        }
        else
        {
            page = pages[pageId];
        }
        if (tlx<0)
            tlx=0;
        if (tly<0)
            tly=0;
        if (brx >= page->getImg()->cols)
            brx=page->getImg()->cols-1;
        if (bry >= page->getImg()->rows)
            bry=page->getImg()->rows-1;
        if (tlx>brx || tly>bry)
        {
            cout<<"ERROR, fliped box: "+line<<endl;
            exit(1);
        }
        vector<Line*> lines = page->lines();
        bool oneLine=false;
        float bestOverlap=0;
        Line* bestLine=NULL;
        for (Line* line : lines)
        {
            int line_ty, line_by;
            line->wordsAndBounds(&line_ty,&line_by);
            int overlap = min(bry,line_by) - max(tly,line_ty);
            float overlapPortion = overlap/(0.0+bry-tly);
            //cout <<"overlap: "<<overlapPortion<<endl;
            if (overlapPortion > OVERLAP_LINE_THRESH)
            {
                oneLine=true;
                //line->addWord(tlx,tly,brx,bry,gt);
                if (overlapPortion>bestOverlap)
                {
                    bestOverlap=overlapPortion;
                    bestLine=line;
                }
                
            }
        }
        if (oneLine)
            bestLine->addWord(tlx,tly,brx,bry,gt);
        if (!oneLine)
        {
            page->addWord(tlx,tly,brx,bry,gt);
        }
        numWordsReadIn++;
    }
    
    /*double heightAvg=0;
    int count=0;

    for (auto pp : pages)
    {
        Page* page = pp.second;
        for (Line* line : page->lines())
        {
            for (Word* word : line->wordsAndBounds(NULL,NULL))
            {
                int topBaseline, botBaseline;
                word->getBaselines(&topBaseline,&botBaseline);
                heightAvg += botBaseline-topBaseline;
                count++;
            }
        }
    }
    heightAvg/=count;
    averageCharWidth=CHAR_ASPECT_RATIO*heightAvg;
    countCharWidth=10;
#ifdef TEST_MODE
    cout<<"avg char height: "<<heightAvg<<endl;
    cout<<"avg char width : "<<averageCharWidth<<endl;
#endif*/
    recreateDatasetVectors(false);
    pthread_rwlock_unlock(&pagesLock);

    in.close();

    //TODO
    //averageCharWidth = getAverageCharWidth();
}



const cv::Mat* Knowledge::Corpus::imgForPageId(int pageId) const
{

    const Page* page = pages.at(pageId);
    return page->getImg();
}

/*
TranscribeBatch* Knowledge::Corpus::makeManualBatch(int maxWidth, bool noSpottings)
{
    TranscribeBatch* ret=NULL;
    for (Word* word : _words)
    {
        int tlx,tly,brx,bry;
        bool done;
        bool sent;
        string gt;
        word->getBoundsAndDoneAndSentAndGT(&tlx, &tly, &brx, &bry, &done, &sent, &gt);
        //cout << "at word, "<<done<<", "<<sent<<endl;
        if (!done && !sent && (!noSpottings || word->getSpottings().size()==0))
        {
            vector<string> prunedDictionary = word->getRestrictedLexicon(PRUNED_LEXICON_MAX_SIZE);
            if (prunedDictionary.size()>PRUNED_LEXICON_MAX_SIZE)
                prunedDictionary.clear();
            ret = new TranscribeBatch(word,prunedDictionary,word->getPage(),word->getSpottingsPointer(),tlx,tly,brx,bry,gt);
            word->sent(ret->getId());
            //enqueue and dequeue to keep the queue's state good.
            vector<TranscribeBatch*> theBatch = {ret};
            manQueue.enqueueAll(theBatch);
            ret=manQueue.dequeue(maxWidth);
            break;
        }
    }
    return ret;
}

TranscribeBatch* Knowledge::Corpus::getManualBatch(int maxWidth)
{
    TranscribeBatch* ret=manQueue.dequeue(maxWidth);
    if (ret==NULL)
        ret=makeManualBatch(maxWidth,true);
    if (ret==NULL)
        ret=makeManualBatch(maxWidth,false);
    return ret;
}

void Knowledge::Corpus::checkIncomplete()
{
    manQueue.checkIncomplete();
}
vector<Spotting*> Knowledge::Corpus::transcriptionFeedback(unsigned long id, string transcription, vector<pair<unsigned long, string> >* toRemoveExemplars)
{
    if (transcription.find('\n') != string::npos)
        transcription="$PASS$";
    return manQueue.feedback(id,transcription,toRemoveExemplars);
}*/

const vector<string>& Knowledge::Corpus::labels() const
{
    return _gt;
}
int Knowledge::Corpus::size() const
{
    return _gt.size();
}
const Mat Knowledge::Corpus::image(unsigned int i) const
{
    return _wordImgs.at(i);
}
//unsigned int Knowledge::Corpus::wordId(unsigned int i) const
//{
//    return _words[i]->getId();
//}
Word* Knowledge::Corpus::getWord(unsigned int i) const
{
    //TODO, resource lock
    return _words.at(i);
}
Word* Knowledge::Corpus::word(unsigned int i)
{
    //TODO, resource lock
    if (_words.find(i)==_words.end())
        return NULL;
    return _words.at(i);
}
const Word* Knowledge::Corpus::word(unsigned int i) const
{
    //TODO, resource lock
    if (_words.find(i)==_words.end())
        return NULL;
    return _words.at(i);
}

void Knowledge::Corpus::recreateDatasetVectors(bool lockPages)
{
    _words.clear();
    _wordImgs.clear();
    _gt.clear();
    if (lockPages)
        pthread_rwlock_rdlock(&pagesLock);
    for (auto p : pages)
    {
        Page* page = p.second;
        vector<Line*> lines = page->lines();
        for (Line* line : lines)
        {
            int line_ty, line_by;
            vector<Word*> wordsInLine = line->wordsAndBounds(&line_ty,&line_by);
            for (Word* word : wordsInLine)
            {
                //word->setSpottingIndex(_words.size());
                //_words.push_back(word);
                //_wordImgs.push_back(word->getImg());
                //_gt.push_back(word->getGT());
                _words[word->getId()]=word;
                _wordImgs[word->getId()]=word->getImg();
                //_gt[word->getId()]=word->getGT();

            }

            //We assume no more words are added at the moment
        }
    }
    assert(_words.size()==numWordsReadIn);
    for (auto p : _words)
    {
        assert(p.second->getId() == _gt.size());
        _gt.push_back(p.second->getGT());
    }
    if (lockPages)
        pthread_rwlock_unlock(&pagesLock);
    
}

/*vector<Spotting>* Knowledge::Corpus::runQuery(SpottingQuery* query)// const
{
    vector<SpottingResult> res = spotter->runQuery(query);
    vector<Spotting>* ret = new vector<Spotting>(res.size());
    for (int i=0; i<res.size(); i++)
    {
        Word* w = getWord(res[i].imIdx);
        int tlx, tly, brx, bry;
        bool done;
        int gt=0;//UNKNOWN_GT;
        string wordGT;
        w->getBoundsAndDoneAndGT(&tlx, &tly, &brx, &bry, &done, &wordGT);
        //int posN = wordGT.find_first_of(query->getNgram());
        //if (posN ==  string::npos && wordGT.length()>0)
        //    gt=0;
        int endPos=wordGT.length()-(query->getNgram().length());
        for (int c=0; c<= endPos; c++)
        {
            assert(c+query->getNgram().length() <= wordGT.length());
            if (query->getNgram().compare( wordGT.substr(c,query->getNgram().length()) ) == 0)
            {
                //cout <<"found ["<<query->getNgram()<<"] in: "<<wordGT<<endl;
                gt=UNKNOWN_GT;
                break;
            }
        }

        ret->at(i) = Spotting(res[i].startX+tlx, tly, res[i].endX+tlx, bry, w->getPageId(), w->getPage(), query->getNgram(), res[i].score, gt);
        assert(i==0 || ret->at(i).id != ret->at(i-1).id);
        //if (done)
        //    w->preapproveSpotting(&ret->at(i));
    }
    return ret;
}*/

void Knowledge::Corpus::writeTranscribed(string retrainFile)
{
    ofstream out(retrainFile);
    pthread_rwlock_rdlock(&pagesLock);
    for (auto p : _words)
    {
        Word* word = p.second;
        int tlx, tly, brx, bry;
        bool done;
        word->getBoundsAndDone(&tlx,&tly,&brx,&bry,&done);
        if (done)
        {
            string trans = word->getTranscription();
            //toWrite[trans].push_back(pages.at(pageId)->getPageImgLoc()+" "+to_string(tlx)+" "+to_string(tly)+" "+to_string(brx)+" "+to_string(bry)+" "+trans);
            out << pages.at(word->getPageId())->getPageImgLoc()<<" "<<tlx<<" "<<tly<<" "<<brx<<" "<<bry<<" "<<trans<<endl;
        }
            
    }
    pthread_rwlock_unlock(&pagesLock);
    out.close();
}


/*CorpusRef* Knowledge::Corpus::getCorpusRef()
{
    CorpusRef* ret = new CorpusRef();
    for (int i=0; i<_words.size(); i++)
    {
        ret->addWord(i,_words.at(i),_words.at(i)->getPage());
        int x1,y1,x2,y2;
        bool toss;
        _words.at(i)->getBoundsAndDone(&x1,&y1,&x2,&y2,&toss);
        ret->addLoc(Location(_words.at(i)->getPageId(),x1,y1,x2,y2));
    }
    return ret;
}
PageRef* Knowledge::Corpus::getPageRef()
{
    PageRef* ret = new PageRef();
    for (auto p : pages)
    {
        ret->addPage(p.first,p.second->getImg());
    }

    //For debugging
    for (int i=0; i<_words.size(); i++)
    {
        int x1,y1,x2,y2;
        bool toss;
        _words.at(i)->getBoundsAndDone(&x1,&y1,&x2,&y2,&toss);
        ret->addWord(Location(_words.at(i)->getPageId(),x1,y1,x2,y2));
    }

    return ret;
}*/

void Knowledge::Page::save(ofstream& out)
{
    out<<"PAGE"<<endl;
    vector<Line*> ls = lines();
    out<<ls.size()<<"\n";
    out<<pageImgLoc<<"\n";
    for (Line* l : ls)
        l->save(out);
    out<<_id<<"\n";
    out<<id<<"\n";
}

Knowledge::Page::Page(ifstream& in, /*const Spotter* const* spotter,*/ float* averageCharWidth, int* countCharWidth) : /*spotter(spotter),*/ averageCharWidth(averageCharWidth), countCharWidth(countCharWidth)
{
    pthread_rwlock_init(&lock,NULL);
    string line;
    getline(in,line);
    assert(line.compare("PAGE")==0);
    getline(in,line);
    int sizeLines = stoi(line);
    getline(in,pageImgLoc);
    pageImg = cv::imread(pageImgLoc);
    assert(pageImg.cols*pageImg.rows > 1);
    _lines.resize(sizeLines);
    for (int i=0; i<sizeLines; i++)
    {
        _lines.at(i) = new Line(in,&pageImg,/*spotter,*/averageCharWidth,countCharWidth);
    }
    getline(in,line);
    _id=stoul(line);
    getline(in,line);
    id=stoul(line);
}

void Knowledge::Line::save(ofstream& out)
{
    out<<"LINE"<<endl;
    int cty, cby;
    vector<Word*> ws = wordsAndBounds(&cty,&cby);
    out<<ws.size()<<"\n";
    for (Word* w : ws)
    {
        w->save(out);
    }
    out<<cty<<"\n"<<cby<<"\n";
    out<<pageId<<"\n";
}

Knowledge::Line::Line(ifstream& in, const cv::Mat* pagePnt, /*const Spotter* const* spotter,*/ float* averageCharWidth, int* countCharWidth) : pagePnt(pagePnt), /*spotter(spotter),*/ averageCharWidth(averageCharWidth), countCharWidth(countCharWidth)
{
    pthread_rwlock_init(&lock,NULL);
    string line;
    getline(in,line);
    assert(line.compare("LINE")==0);
    getline(in,line);
    int wordSize = stoi(line);
    _words.resize(wordSize);
    for (int i=0; i<wordSize; i++)
    {
        _words.at(i)=new Word(in, pagePnt, averageCharWidth, countCharWidth);
    }
    getline(in,line);
    ty = stoi(line);
    getline(in,line);
    by = stoi(line);
    getline(in,line);
    pageId = stoi(line);
}

void Knowledge::Corpus::save(ofstream& out)
{
    out<<"CORPUS"<<endl;
    out<<averageCharWidth<<"\n"<<countCharWidth<<"\n"<<threshScoring<<"\n";
    pthread_rwlock_rdlock(&pagesLock);
    out<<pages.size()<<"\n";
    for (auto p : pages)
    {
        out<<p.first<<"\n";
        p.second->save(out);
    }
    out<<pageIdMap.size()<<"\n";
    for (auto p : pageIdMap)
    {
        out<<p.first<<"\n"<<p.second<<"\n";
    }
    out<<numWordsReadIn<<"\n";
    pthread_rwlock_unlock(&pagesLock);

    /*pthread_rwlock_rdlock(&spottingsMapLock);
    out<<spottingsToWords.size()<<"\n";
    for (auto p : spottingsToWords)
    {
        out<<p.first<<"\n"<<p.second.size()<<"\n";
        for (Word* w : p.second)
            out<<w->getId()<<"\n";
    }
    pthread_rwlock_unlock(&spottingsMapLock);

    /*out<<_gt.size()<<"\n";
    for (string g : _gt)
        out<<g<<"\n";
    out<<_words.size()<<"\n";
    for (auto w : _words)
        out<<w->getSpottingIndex()<<"\n";
    out<<_wordImgs.size()<<"\n";
    for (const Mat& m : _wordImages)*/
    //just call recreateDatasetVectors
}

Knowledge::Corpus::Corpus(ifstream& in)
{

    pthread_rwlock_init(&pagesLock,NULL);
    //pthread_rwlock_init(&spottingsMapLock,NULL);

    string line;
    getline(in,line);
    assert(line.compare("CORPUS")==0);
    getline(in,line);
    averageCharWidth = stof(line);
    getline(in,line);
    countCharWidth = stoi(line);
    getline(in,line);
    threshScoring = stof(line);
    getline(in,line);
    int pagesSize = stoi(line);
    for (int i=0; i<pagesSize; i++)
    {
        getline(in,line);
        int pageId = stoi(line);
        Page* page = new Page(in,/*&spotter,*/&averageCharWidth,&countCharWidth);
        pages[pageId]=page;
    }
    getline(in,line);
    pagesSize = stoi(line);
    for (int i=0; i<pagesSize; i++)
    {
        string s;
        getline(in,s);
        getline(in,line);
        int pageId = stoi(line);
        pageIdMap[s]=pageId;
    }
    getline(in,line);
    numWordsReadIn=stoi(line);
    recreateDatasetVectors(false);

    /*getline(in,line);
    int spottingsToWordsSize=stoi(line);
    for (int i=0; i<spottingsToWordsSize; i++)
    {
        getline(in,line);
        unsigned long sid=stoul(line);
        getline(in,line);
        int wordLen=stoi(line);
        for (int j=0; j<wordLen; j++)
        {
            getline(in,line);
            int spottingIndex=stoi(line);
            spottingsToWords[sid].push_back(_words[spottingIndex]);
        }
    }*/
}

//For data collection, when I deleted all my trans... :(
/*
vector<TranscribeBatch*> Knowledge::Corpus::resetAllWords_()
{
    vector<TranscribeBatch*> ret;
    vector<Spotting*> newExemplars;
    for (Word* w : _words)
    {
        TranscribeBatch* b = w->reset_(&newExemplars);
        if (b!=NULL)
            ret.push_back(b);
    }
    return ret;
}*/

/*TranscribeBatch* Knowledge::Word::reset_(vector<Spotting*>* newExemplars)
{
    done=false;
    loose=false;
    transcription="";
    query="";
    return queryForBatch(newExemplars);
}*/

void Knowledge::Corpus::getStats(float* accTrans, float* pWordsTrans, /*float* pWords80_100, float* pWords60_80, float* pWords40_60, float* pWords20_40, float* pWords0_20, float* pWords0,*/ string* misTrans,
                                 float* accTrans_IV, float* pWordsTrans_IV, /*float* pWords80_100_IV, float* pWords60_80_IV, float* pWords40_60_IV, float* pWords20_40_IV, float* pWords0_20_IV, float* pWords0_IV,*/ string* misTrans_IV)
{
    int trueTrans, cTrans, c80_100, c60_80, c40_60, c20_40, c0_20, c0;
    trueTrans= cTrans= c80_100= c60_80= c40_60= c20_40= c0_20= c0=0;
    *misTrans="";

    //IV is In-Vocabulary
    int trueTrans_IV, cTrans_IV, c80_100_IV, c60_80_IV, c40_60_IV, c20_40_IV, c0_20_IV, c0_IV;
    trueTrans_IV= cTrans_IV= c80_100_IV= c60_80_IV= c40_60_IV= c20_40_IV= c0_20_IV= c0_IV=0;
    *misTrans_IV="";
    
    int numIV=0;
    for (auto p : _words)
    {
        Word* w = p.second;
        bool done;
        string gt;
        w->getDoneAndGT(&done,&gt);
        for (int i=0; i<gt.length(); i++)
            gt[i]=tolower(gt[i]);
        bool inVocab = Lexicon::instance()->inVocab(gt);
        if (inVocab)
            numIV++;
        if (done)
        {
            cTrans++;
            if (inVocab)
                cTrans_IV++;
            string trans = w->getTranscription();
            for (int i=0; i<trans.length(); i++)
                trans[i] = tolower(trans[i]);
            if (gt.compare(trans)==0)
                trueTrans++;
            else
            {
                *misTrans+=trans+"("+gt+")_";
                if (inVocab)
                    *misTrans_IV+=trans+"("+gt+")_";
            }
        }
        /*else if (query.length()==0)
        {
            c0++;
            if (inVocab)
                c0_IV++;
        }
        else
        {
            int numMatch=0;
            int posGT=0;
            bool skip=false;
            char last='.';
            for (int i=0; i<query.length(); i++)
            {
                if (skip)
                {
                    if (query[i]==']')
                        skip=false;
                }
                else
                {
                    if (query[i]==']')
                    {
                        skip=true;
                        last='.';
                    }
                    else if (query[i]>='a' && query[i]<='z')
                    {
                        if (query[i]==last)
                        {
                            if (query[i]==gt[posGT])
                            {
                                posGT++;
                                numMatch++;
                            }
                        }
                        else
                        {
                            for (;posGT<gt.length(); posGT++)
                                if (gt[posGT]==query[i])
                                {
                                    numMatch++;
                                    posGT++;
                                    break;
                                }
                        }
                        last=query[i];
                    }
                }
            }
            float p = numMatch/(0.0+gt.length());
            if (p>.8)
            {
                c80_100++;
                if (inVocab)
                    c80_100_IV++;
            }
            else if (p>.6)
            {
                c60_80++;
                if (inVocab)
                    c60_80_IV++;
            }
            else if (p>.4)
            {
                c40_60++;
                if (inVocab)
                    c40_60_IV++;
            }
            else if (p>.2)
            {
                c20_40++;
                if (inVocab)
                    c20_40_IV++;
            }
            else
            {
                c0_20++;
                if (inVocab)
                    c0_20_IV++;
            }
        }*/
    }
    if (cTrans>0)
        *accTrans= trueTrans/(0.0+cTrans);
    else
        *accTrans=0;
    *pWordsTrans= cTrans/(0.0+_words.size());
    /**pWords80_100= c80_100/(0.0+_words.size());
    *pWords60_80= c60_80/(0.0+_words.size());
    *pWords40_60= c40_60/(0.0+_words.size());
    *pWords20_40= c20_40/(0.0+_words.size());
    *pWords0_20= c0_20/(0.0+_words.size());
    *pWords0= c0/(0.0+_words.size());
    */
    if (cTrans_IV>0)
        *accTrans_IV= trueTrans_IV/(0.0+cTrans_IV);
    else
        *accTrans_IV=0;
    *pWordsTrans_IV= cTrans_IV/(0.0+numIV);
    /**pWords80_100_IV= c80_100_IV/(0.0+numIV);
    *pWords60_80_IV= c60_80_IV/(0.0+numIV);
    *pWords40_60_IV= c40_60_IV/(0.0+numIV);
    *pWords20_40_IV= c20_40_IV/(0.0+numIV);
    *pWords0_20_IV= c0_20_IV/(0.0+numIV);
    *pWords0_IV= c0_IV/(0.0+numIV);
    */
}
