#include "Knowledge.h"

vector< cv::Vec3f > TranscribeBatch::colors = {cv::Vec3f(1.15,1.15,0.85),cv::Vec3f(1.15,0.85,1.15),cv::Vec3f(0.86,0.96,1.2),cv::Vec3f(0.87,1.2,0.87),cv::Vec3f(1.2,0.87,0.87),cv::Vec3f(0.87,0.87,1.2)};
cv::Vec3f TranscribeBatch::wordHighlight(0.9,1.2,1.2);
atomic_ulong TranscribeBatch::_id;//I'm assuming 0 is the default value

void TranscribeBatch::highlightPix(cv::Vec3b &p, cv::Vec3f color)
{

    p[0] = min(255.f,p[0]*color[0]);
    p[1] = min(255.f,p[1]*color[1]);
    p[2] = min(255.f,p[2]*color[2]);
}

TranscribeBatch::TranscribeBatch(WordBackPointer* origin, multimap<float,string> scored, const cv::Mat* origImg, const multimap<int,Spotting>* spottings, int tlx, int tly, int brx, int bry, unsigned long id)
{
    this->origin=origin;
    if (id!=0)
        this->id=id;
    else
        this->id = --_id;
    for (auto p : scored)
    {
        possibilities.push_back(p.second);
    }
    this->origImg=origImg;
    this->spottings=spottings;
    this->tlx=tlx;
    this->tly=tly;
    this->brx=brx;
    this->bry=bry;


    int wordH = bry-tly+1;
    int wordW = brx-tlx+1;

    wordImg = cv::Mat::zeros(wordH,wordW,origImg->type());
    (*origImg)(cv::Rect(tlx,tly,wordW,wordH)).copyTo(wordImg(cv::Rect(0, 0, wordW, wordH)));
    if (wordImg.type()!=CV_8UC3)
        cv::cvtColor(wordImg,wordImg,CV_GRAY2RGB);

    //textImg = cv::Mat::zeros(50,wordW,CV_8UC3);

    int colorIndex=0;
    for (auto iter : *spottings)
    {
        const Spotting& s = iter.second;
        cv::Point org((int)((min(wordW,(s.brx-tlx))-max(0,(s.tlx-tlx)))*0.2 + max(0,(s.tlx-tlx))) , 33);
        //cv::putText(textImg, s.ngram, org, cv::FONT_HERSHEY_DUPLEX, 1.0, cv::Scalar(colors[colorIndex][0]*255,colors[colorIndex][1]*255,colors[colorIndex][2]*255),1);
        spottingPoints.push_back(SpottingPoint(s.id,org.x,s.ngram,colors[colorIndex][0]*255,colors[colorIndex][1]*255,colors[colorIndex][2]*255));    
        for (int r= max(0,s.tly-tly); r<min(wordH,(s.bry-tly)); r++)
            for (int c= max(0,s.tlx-tlx); c<min(wordW,(s.brx-tlx)); c++)
            {
                highlightPix(wordImg.at<cv::Vec3b>(r,c),colors[colorIndex]);
                
            }
        colorIndex = (colorIndex+1)%colors.size();
    }
    for (int r= 0; r<5; r++)
        for (int c= 0; c<wordW; c++)
        {
            highlightPix(wordImg.at<cv::Vec3b>(r,c),wordHighlight);
            
        }
    for (int r= wordH-6; r<wordH; r++)
        for (int c= 0; c<wordW; c++)
        {
            highlightPix(wordImg.at<cv::Vec3b>(r,c),wordHighlight);
        }
    for (int r= 5; r<wordH-6; r++)
        for (int c= 0; c<5; c++)
        {
            highlightPix(wordImg.at<cv::Vec3b>(r,c),wordHighlight);
        }
    for (int r= 5; r<wordH-6; r++)
        for (int c= wordW-6; c<wordW; c++)
        {
            highlightPix(wordImg.at<cv::Vec3b>(r,c),wordHighlight);
        }
}

