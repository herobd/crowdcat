#include "Knowledge.h"


Knowledge::Corpus::Corpus()
{
    pthread_rwlock_init(&pagesLock,NULL);
    averageCharWidth=40;
    threshScoring= 1.0;
}

void Knowledge::Corpus::addSpotting(Spotting s)
{
    pthread_rwlock_rdlock(&pagesLock);
    Page* page = pages[s.pageId];
    pthread_rwlock_unlock(&pagesLock);
    if (page==NULL)
    {
        pthread_rwlock_wrlock(&pagesLock);
        pages[s.pageId] = new Page();
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
                word->getBounds(&word_tlx,&word_tly,&word_brx,&word_bry);
                int overlap = max(0,min(s.bry,word_bry) - max(s.tly,word_tly)) * max(0,min(s.brx,word_brx) - max(s.tlx,word_tlx));
                float overlapPortion = overlap/(0.0+(s.bry-s.tly)*(s.brx-s.tlx));
                TranscribeBatch* newBatch=NULL;
                if (overlapPortion > OVERLAP_WORD_THRESH)
                {
                    oneWord=true;
                    //possibleWords.push_back(word);
                    newBatch = word->addSpotting(s);
                    spottingsToWords[s.id].push_back(word);
                }
                
                if (newBatch != NULL)
                {
                    //TODO submit/update batch
                    cout<<"Batch Possibilities: ";
                    for (string pos : newBatch->getPossibilities())
                    {
                        cout << pos <<", ";
                    }
                    cout <<endl;
                }
            }
            
            if (!oneWord)
            {
                //TODO make a new word
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
    
    
}

TranscribeBatch* Knowledge::Word::addSpotting(Spotting s)
{
    pthread_rwlock_wrlock(&lock);
    //decide if it should be merge with another
    int width = s.brx=s.tlx;
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
                break;
            }
        }
    }
    if (!merged)
    {
        spottings.emplace(s.tlx,s);//multimap, so they are properly ordered
    }
    string newQuery = generateQuery();
    TranscribeBatch* ret=NULL;
    if (query.compare(newQuery) !=0)
    {
        query=newQuery;
        vector<string> matches = Lexicon::search(query,meta);
        if (matches.size() < THRESH_LEXICON_LOOKUP_COUNT)
        {
            multimap<float,string> scored = scoreAndThresh(matches);//,*threshScoring);
            if (scored.size()>0 && scored.size()<THRESH_SCORING_COUNT)
            {
                ret= createBatch(scored);
            }
        }
        else if (matches.size()==0)
        {
            //TODO, OoV or something is wrong. Perhaps return as manual batch
            //ret= createManualBatch();
        }
    }
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

TranscribeBatch* Knowledge::Word::createBatch(multimap<float,string> scored)
{
    vector<string> pos;
    for (auto p : scored)
    {
        pos.push_back(p.second);
    }    
    return new TranscribeBatch(pos,cv::Mat(),cv::Mat());
}

string Knowledge::Word::generateQuery()
{
    auto spot = spottings.begin();
    int pos = tlx;
    string ret = "";
    while (spot != spottings.end())
    {
        int dif = spot->second.tlx-pos;
        float numChars = dif/Knowledge::averageCharWidth;
        
        
        
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
            if (-1*numChars<THRESH_UNKNOWN_EST)
                ret += "[a-zA-Z0-9]?";
        }
        ret += spot->second.ngram;
        pos = spot->second.brx;
        spot++;
    }
    
    int dif = brx-pos;
    float numChars = dif/averageCharWidth;
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
        if (-1*numChars<THRESH_UNKNOWN_EST)
            ret += "[a-zA-Z0-9]?";
    }
    
}


void Knowledge::findPotentailWordBoundraies(Spotting s, int* tlx, int* tly, int* brx, int* bry)
{
    //Set it to bounding box of connected component which intersects the given spottings  
    
    //int centerY = s.tly+(s.bry-s.tly)/2.0;
    cv::Mat b;// = otsu(*t.pagePnt);
    cv::adaptiveThreshold(*s.pagePnt, b, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY_INV, max(s.brx-s.tlx,s.bry-s.tly), 20);
    cv::imshow("adp thresh", b);
    cv::waitKey();
    cv::Mat ele = (cv::Mat_<float>(3,3) << 1, 1, 1, 1, 1, 1, 1, 1, 1);
    cv::dilate(b, b, ele);
    vector<cv::Point> ps;
    for (int r=s.tly; r<=s.bry; r++)
        for (int c=s.tlx; c<=s.brx; c++)
        {
            if (b.at<unsigned char>(r,c) >0)
            {
                //startX=c;
                //startY=r;
                ps.push_back(cv::Point(c,r));
                r= s.bry+1;
                break;
            }
        }
    if (ps.size()>0)
    {
        int minX = ps[0].x;
        int maxX = ps[0].x;
        int minY = ps[0].y;
        int maxY = ps[0].y;
        b.at<unsigned char>(ps[0])=0;
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
        *tlx=min(s.tlx,minX);
        *tly=min(s.tly,minY);
        *brx=max(s.brx,maxX);
        *bry=max(s.bry,maxY);
    }
    else
    {   //no word?
        *tlx=s.tlx;
        *tly=s.tly;
        *brx=s.brx;
        *bry=s.bry;
    }
}
