#include "Knowledge.h"

vector< cv::Vec3f > TranscribeBatch::colors = {cv::Vec3f(1.2,1.2,0.8),cv::Vec3f(1.2,0.8,1.2),cv::Vec3f(0.83,0.93,1.3),cv::Vec3f(0.85,1.3,0.85),cv::Vec3f(1.3,0.85,0.85),cv::Vec3f(0.85,0.85,1.3)};
atomic_ulong TranscribeBatch::_id;//I'm assuming 0 is the default value

TranscribeBatch::TranscribeBatch(WordBackPointer* origin, multimap<float,string> scored, const cv::Mat wordImg, const multimap<int,Spotting>* spottings, int tlx, int tly, int brx, int bry, unsigned long id)
{
    this->origin=origin;
    if (id!=0)
        this->id=id;
    else
        this->id = ++_id;
    for (auto p : scored)
    {
        possibilities.push_back(p.second);
    }
    if (wordImg.type()==CV_8UC3)
        this->wordImg = wordImg.clone();
    else
        cv::cvtColor(wordImg,this->wordImg,CV_GRAY2RGB);
    textImg = cv::Mat::zeros(40,this->wordImg.cols,CV_8UC3);

    int colorIndex=0;
    for (auto iter : *spottings)
    {
        const Spotting& s = iter.second;
        cv::Point org((min(wordImg.cols,s.brx-tlx)-max(0,s.tlx-tlx))*0.2 + max(0,s.tlx-tlx), 33);
        cv::putText(textImg, s.ngram, org, cv::FONT_HERSHEY_DUPLEX, 1.0, cv::Scalar(colors[colorIndex][0]*255,colors[colorIndex][1]*255,colors[colorIndex][2]*255),1);
            
        for (int r= max(0,s.tly-tly); r<min(wordImg.rows,s.bry-tly); r++)
            for (int c= max(0,s.tlx-tlx); c<min(wordImg.cols,s.brx-tlx); c++)
            {
                this->wordImg.at<cv::Vec3b>(r,c)[0] = min(255.f,this->wordImg.at<cv::Vec3b>(r,c)[0]*colors[colorIndex][0]);
                this->wordImg.at<cv::Vec3b>(r,c)[1] = min(255.f,this->wordImg.at<cv::Vec3b>(r,c)[1]*colors[colorIndex][1]);
                this->wordImg.at<cv::Vec3b>(r,c)[2] = min(255.f,this->wordImg.at<cv::Vec3b>(r,c)[2]*colors[colorIndex][2]);
                
            }
        colorIndex = (colorIndex+1)%colors.size();
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
    vector<TranscribeBatch*> ret;
    pthread_rwlock_rdlock(&pagesLock);
    Page* page = pages[s.pageId];
    pthread_rwlock_unlock(&pagesLock);
    if (page==NULL)
    {
        page = new Page();
        pthread_rwlock_wrlock(&pagesLock);
        pages[s.pageId] = page;
        pthread_rwlock_unlock(&pagesLock);
    }
    
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
                    int overlap = max(0,min(s.bry,word_bry) - max(s.tly,word_tly)) * max(0,min(s.brx,word_brx) - max(s.tlx,word_tlx));
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
                //make a new word
                assert(false);
                line->addWord(s);
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
    //pthread_rwlock_unlock(&page->linesLock);
    
    return ret;
}

void Knowledge::Corpus::removeSpotting(unsigned long sid)
{

    pthread_rwlock_wrlock(&spottingsMapLock);
    vector<Word*> words = spottingsToWords[sid];
    spottingsToWords[sid].clear(); 
    pthread_rwlock_unlock(&spottingsMapLock);
    for (Word* word : words)
    {
        
        unsigned long retractId=0;
        TranscribeBatch* newBatch = word->removeSpotting(sid,&retractId);
        if (retractId!=0 && newBatch==0)
        {
            //TODO retract the batch
        }
        else if (newBatch != NULL)
        {
            //TODO modify batch
        }
    }
}

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
    TranscribeBatch* ret=queryForBatch();
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
            if (scored.size()>0 && scored.size()<THRESH_SCORING_COUNT)
            {
                //ret= createBatch(scored);
                ret = new TranscribeBatch(this,scored,(*pagePnt)(cv::Rect(tlx,tly,brx-tlx,bry-tly)),&spottings,tlx,tly,brx,bry,sentBatchId);
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
    *sentBatchId = this->sentBatchId;
    for (auto iter= spottings.begin(); iter!=spottings.end(); iter++)
    {
        if (iter->second.id == sid)
        {
            spottings.erase(iter);
            break;
        }
    }
    TranscribeBatch* ret=queryForBatch();
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


void Knowledge::findPotentailWordBoundraies(Spotting s, int* tlx, int* tly, int* brx, int* bry)
{
    //Set it to bounding box of connected component which intersects the given spottings  
    
    //int centerY = s.tly+(s.bry-s.tly)/2.0;
    cv::Mat b;// = otsu(*t.pagePnt);
    int blockSize = max(s.brx-s.tlx,s.bry-s.tly);
    if (blockSize%2==0)
        blockSize++;
    cv::adaptiveThreshold(*s.pagePnt, b, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY_INV, blockSize, 10);
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
                        if (word->getPage()->dims ==3)
                            draw[word->getPage()] = word->getPage()->clone();
                        else
                            cv::cvtColor(*word->getPage(),draw[word->getPage()],CV_GRAY2BGR);
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

