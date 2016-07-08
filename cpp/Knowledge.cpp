#include "Knowledge.h"

vector< cv::Vec3f > TranscribeBatch::colors = {cv::Vec3f(1.15,1.15,0.85),cv::Vec3f(1.15,0.85,1.15),cv::Vec3f(0.86,0.96,1.2),cv::Vec3f(0.87,1.2,0.87),cv::Vec3f(1.2,0.87,0.87),cv::Vec3f(0.87,0.87,1.2)};
cv::Vec3f TranscribeBatch::wordHighlight(0.9,1.2,1.2);
atomic_ulong TranscribeBatch::_id;//I'm assuming 0 is the default value
int Knowledge::Page::_id=0;

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

vector<TranscribeBatch*> Knowledge::Corpus::addSpotting(Spotting s,vector<Spotting>* newExemplars)
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
    
    addSpottingToPage(s,page,ret,newExemplars);
    
    cout <<"END addSpotting"<<endl;
    return ret;
}
vector<TranscribeBatch*> Knowledge::Corpus::updateSpottings(vector<Spotting>* spottings, vector<pair<unsigned long,string> >* removeSpottings, vector<unsigned long>* toRemoveBatches, vector<Spotting>* newExemplars)
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
            vector<Word*> words = wordsForIds[sid.first];
            for (Word* word : words)
            {
                
                unsigned long retractId=0;  
                TranscribeBatch* newBatch = word->removeSpotting(sid.first,&retractId);
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


void Knowledge::Corpus::addSpottingToPage(Spotting& s, Page* page, vector<TranscribeBatch*>& ret, vector<Spotting>* newExemplars)
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
                        newBatch = word->addSpotting(s,newExemplars);
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
        page->addLine(s,newExemplars);
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

TranscribeBatch* Knowledge::Word::addSpotting(Spotting s, vector<Spotting>* newExemplars)
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
        spottings.insert(make_pair(s.tlx,s));//multimap, so they are properly ordered
    }
    TranscribeBatch* ret=queryForBatch(newExemplars);//This has an id matching the sent batch (if it exists)
    pthread_rwlock_unlock(&lock);
    return ret;
}

TranscribeBatch* Knowledge::Word::queryForBatch(vector<Spotting>* newExemplars)
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
                if (newExemplars!=NULL)
                {
                    vector<Spotting> *newE = harvest();
                    newExemplars->insert(newExemplars->end(),newE->begin(),newE->end());
                    delete newE;
                }
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
        ret=queryForBatch(NULL);
    pthread_rwlock_unlock(&lock);
    return ret;
}