void TranscribeBatch::setWidth(unsigned int width) 
{

    int wordH = wordImg.rows;
    int wordW = wordImg.cols;
    //int textH= textImg.rows;
    newWordImg = cv::Mat::zeros(wordH,width,CV_8UC3);
    //newTextImg = cv::Mat::zeros(textH,width,CV_8UC3);
    int padLeft = max((((int)width)-wordW)/2,0);
    for (SpottingPoint& sp : spottingPoints)
        sp.setPad(padLeft);
    double scale=1.0;
    if (width>=wordW)
    {
        if (width>wordW)
            (*origImg)(cv::Rect(tlx-padLeft,tly,width,wordH)).copyTo(newWordImg(cv::Rect(0, 0, width, wordH)));
        wordImg(cv::Rect(0,0,wordW,wordH)).copyTo(newWordImg(cv::Rect(padLeft, 0, wordW, wordH)));
        //textImg(cv::Rect(0,0,wordW,textH)).copyTo(newTextImg(cv::Rect(padLeft, 0, wordW, textH)));
    }
    else
    {
        scale = width/(0.0+wordW);
        cv::resize(wordImg(cv::Rect(0,0,wordW,wordH)), newWordImg, cv::Size(), scale,scale, cv::INTER_CUBIC );
        //cv::resize(textImg(cv::Rect(0,0,wordW,textH)), newTextImg, cv::Size(), scale,1, cv::INTER_CUBIC );
    }


}

Knowledge::Corpus::Corpus()
{
    pthread_rwlock_init(&pagesLock,NULL);
    pthread_rwlock_init(&spottingsMapLock,NULL);
    //averageCharWidth=40;
    threshScoring= 1.0;
}