multimap<float,string> Knowledge::Word::scoreAndThresh(vector<string> match)//, float thresh)
{
    //TODO, perhaps find a good thresh point?
    multimap<float,string> ret;
    for (string m : match)
        ret.insert(make_pair(0.1,m));
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

vector<Spotting>* Knowledge::Word::harvest()
{
#ifdef TEST_MODE
    cout<<"harvesting: "<<transcription<<endl;
#endif
    vector<Spotting>* ret = new vector<Spotting>();
    string unspotted = transcription;
    multimap<int,Spotting> spottingsByIndex;
    int curI =0;
    for (auto p : spottings)
    {
        for (; curI<transcription.length(); curI++)
        {
            if (transcription.substr(curI,p.second.ngram.length()).compare(p.second.ngram)==0)
            {
                spottingsByIndex.insert(make_pair(curI,p.second));
                for (int i=curI; i<curI+p.second.ngram.length(); i++)
                    unspotted[i]='$';
#ifdef TEST_MODE
                cout <<"["<<curI<<"]:"<<p.second.ngram<<", ";
#endif
                break;
            }
        }
    }
#ifdef TEST_MODE
    cout<<endl<<"unspotted: "<<unspotted<<endl;;
#endif
    for (int i=0; i< 1+transcription.length()-MIN_N; i++)
    {
        for (int n=MIN_N; n<=MAX_N; n++)
        {
            //if we have anchors on either side
            if ((i==0|| (unspotted[i-1]=='$' || unspotted[i]=='$')) && (i==transcription.length()-n || (i<transcription.length()-n && (unspotted[i+n]=='$' || unspotted[i+n-1]=='$'))))
            {
                bool isNew=true;
                string ngram = transcription.substr(i,n);
                /*for (int offset=0; offset<n; offset++)
                {
                    if (unspotted[i+offset]!='$')
                    {
                        isNew=true;
                        break;
                    }
                }*/
                auto startAndEnd = spottingsByIndex.equal_range(i);
                for (auto iter=startAndEnd.first; iter!=startAndEnd.second; iter++)
                {
#ifdef TEST_MODE
                        cout<<"comparing, ["<<ngram<<"] to ["<<iter->second.ngram<<"] at "<<i<<endl;
#endif
                    if (iter->second.ngram.compare(ngram)==0)
                    {
                        isNew=false;
#ifdef TEST_MODE
                        cout<<"Nope, ["<<ngram<<"] already there. "<<i<<endl;
#endif
                        break;
                    }
                }
                if (isNew)
                {
#ifdef TEST_MODE
                    cout<<"New ngram: "<<ngram<<", at "<<i<<endl;
#endif
                    int rank = Global::knowledge()->getNgramRank(ngram);
                    if ( rank<=MAX_NGRAM_RANK )
                    {
                        int etlx, ebrx;
                        //getting left x location
                        int lowerBound=0;
                        int upperBound=0;
                        int bc=0;
                        //start from first char of new exemplar, working backwards
                        for (int si=i; si>=0; si--)
                        {
                            startAndEnd = spottingsByIndex.equal_range(si);
                            for (auto iter=startAndEnd.first; iter!=startAndEnd.second; iter++)
                            {
                                if (iter->second.ngram.length()+si==i)
                                {
                                    assert(iter->second.ngram[iter->second.ngram.length()-1]==transcription[i-1]);
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
                                        assert( transcription[i+oo]==ngram[oo] );
                                    int per = (iter->second.brx-iter->second.tlx)/iter->second.ngram.length();
                                    int loc = per * (iter->second.ngram.length()-numOverlap) + iter->second.brx;
                                    lowerBound += loc - (iter->second.brx-iter->second.tlx)/5;
                                    upperBound += loc + (iter->second.brx-iter->second.tlx)/5;
                                    bc++;
                                }
                            }
                        }
                        if (bc>0)
                        {
                            lowerBound/=bc;
                            upperBound/=bc;
                            etlx = getBreakPoint(lowerBound,tly,upperBound,bry,pagePnt);
                        }
                        else
                        {
                            assert(i==0);
                            etlx=tlx;
                        }
                        //getting right x location
                        lowerBound=0;
                        upperBound=0;
                        bc=0;
                        //search from one char into the new exemplar, working forwards
                        for (int si=i+1; si<=i+n; si++)
                        {
                            auto startAndEnd = spottingsByIndex.equal_range(si);
                            for (auto iter=startAndEnd.first; iter!=startAndEnd.second; iter++)
                            {
                                if (si==i+n)
                                {
                                    assert(iter->second.ngram[0]==transcription[i+n]);
                                    //bounds from front
                                    lowerBound += iter->second.tlx - (iter->second.brx-iter->second.tlx)/8;
                                    upperBound += iter->second.tlx + (iter->second.brx-iter->second.tlx)/8;
                                    bc++;
                                }
                                else if (si<i+n)
                                {
                                    //bounds from middle
                                    int numTo = (i+n)-si;
                                    int per = (iter->second.brx-iter->second.tlx)/iter->second.ngram.length();
                                    int loc = per * (numTo) + iter->second.tlx;
                                    lowerBound += loc - (iter->second.brx-iter->second.tlx)/5;
                                    upperBound += loc + (iter->second.brx-iter->second.tlx)/5;
                                    bc++;
                                }
                            }
                        }
                        if (bc>0)
                        {
                            lowerBound/=bc;
                            upperBound/=bc;
                            ebrx = getBreakPoint(lowerBound,tly,upperBound,bry,pagePnt);
                        }
                        else
                        {
                            assert(i==transcription.length()-n);
                            ebrx=brx;
                        }

                        Spotting toRet(etlx,tly,ebrx,bry,pageId,pagePnt,ngram,0.0);
                        toRet.type=SPOTTING_TYPE_EXEMPLAR;
                        toRet.ngramRank=rank;
                        //toRet.setHarvested();
                        ret->push_back(toRet);
                        //ret.emplace(ngram,(*pagePnt)(cv::Rect(etlx,etly,ebrx-etlx+1,bry-tly+1));
#ifdef TEST_MODE
                        cout <<"harvested: "<<ngram<<endl;
                        cv::imshow("harvested",toRet.img());
                        cv::waitKey();
#endif
                    }
                }
            }
        }
    }
    return ret;

}


int Knowledge::getBreakPoint(int lxBound, int ty, int rxBound, int by, const cv::Mat* pagePnt)
{
    int retX;
    cv::Mat b;// = otsu(*t.pagePnt);
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
    orig=orig.clone();
    for (int r=0; r<orig.rows; r++)
        orig.at<unsigned char>(r,retX)=0;
    cv::imshow("break point",orig);
    cv::waitKey();
#endif
    /*GraphType *g = new GraphType(width*height, 4*(width-1)*(height-1)-(height+width));

    for (int i=0; i<width*height; i++)
    {
        g->add_node();
    }

                if (img.pixel(i,j))
                {
                    int index = i+width*j;
                    g -> add_tweights(index, anchor_weight,0);//invDistMap[index], 0);
                    count_source--;
                }
    for (int i=0; i<width; i++)
    {
        for (int j=0; j<height; j++)
        {
            if (i+1<width)
            {
                setEdge(i,j,i+1,j,g,img,invDistMap,BLACK_TO_BLACK_H_BIAS,WHITE_TO_BLACK_BIAS,BLACK_TO_WHITE_BIAS,WHITE_TO_WHITE_H_BIAS,reducer,width);
            }

            if (j+1<height)
            {
                setEdge(i,j,i,j+1,g,img,invDistMap,BLACK_TO_BLACK_V_BIAS,WHITE_TO_BLACK_BIAS,BLACK_TO_WHITE_BIAS,WHITE_TO_WHITE_V_BIAS,reducer,width);
            }

            if (j>0 && i<width-1)
            {
                setEdge(i,j,i+1,j-1,g,img,invDistMap,BLACK_TO_BLACK_D_BIAS,WHITE_TO_BLACK_BIAS,BLACK_TO_WHITE_BIAS,WHITE_TO_WHITE_D_BIAS,reducer,width);
            }

            if (j<height-1 && i<width-1)
            {
                setEdge(i,j,i+1,j+1,g,img,invDistMap,BLACK_TO_BLACK_D_BIAS,WHITE_TO_BLACK_BIAS,BLACK_TO_WHITE_BIAS,WHITE_TO_WHITE_D_BIAS,reducer,width);
            }
        }
    }
    int ret = g -> maxflow();
    for (int index=0; index<width*height; index++)
    {
        if (img.pixelIsMine(index%width,index/width))
        {
            if (g->what_segment(index) == GraphType::SOURCE)
            {
            }
        }
    }*/

    return retX+lxBound;
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