vector<TranscribeBatch*> Knowledge::Corpus::addSpotting(Spotting s)
{
    cout <<"addSpotting"<<endl;
    vector<TranscribeBatch*> ret;
    pthread_rwlock_rdlock(&pagesLock);
    Page* page = pages[s.pageId];
    pthread_rwlock_unlock(&pagesLock);
    if (page==NULL)
    {
        /*page = new Page();
        pthread_rwlock_wrlock(&pagesLock);
        pages[s.pageId] = page;
        pthread_rwlock_unlock(&pagesLock);*/
        assert(false && "ERROR, page not present");
    }
    
    addSpottingToPage(s,page,ret);
    
    cout <<"END addSpotting"<<endl;
    return ret;
}
vector<TranscribeBatch*> Knowledge::Corpus::updateSpottings(vector<Spotting>* spottings, vector<unsigned long>* removeSpottings, vector<unsigned long>* toRemoveBatches)
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
            /*if (!writing)
            {
                pthread_rwlock_unlock(&pagesLock);
                cout <<"addSpottings: release lock"<<endl;
                pthread_rwlock_wrlock(&pagesLock);
                cout <<"addSpottings: got write lock"<<endl;
                writing=true;
            }
            page = new Page();
            pages[s.pageId] = page;*/
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
        addSpottingToPage(spottings->at(i),thesePages[i],ret);

    //Removing spottings
    map<unsigned long, vector<Word*> > wordsForIds;
    if (removeSpottings)
    {
        pthread_rwlock_wrlock(&spottingsMapLock);
        for (unsigned long sid : *removeSpottings)
        {
            wordsForIds[sid] = spottingsToWords[sid];
            spottingsToWords[sid].clear(); 
        }
        pthread_rwlock_unlock(&spottingsMapLock);
        for (unsigned long sid : *removeSpottings)
        {
            vector<Word*> words = wordsForIds[sid];
            for (Word* word : words)
            {
                
                unsigned long retractId=0;  
                TranscribeBatch* newBatch = word->removeSpotting(sid,&retractId);
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

void Knowledge::Corpus::addSpottingToPage(Spotting& s, Page* page, vector<TranscribeBatch*>& ret)
{
    cout<<"  addSpottingsToPage"<<endl;
    vector<Line*> possibleLines;
    //pthread_rwlock_rdlock(&page->linesLock);
    vector<Line*> lines = page->lines();
    bool oneLine=false;
    for (Line* line : lines)
    {
        int line_ty, line_by;
        vector<Word*> words = line->wordsAndBounds(&line_ty,&line_by);
        int overlap = min(s.bry,line_by) - max(s.tly,line_ty);
        float overlapPortion = overlap/(0.0+s.bry-s.tly);
        if (overlapPortion > OVERLAP_LINE_THRESH)
        {
            oneLine=true;
            //pthread_rwlock_rdlock(&line->wordsLock);
            bool oneWord=false;
            for (Word* word : words)
            {
                int word_tlx, word_tly, word_brx, word_bry;
                bool isDone;
                word->getBoundsAndDone(&word_tlx,&word_tly,&word_brx,&word_bry,&isDone);
                if (!isDone)
                {
                    int overlap = (max(0,min(s.bry,word_bry) - max(s.tly,word_tly))) * (max(0,min(s.brx,word_brx) - max(s.tlx,word_tlx)));
                    float overlapPortion = overlap/(0.0+(s.bry-s.tly)*(s.brx-s.tlx));
                    cout <<"Overlap for word "<<word<<": overlapPortion="<<overlapPortion<<" ="<<overlap<<"/"<<(0.0+(s.bry-s.tly)*(s.brx-s.tlx))<<endl;
                    TranscribeBatch* newBatch=NULL;
                    if (overlapPortion > OVERLAP_WORD_THRESH)
                    {
                        oneWord=true;
                        //possibleWords.push_back(word);
                        newBatch = word->addSpotting(s);
                        pthread_rwlock_wrlock(&spottingsMapLock);
                        spottingsToWords[s.id].push_back(word);
                        pthread_rwlock_unlock(&spottingsMapLock);
                    }
                    
                    if (newBatch != NULL)
                    {
                        /*submit/update batch
                        cout<<"Batch Possibilities: ";
                        for (string pos : newBatch->getPossibilities())
                        {
                            cout << pos <<", ";
                        }
                        cout <<endl;
                        cv::imshow("highligh",newBatch->getImage());
                        cv::waitKey();*/
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
            
            /*if (possibleWords.size()>0)
            {
                for (Word* word : possibleWords)
                {
                    word->addSpottings(s);
                    spottingsToWords[s.id].push_back(word);
                }
            }
            else
            {
                //make a new word
                assert(false);
            }*/
        }
    }
    if (!oneLine)
    {
        //Addline
        //I'm assumming most lines are added prior with a preprocessing step
        page->addLine(s);
    }
    cout<<"  END addSpottingsToPage"<<endl;
}

/*void Knowledge::Corpus::removeSpotting(unsigned long sid)
{

    pthread_rwlock_wrlock(&spottingsMapLock);
    vector<Word*> words = spottingsToWords[sid];
    spottingsToWords[sid].clear(); 
    pthread_rwlock_unlock(&spottingsMapLock);
    for (Word* word : words)
    {
        
        unsigned long retractId=0;
        TranscribeBatch* newBatch = word->removeSpotting(sid,&retractId);
        if (retractId!=0 && newBatch==NULL)
        {
            //TODO retract the batch
        }
        else if (newBatch != NULL)
        {
            //TODO modify batch
        }
    }
}*/

TranscribeBatch* Knowledge::Word::addSpotting(Spotting s)
{
    pthread_rwlock_wrlock(&lock);
    //decide if it should be merge with another
    int width = s.brx-s.tlx;
    bool merged=false;
    for (auto otherS : spottings)
    {
        if (otherS.second.ngram.compare(s.ngram)==0)
        {
            int overlap = min(otherS.second.brx,s.brx)-max(otherS.second.tlx,s.tlx);
            if (overlap > width/2)
            {
                //These are probably the same and should be merged
                otherS.second.tlx = (otherS.second.tlx+s.tlx)/2;
                otherS.second.brx = (otherS.second.brx+s.brx)/2;
                merged=true;
                cout <<"merge"<<endl;
                break;
            }
        }
    }
    if (!merged)
    {
        spottings.emplace(s.tlx,s);//multimap, so they are properly ordered
    }
    TranscribeBatch* ret=queryForBatch();//This has an id matching the sent batch (if it exists)
    pthread_rwlock_unlock(&lock);
    return ret;
}

TranscribeBatch* Knowledge::Word::queryForBatch()
{

    string newQuery = generateQuery();
    TranscribeBatch* ret=NULL;
    if (query.compare(newQuery) !=0)
    {
        query=newQuery;
        vector<string> matches = Lexicon::instance()->search(query,meta);
        if (matches.size() < THRESH_LEXICON_LOOKUP_COUNT)
        {
            multimap<float,string> scored = scoreAndThresh(matches);//,*threshScoring);
            if (scored.size() == 1)
            {
                transcription=scored.begin()->second;
                done=true;
            }
            else if (scored.size()>0 && scored.size()<THRESH_SCORING_COUNT)
            {
                //ret= createBatch(scored);
                ret = new TranscribeBatch(this,scored,pagePnt,&spottings,tlx,tly,brx,bry,sentBatchId);
            }
        }
        else if (matches.size()==0)
        {
            //TODO, OoV or something is wrong. Perhaps return as manual batch
            //ret= createManualBatch();
        }
    }
    if (ret!=NULL)
        sentBatchId=ret->getId();
    return ret;
}
TranscribeBatch* Knowledge::Word::removeSpotting(unsigned long sid, unsigned long* sentBatchId)
{
    pthread_rwlock_wrlock(&lock);
    if (sentBatchId!=NULL)
        *sentBatchId = this->sentBatchId;
    for (auto iter= spottings.begin(); iter!=spottings.end(); iter++)
    {
        if (iter->second.id == sid)
        {
            spottings.erase(iter);
            break;
        }
    }
    TranscribeBatch* ret=NULL;
    if (spottings.size()>0)
        ret=queryForBatch();
    pthread_rwlock_unlock(&lock);
    return ret;
}

multimap<float,string> Knowledge::Word::scoreAndThresh(vector<string> match)//, float thresh)
{
    //TODO, perhaps find a good thresh point?
    multimap<float,string> ret;
    for (string m : match)
        ret.emplace(0.1,m);
    return ret;
}

/*TranscribeBatch* Knowledge::Word::createBatch(multimap<float,string> scored)
{
    vector<string> pos;
    for (auto p : scored)
    {
        pos.push_back(p.second);
    }    
    return new TranscribeBatch(pos,cv::Mat(),cv::Mat());
}*/

string Knowledge::Word::generateQuery()
{
    auto spot = spottings.begin();
    int pos = tlx;
    string ret = "";
    while (spot != spottings.end())
    {
        int dif = spot->second.tlx-pos;
        float numChars = dif/(0.0+averageCharWidth);
        cout <<"pos: "<<pos<<" str: "<<spot->second.tlx<<endl;
        cout <<"num chars: "<<numChars<<endl;
        
        
        if (numChars>0)
        {
            int least = floor(numChars);
            int most = ceil(numChars);
            
            if (numChars-least < THRESH_UNKNOWN_EST)
                least-=1;
            if (most-numChars < THRESH_UNKNOWN_EST)
                most+=1;
            least = max(0,least);
            ret += "[a-zA-Z0-9]{"+to_string(least)+","+to_string(most)+"}";
        }
        else
        {
            if (spot->second.ngram.length()>1 && ret[ret.length()-2] == spot->second.ngram[0] && ret[ret.length()-1] == spot->second.ngram[1])
            {
                ret = ret.substr(0,ret.length()-2) + "(" + ret.substr(ret.length()-2,2) + ")?";
            }
            else if (ret[ret.length()-1] == spot->second.ngram[0])
                ret+="?";
            //if (-1*numChars<THRESH_UNKNOWN_EST/2)
            //    ret += "[a-zA-Z0-9]?";
        }
        ret += spot->second.ngram;
        pos = spot->second.brx;
        spot++;
    }
    
    int dif = brx-pos;
    float numChars = dif/(0.0+averageCharWidth);
    cout <<"pos: "<<pos<<" end: "<<brx<<endl;
    cout <<"E num chars: "<<numChars<<endl;
    if (numChars>0)
    {
        int least = floor(numChars);
        int most = ceil(numChars);
        
        if (numChars-least < THRESH_UNKNOWN_EST)
            least-=1;
        if (most-numChars < THRESH_UNKNOWN_EST)
            most+=1;
        least = max(0,least);
        ret += "[a-zA-Z0-9]{"+to_string(least)+","+to_string(most)+"}";
    }
    /*else
    {
        if (-1*numChars<THRESH_UNKNOWN_EST/2)
            ret += "[a-zA-Z0-9]?";
    }*/
    cout << "query : "<<ret<<endl;
    return ret;
}

multimap<string, const cv::Mat> Knowledge::Word::harvest()
{
    multimap<string, const cv::Mat> ret;
    string unspotted = gt;
    multimap<int,const Spotting*> spottingsByIndex;
    int curI =0;
    for (auto p : spottings)
    {
        for (; curI<gt.length(); curI++)
        {
            if (gt.substr(curI,p.second.ngram.length()).compare(p.second.ngram))
            {
                spottingsByIndex[curI]=&p.second;
                for (int i=curI; i<curI+p.second.ngram.length(); i++)
                    unspotted[i]='$';
                break;
            }
        }
    }
    for (int i=0; i< gt.length()-MIN_N; i++)
    {
        for (int n=MIN_N; n<=MAX_N; n++)
        {
            if ((i==0||unspotted[i-1]=='$') && (i==gt.length()-n || (i<gt.length-n && unspotted[i+1]=='$')))
            {
                bool isNew=false;
                for (int offset=0; offset<n; offset++)
                {
                    if (unspotted[i+offset]!='$')
                    {
                        isNew=true;
                        break;
                    }
                }
                if (isNew)
                {
                    string ngram = gt.substr(i,n);
                    if ( isImportant(ngram) )
                    {
                        int etlx, ebrx;
                        //getting left x location
                        int lowerBound=0;
                        int uppperBound=0;
                        int bc=0;
                        //start from first char of new exemplar, working backwards
                        for (int si=i; si>=0; si--)
                        {
                            auto startAndEnd = spottingsByIndex(si);
                            for (auto iter=startAndEnd.first; iter!=startAndEnd.second; iter++)
                            {
                                if (iter->second.ngram.length()+si==i)
                                {
                                    assert(iter->second.ngram[iter->second.ngram.length()-1]==gt[i-1]);
                                    //bounds from end
                                    lowerBound += iter->second.brx - (iter->second.brx-iter->second.tlx)/8;
                                    upperBound += iter->second.brx + (iter->second.brx-iter->second.tlx)/8;
                                    bc++;
                                }
                                else if (iter->second.ngram.length()+si>i)
                                {
                                    //bounds from middle
                                    int numOverlap = iter->second.ngram.length()+si-i;
                                    for (int oo=0; oo<numOverlap; oo++)
                                        assert( gt[i+oo]==ngram[oo] );
                                    int per = (iter->second.brx-iter->second.tlx)/iter->ngram.length;
                                    int loc = per * (iter->second.ngram.length()-numOverlap) + iter->second.brx;
                                    lowerBound += loc - (iter->second.brx-iter->second.tlx)/5;
                                    upperBound += loc + (iter->second.brx-iter->second.tlx)/5;
                                    bc++;
                                }
                            }
                        }
                        assert(bc>0);
                        lowerBound/=bc;
                        uppBounds/=bc;
                        etlx = getBreakPoint(lowerBound,tly,upperBound,bry,pagePnt);

                        //getting right x location
                        lowerBound=0;
                        uppperBound=0;
                        bc=0;
                        //search from one char into the new exemplar, working forwards
                        for (int si=i+1; si<=i+n; si++)
                        {
                            auto startAndEnd = spottingsByIndex(si);
                            for (auto iter=startAndEnd.first; iter!=startAndEnd.second; iter++)
                            {
                                if (si==i+n)
                                {
                                    assert(iter->second.ngram[0]==gt[i+n]);
                                    //bounds from front
                                    lowerBound += iter->second.tlx - (iter->second.brx-iter->second.tlx)/8;
                                    upperBound += iter->second.tlx + (iter->second.brx-iter->second.tlx)/8;
                                    bc++;
                                }
                                else if (si<i+n)
                                {
                                    //bounds from middle
                                    int numTo = (i+n)-si;
                                    int per = (iter->second.brx-iter->second.tlx)/iter->ngram.length;
                                    int loc = per * (numTo) + iter->second.tlx;
                                    lowerBound += loc - (iter->second.brx-iter->second.tlx)/5;
                                    upperBound += loc + (iter->second.brx-iter->second.tlx)/5;
                                    bc++;
                                }
                            }
                        }
                        assert(bc>0);
                        lowerBound/=bc;
                        uppBounds/=bc;
                        ebrx = getBreakPoint(lowerBound,tly,upperBound,bry,pagePnt);

                        //Spotting toRet(etlx,tly,ebrx,bry,pageId,pagePnt,ngram,0.0);
                        //toRet.setHarvested();
                        //ret.push_back(ret);
                        ret.emplace(ngram,(*pagePnt)(cv::Rect(etlx,etly,ebrx-etlx+1,bry-tly+1));
                    }
                }
            }
        }
    }
    return ret;

}

void Knowledge::findPotentailWordBoundraies(Spotting s, int* tlx, int* tly, int* brx, int* bry)
{
    //Set it to bounding box of connected component which intersects the given spottings  
    
    //int centerY = s.tly+(s.bry-s.tly)/2.0;
    cv::Mat b;// = otsu(*t.pagePnt);
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
        cout<<"findPotentailWordBoundraies: "<<minX<<", "<<minY<<", "<<maxX<<", "<<maxY<<endl;
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
}

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
            vector<Word*> words = line->wordsAndBounds(&line_ty,&line_by);
            for (Word* word : words)
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
                    cv::putText(draw[word->getPage()],word->getTranscription(),cv::Point(tlx+(brx-tlx)/2,tly+(bry-tly)/2),cv::FONT_HERSHEY_PLAIN,2.0,cv::Scalar(50,50,255));
                }
                else
                    cout<<"word not done at "<<tlx<<", "<<tly<<endl;
            }
        }
    }
    for (auto p : draw)
    {
        cv::imshow("a page",p.second);
        cv::waitKey();
    }

    pthread_rwlock_unlock(&pagesLock);
}

void Knowledge::Corpus::addWordSegmentaionAndGT(string imageLoc, string queriesFile)
{
    ifstream in(queriesFile);

    assert(in.is_open());
    string line;
    
    //std::getline(in,line);
    //float initSplit=0;//stof(line);//-0.52284769;
    
    while(std::getline(in,line))
    {
        vector<string> strV;
        //split(line,',',strV);
        std::stringstream ss(line);
        std::string item;
        while (std::getline(ss, item, ' ')) {
            strV.push_back(item);
        }
        
        
        
        string imageFile = strV[0];
        int pageId = stoi(strV[0]);
        string gt = strV[5];
        int tlx=stoi(strV[1]);
        int tly=stoi(strV[2]);
        int brx=stoi(strV[3]);
        int bry=stoi(strV[4]);
        

        Page* page;
        if (pages.find(pageId)==pages.end())
        {
            /*if (!writing)
            {
                pthread_rwlock_unlock(&pagesLock);
                pthread_rwlock_wrlock(&pagesLock);
                writing=true;
            }*/
            page = new Page(imageLoc+"/"+imageFile);
            pages[pageId] = page;
        }
        else
        {
            page = pages[pageId];
        }
        vector<Line*> lines = page->lines();
        bool oneLine=false;
        for (Line* line : lines)
        {
            int line_ty, line_by;
            line->wordsAndBounds(&line_ty,&line_by);
            int overlap = min(bry,line_by) - max(tly,line_ty);
            float overlapPortion = overlap/(0.0+bry-tly);
            if (overlapPortion > OVERLAP_LINE_THRESH)
            {
                oneLine=true;
                line->addWord(tlx,tly,brx,bry,gt);
                
            }
        }
        if (!oneLine)
        {
            page->addWord(tlx,tly,brx,bry,gt);
        }
    }

    in.close();
}


cv::Mat* Knowledge::Corpus::imgForPageId(int pageId)
{

    Page* page = pages[pageId];
    return page->getImg();
}

