#include "Knowledge.h"

int Knowledge::Page::_id=0;

Knowledge::Corpus::Corpus()
{
    pthread_rwlock_init(&pagesLock,NULL);
    pthread_rwlock_init(&spottingsMapLock,NULL);
    averageCharWidth=30;
    countCharWidth=0;
    threshScoring= 1.0;
}
vector<TranscribeBatch*> Knowledge::Corpus::addSpotting(Spotting s,vector<Spotting*>* newExemplars)
{
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
                TranscribeBatch* newBatch = word->removeSpotting(sid.first,&retractId,newExemplars,toRemoveExemplars);
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

TranscribeBatch* Knowledge::Word::addSpotting(Spotting s, vector<Spotting*>* newExemplars)
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
                //cout <<"merge"<<endl;
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

TranscribeBatch* Knowledge::Word::queryForBatch(vector<Spotting*>* newExemplars)
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
                    vector<Spotting*> newE = harvest();
                    newExemplars->insert(newExemplars->end(),newE.begin(),newE.end());
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
TranscribeBatch* Knowledge::Word::removeSpotting(unsigned long sid, unsigned long* sentBatchId, vector<Spotting*>* newExemplars, vector< pair<unsigned long, string> >* toRemoveExemplars)
{
    pthread_rwlock_wrlock(&lock);
    if (sentBatchId!=NULL)
        *sentBatchId = this->sentBatchId;

    toRemoveExemplars->insert(toRemoveExemplars->end(),harvested.begin(),harvested.end());
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
        ret=queryForBatch(newExemplars);
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
        float numChars = dif/(0.0+*averageCharWidth);
        //cout <<"pos: "<<pos<<" str: "<<spot->second.tlx<<endl;
        //cout <<"num chars: "<<numChars<<endl;
        
        
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
    float numChars = dif/(0.0+*averageCharWidth);
    //cout <<"pos: "<<pos<<" end: "<<brx<<endl;
    //cout <<"E num chars: "<<numChars<<endl;
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

void Knowledge::Word::getWordImgAndBin(cv::Mat& wordImg, cv::Mat& b)
{
    //cv::Mat b;
    int blockSize = (1+bry-tly)/2;
    if (blockSize%2==0)
        blockSize++;
    wordImg = (*pagePnt)(cv::Rect(tlx,tly,brx-tlx+1,bry-tly+1));
    if (wordImg.type()==CV_8UC3)
        cv::cvtColor(wordImg,wordImg,CV_RGB2GRAY);
    cv::adaptiveThreshold(wordImg, b, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY_INV, blockSize, 10);
}

vector<Spotting*> Knowledge::Word::harvest()
{
#ifdef TEST_MODE
    cout<<"harvesting: "<<transcription<<endl;
#endif
    cv::Mat wordImg, b;
    vector<Spotting*> ret;
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
                    int rank = GlobalK::knowledge()->getNgramRank(ngram);
                    if ( rank<=MAX_NGRAM_RANK )
                    {
                        //getting left x location
                        int leftLeftBound =0;
                        int rightLeftBound=0;
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
                                    leftLeftBound  += iter->second.brx - (iter->second.brx-iter->second.tlx)/8;
                                    rightLeftBound += iter->second.brx + (iter->second.brx-iter->second.tlx)/8;
                                    bc++;
                                }
                                else if (iter->second.ngram.length()+si>i)
                                {
                                    //bounds from middle
                                    int numOverlap = iter->second.ngram.length()+si-i;
                                    for (int oo=0; oo<numOverlap; oo++)
                                        assert( transcription[i+oo]==ngram[oo] );
                                    int per = (iter->second.brx-iter->second.tlx)/iter->second.ngram.length();
                                    int loc = per * (iter->second.ngram.length()-numOverlap) + iter->second.tlx;
                                    leftLeftBound  += loc - (iter->second.brx-iter->second.tlx)/5;
                                    rightLeftBound += loc + (iter->second.brx-iter->second.tlx)/5;
                                    bc++;
                                }
                            }
                        }
                        if (bc>0)
                        {
                            leftLeftBound /=bc;
                            rightLeftBound/=bc;
                            //etlx = getBreakPoint(lowerBound,tly,upperBound,bry,pagePnt);
                        }
                        else
                        {
                            assert(i==0);
                            //etlx=tlx;
                            leftLeftBound=tlx; 
                            rightLeftBound=tlx; 
                        }
                        //getting right x location
                        int leftRightBound =0;
                        int rightRightBound=0;
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
                                    leftRightBound  += iter->second.tlx - (iter->second.brx-iter->second.tlx)/7;
                                    rightRightBound += iter->second.tlx + (iter->second.brx-iter->second.tlx)/7;
                                    bc++;
                                }
                                else if (si<i+n)
                                {
                                    //bounds from middle
                                    int numTo = (i+n)-si;
                                    int per = (iter->second.brx-iter->second.tlx)/iter->second.ngram.length();
                                    int loc = per * (numTo) + iter->second.tlx;
                                    leftRightBound += loc - (iter->second.brx-iter->second.tlx)/5.5;
                                    rightRightBound += loc + (iter->second.brx-iter->second.tlx)/5.5;
                                    //int nloc = getBreakPoint(leftRightBound,tly,rightRightBound,bry,pagePnt);
                                    //leftRightBound += (nloc-loc)/2;
                                    //rightRightBound += (nloc-loc)/2;
                                    bc++;
                                }
                            }
                        }
                        if (bc>0)
                        {
                            leftRightBound/=bc;
                            rightRightBound/=bc;
                            //ebrx = getBreakPoint(lowerBound,tly,upperBound,bry,pagePnt);
                        }
                        else
                        {
                            assert(i==transcription.length()-n);
                            //ebrx=brx;
                            leftRightBound=brx;
                            rightRightBound=brx;
                        }
                        if (rightLeftBound >= leftRightBound)
                        {
                            int mid=(rightLeftBound+leftRightBound)/2;
                            rightLeftBound=mid-2;
                            leftRightBound=mid+2;
                        }
                        if (wordImg.cols==0)
                            getWordImgAndBin(wordImg,b);
                        SpottingExemplar* toRet = extractExemplar(leftLeftBound,rightLeftBound,leftRightBound,rightRightBound,ngram,wordImg,b);
                        //Spotting toRet(etlx,tly,ebrx,bry,pageId,pagePnt,ngram,0.0);
                        //toRet.type=SPOTTING_TYPE_EXEMPLAR;
                        toRet->ngramRank=rank;
                        //toRet.setHarvested();
                        ret.push_back(toRet);
                        //ret.emplace(ngram,(*pagePnt)(cv::Rect(etlx,etly,ebrx-etlx+1,bry-tly+1));
#ifdef TEST_MODE
                        cout <<"EXAMINE harvested: "<<ngram<<endl;
                        cv::imshow("harvested",toRet->ngramImg());
                        cv::imshow("boundary image",toRet->img());
#ifdef TEST_MODE_LONG
                        cv::waitKey(100);
#else
                        cv::waitKey();
#endif
#endif
                    }
                }
            }
        }
    }
    
    harvested.clear();
    for (Spotting* s : ret)
        harvested.insert(make_pair(s->id,s->ngram));
    /*
    //Also harvest char width
    if (wordImg.cols==0)
        getWordImgAndBin(wordImg,b);
    vector<int> vHist(wordImg.cols);
    map<int,int> hist;
    int maxV=0;
    int total=0;
    for (int c=0; c<wordImg.cols; c++) {
        for (int r=0; r<wordImg.rows; r++)
            vHist[c]+=wordImg.at<unsigned char>(r,c);
        hist[vHist[c]]+=1;
        total++;
        if (vHist[c]>maxV)
            maxV=vHist[c];
    }

    
    //sparse otsu
    double sum =0;
    for (int i = 1; i <= maxV; ++i) {
        if (hist.find(i)!=hist.end())
            sum += i * hist[i];
    }
    double sumB = 0;
    double wB = 0;
    double wF = 0;
    double mB;
    double mF;
    double max = 0.0;
    double between = 0.0;
    double threshold1 = 0.0;
    double threshold2 = 0.0;
    for (int i = 0; i < maxV; ++i)
    {
        if (hist.find(i)!=hist.end())
            wB += hist[i];
        if (wB == 0)
            continue;
        wF = total - wB;
        if (wF == 0)
            break;
        if (hist.find(i)!=hist.end())
            sumB += i * hist[i];
        mB = sumB / (wB*1.0);
        mF = (sum - sumB) / (wF*1.0);
        between = wB * wF * pow(mB - mF, 2);
        if ( between >= max )
        {
            threshold1 = i;
            if ( between > max )
            {
                threshold2 = i;
            }
            max = between; 
        }
    }
    
    double thresh = ( threshold1 + threshold2 ) / 2.0;
    int newLeft, newRight;
    for (newLeft=0; newLeft<vHist.size(); newLeft++)
        if (vHist[newLeft]>thresh)
            break;
    newLeft--;
    for (newRight=vHist.size()-1; newRight>=0; newRight--)
        if (vHist[newRight]>thresh)
            break;
    newRight++;
    float charWidth = (newRight-newLeft+0.0f)/transcription.length();
    *countCharWidth++;
    *averageCharWidth = ((*countCharWidth-1.0)/(*countCharWidth))*(*averageCharWidth) + (1.0/(*countCharWidth))*charWidth;
#ifdef TEST_MODE
    cout <<"harvested char width: "<<charWidth<<endl;
    cout <<"averageCharWidth now: "<<*averageCharWidth<<endl;
#endif
    */
    return ret;

}

inline void setEdge(int x1, int y1, int x2, int y2, GraphType* g, const cv::Mat &img)
{
    float w = ((255-img.at<unsigned char>(y1,x1))+(255-img.at<unsigned char>(y2,x2)))/(255.0+255.0);
    //w = 1/(1+exp(-2*w));
    w = w*w*w;
    g -> add_edge(x1+y1*img.cols, x2+y2*img.cols,w,w);
    //cout << w <<endl;
}

#ifdef TEST_MODE
void Knowledge::Word::emergencyAnchor(cv::Mat& b, GraphType* g,int startX, int endX, float sum_anchor, float goal_sum, bool word, cv::Mat& showA)
#else
void Knowledge::Word::emergencyAnchor(cv::Mat& b, GraphType* g,int startX, int endX, float sum_anchor, float goal_sum, bool word)
#endif
{
    float baselineH = (botBaseline-topBaseline)/2.0;
    int c=startX;
    float inc=.5;
    float addStr=1;
    //float init = sum_anchor;
    //float sum_anchor=0;
    while (sum_anchor < goal_sum)
    {

        for (int r=topBaseline+1; r<botBaseline; r++)
        {
            float str= (baselineH-fabs((r-topBaseline)-baselineH))/baselineH;
            if (b.at<unsigned char>(wordCord(r,c)))
            {
                g -> add_tweights(wordIndex(r,c), ANCHOR_CONST*addStr*(word?1:0),ANCHOR_CONST*addStr*(word?0:1));
                sum_anchor+=addStr;
#ifdef TEST_MODE
                showA.at<cv::Vec3b>(wordCord(r,c))[0]=(word?1:0)*max(.40*ANCHOR_CONST*addStr,255.0);
                showA.at<cv::Vec3b>(wordCord(r,c))[1]=(word?0:1)*max(.40*ANCHOR_CONST*addStr,255.0);
                showA.at<cv::Vec3b>(wordCord(r,c))[2]=0;
#endif
            }
            //else
            //    g -> add_tweights(wordIndex(r,c), 0,NGRAM_GRAPH_BIAS);
        }
        if (endX>startX)
        {
            if (++c > endX) 
            {
                c=startX;
                endX+=2;
                //addStr+=inc;
            }
        }
        else
        {
            if (--c < endX) 
            {
                c=startX;
                endX-=2;
                //addStr+=inc;
            }
        }
    }
}

SpottingExemplar* Knowledge::Word::extractExemplar(int leftLeftBound, int rightLeftBound, int leftRightBound, int rightRightBound, string newNgram, cv::Mat& wordImg, cv::Mat& b)
{
    assert(leftLeftBound<=rightLeftBound && rightLeftBound<leftRightBound && leftRightBound<=rightRightBound);

    if (topBaseline==-1 || botBaseline==-1)
        findBaselines(wordImg,b);

    int width = (brx+1)-tlx;
    int height = (bry+1)-tly;

    GraphType *g = new GraphType(width*height, 2*(width)*(height)-(height+width));

    for (int i=0; i<width*height; i++)
    {
        g->add_node();
    }
#ifdef TEST_MODE
    cv::Mat showA;
    cv::cvtColor(wordImg,showA,CV_GRAY2RGB);
#endif    

    //Add anchors
    float baselineH = (botBaseline-topBaseline)/2.0;
    float sum_anchor_wordFront=0;
    float sum_anchor_wordBack=0;
    float sum_anchor_ngram=0;
    for (int c=tlx; c<=brx; c++)
    {
        float anchor_word=0;
        float anchor_ngram=0;
        if (c<leftLeftBound || c>rightRightBound)
        {
            anchor_word=ANCHOR_CONST;
            /*for (int r=tly; r<topBaseline; r++)
                if (b.at<unsigned char>(wordCord(r,c)))
                    g -> add_tweights(wordIndex(r,c), NGRAM_GRAPH_BIAS,0);
            for (int r=botBaseline+1; r<bry; r++)
                if (b.at<unsigned char>(wordCord(r,c)))
                    g -> add_tweights(wordIndex(r,c), NGRAM_GRAPH_BIAS,0);*/
        }
        else if (c> rightLeftBound && c<leftRightBound)
        {
            anchor_ngram=ANCHOR_CONST;
        }
        /*else if (c>=leftLeftBound && c<leftLeftBound+0.3*(rightLeftBound-leftLeftBound))
        {
            anchor_word = ((0.3*(rightLeftBound-leftLeftBound))-(c-leftLeftBound))/(0.3*(rightLeftBound-leftLeftBound));
        }
        else if (c<=rightRightBound && c>rightRightBound-0.3*(rightRightBound-leftRightBound))
        {
            anchor_ngram = ((0.3*(rightRightBound-leftRightBound))-(rightRightBound-c))/(0.3*(rightRightBound-leftRightBound));
        }*/
        for (int r=topBaseline+1; r<botBaseline; r++)
        {
            float str= (baselineH-fabs((r-topBaseline)-baselineH))/baselineH;
            if (b.at<unsigned char>(wordCord(r,c)))
            {
                g -> add_tweights(wordIndex(r,c), anchor_word*str,anchor_ngram*str);
                if (c<leftLeftBound)
                    sum_anchor_wordFront+=str;
                else if (c>rightRightBound)
                    sum_anchor_wordBack+=str;
                else if (c> rightLeftBound && c<leftRightBound)
                    sum_anchor_ngram+=str;
#ifdef TEST_MODE
                showA.at<cv::Vec3b>(wordCord(r,c))[0]=.40*anchor_word*str;
                showA.at<cv::Vec3b>(wordCord(r,c))[1]=.40*anchor_ngram*str;
                showA.at<cv::Vec3b>(wordCord(r,c))[2]=0;
#endif
            }
            //else
            //    g -> add_tweights(wordIndex(r,c), 0,NGRAM_GRAPH_BIAS);
        }
        /*for (int r=tly; r<topBaseline; r++)
            if (!b.at<unsigned char>(wordCord(r,c)))
                g -> add_tweights(wordIndex(r,c), 0,NGRAM_GRAPH_BIAS);
        for (int r=botBaseline+1; r<bry; r++)
            if (!b.at<unsigned char>(wordCord(r,c)))
                g -> add_tweights(wordIndex(r,c), 0,NGRAM_GRAPH_BIAS);*/
    }

    //Ensure some anchor gets placed
    if (leftLeftBound!=rightLeftBound &&sum_anchor_wordFront < min(sum_anchor_wordBack,sum_anchor_ngram)/2)
    {
        emergencyAnchor(b,g,tlx, leftLeftBound, sum_anchor_wordFront, min(sum_anchor_wordBack,sum_anchor_ngram)/2, true
#if TEST_MODE
               , showA
#endif
               );
    }
    if (leftRightBound!=rightRightBound && sum_anchor_wordBack < min(sum_anchor_wordFront,sum_anchor_ngram)/2)
    {
        emergencyAnchor(b,g,brx, rightRightBound, sum_anchor_wordBack, min(sum_anchor_wordFront,sum_anchor_ngram)/2, true
#if TEST_MODE
               , showA
#endif
               );
    }
    if (sum_anchor_ngram < min(sum_anchor_wordBack,sum_anchor_wordFront)/2)
    {
        emergencyAnchor(b,g,rightLeftBound, leftRightBound, sum_anchor_ngram, min(sum_anchor_wordBack,sum_anchor_wordFront)/2, false
#if TEST_MODE
               , showA
#endif
               );
    }



#ifdef TEST_MODE
    //cv::resize(showA,showA,cv::Size(),2,2,cv::INTER_NEAREST);
    cv::imshow("anchors",showA);
#ifdef TEST_MODE_LONG
    //if (newNgram.compare("ex")==0)
    //{
    //    cv::waitKey();
    //}
#endif
#endif

    //set up graph
    for (int i=0; i<width; i++)
    {
        for (int j=0; j<height; j++)
        {
            if (i+1<width)
            {
                setEdge(i,j,i+1,j,g,wordImg);
            }

            if (j+1<height)
            {
                setEdge(i,j,i,j+1,g,wordImg);
            }

            /*if (j>0 && i<width-1)
            {
                setEdge(i,j,i+1,j-1,g,wordImg);
            }

            if (j<height-1 && i<width-1)
            {
                setEdge(i,j,i+1,j+1,g,wordImg);
            }*/
        }
    }
    g -> maxflow();
    
    cv::Mat mask(b.rows,b.cols,CV_8U);
    int etlx=99999;
    int etly=99999;
    int ebrx=-99999;
    int ebry=-99999;

    for (int i=0; i<width; i++)
    {
        for (int j=0; j<height; j++)
        {
            int index=i+j*width;
            if (g->what_segment(index) == GraphType::SOURCE)
            {
                mask.at<unsigned char>(j,i) = 0;
            }
            else
            {
                mask.at<unsigned char>(j,i) = 1;
                if (b.at<unsigned char>(j,i))
                {
                    if (i<etlx)
                        etlx=i;
                    if (j<etly)
                        etly=j;
                    if (i>ebrx)
                        ebrx=i;
                    if (j>ebry)
                        ebry=j;
        
                }
            }

        }
    }
    //pad
    int pad=2;
    if (etlx-pad>=0)
        etlx-=pad;
    if (etly-pad>=0)
        etly-=pad;
    if (ebrx+pad<width)
        ebrx+=pad;
    if (ebry+pad<height)
        ebry+=pad;
    
    cv::Mat exe = inpainting(wordImg(cv::Rect(etlx,etly,1+ebrx-etlx,1+ebry-etly)),mask(cv::Rect(etlx,etly,1+ebrx-etlx,1+ebry-etly)));
    SpottingExemplar* ret = new SpottingExemplar(tlx+etlx,tly+etly,tlx+ebrx,tly+ebry,pageId,pagePnt,newNgram,NAN,exe);
#ifdef TEST_MODE
    cv::Mat show;
    cv::cvtColor(wordImg,show,CV_GRAY2RGB);
    
    
    for (int i=0; i<width; i++)
    {
        for (int j=0; j<height; j++)
        {
            int index=i+j*width;
            if (g->what_segment(index) == GraphType::SOURCE)
            {
                show.at<cv::Vec3b>(j,i)[0] = 0;
                show.at<cv::Vec3b>(j,i)[1] = 255;
            }
            else
            {
                show.at<cv::Vec3b>(j,i)[1] = 0;
                show.at<cv::Vec3b>(j,i)[0] = 255;
            }
        }
    }
    cv::imshow("seg results",show);
    //cv::imshow("exe",ret->ngramImg());
    //cv::imshow("fromPage",ret->img());
    //cv::waitKey();
#endif
    delete g;
    return ret;
}

void Knowledge::Word::getBaselines(int* top, int* bot)
{
    if (topBaseline<0 || botBaseline<0)
    {
        
        cv::Mat wordImg, b;
        getWordImgAndBin(wordImg,b);
        findBaselines(wordImg,b);
    }
    *top=topBaseline;
    *bot=botBaseline;
}

void Knowledge::Word::findBaselines(const cv::Mat& gray, const cv::Mat& bin)
{
    
    //top and botBaseline should have page cords.
    int avgWhite=0;
    int countWhite=0;
    cv::Mat hist = cv::Mat::zeros(gray.rows,1,CV_32F);
    map<int,int> topPixCounts, botPixCounts;
    for (int c=0; c<gray.cols; c++)
    {
        int topPix=-1;
        int lastSeen=-1;
        for (int r=0; r<gray.rows; r++)
        {
            if (bin.at<unsigned char>(r,c))
            {
                if (topPix==-1)
                {
                    topPix=r;
            }
                lastSeen=r;
            }
            else
            {
                avgWhite+=gray.at<unsigned char>(r,c);
                countWhite++;
            }
            hist.at<float>(r,0)+=gray.at<unsigned char>(r,c);
        }
        if (topPix!=-1)
            topPixCounts[topPix]++;
        if (lastSeen!=-1)
            botPixCounts[lastSeen]++;
    }
    avgWhite /= countWhite;

    int maxTop=-1;
    int maxTopCount=-1;
    for (auto c : topPixCounts)
    {
        if (c.second > maxTopCount)
        {
            maxTopCount=c.second;
            maxTop=c.first;
        }
    }
    int maxBot=-1;
    int maxBotCount=-1;
    for (auto c : botPixCounts)
    {
        if (c.second > maxBotCount)
        {
            maxBotCount=c.second;
            maxBot=c.first;
        }
    }

    //cv::Mat kernel = cv::Mat::ones( 5, 1, CV_32F )/ (float)(5);
    //cv::filter2D(hist, hist, -1 , kernel );
    cv::Mat edges;
    int pad=5;
    cv::Mat paddedHist = cv::Mat::ones(hist.rows+2*pad,1,hist.type());
    double avg=0;
    double maxHist=-99999;
    double minHist=99999;
    for (int r=0; r<hist.rows; r++)
    {
        avg+=hist.at<float>(r,0);
        if (hist.at<float>(r,0)>maxHist)
            maxHist=hist.at<float>(r,0);
        if (hist.at<float>(r,0)<minHist)
            minHist=hist.at<float>(r,0);
    }
    avg/=hist.rows;
    paddedHist *= avg;
    hist.copyTo(paddedHist(cv::Rect(0,pad,1,hist.rows)));
    float kernelData[11] = {1,1,1,1,.5,0,-.5,-1,-1,-1,-1};
    cv::Mat kernel = cv::Mat(11,1,CV_32F,kernelData);
    cv::filter2D(paddedHist, edges, -1 , kernel );//, Point(-1,-1), 0 ,BORDER_AVERAGE);
    float kernelData2[11] = {.1,.1,.1,.1,.1,.1,.1,.1,.1,.1,.1};
    cv::Mat kernel2 = cv::Mat(11,1,CV_32F,kernelData2);
    cv::Mat blurred;
    cv::filter2D(hist, blurred, -1 , kernel2 );//, Point(-1,-1), 0 ,BORDER_AVERAGE);
    topBaseline=-1;
    float maxEdge=-9999999;
    botBaseline=-1;
    float minEdge=9999999;
    float minPeak=9999999;
    float center=-1;
    for (int r=pad; r<gray.rows+pad; r++)
    {
        float v = edges.at<float>(r,0);
        if (v>maxEdge)
        {
            maxEdge=v;
            topBaseline=r-pad;
        }
        if (v<minEdge)
        {
            minEdge=v;
            botBaseline=r-pad;
        }

        if (blurred.at<float>(r-pad,0) < minPeak) {
            center=r-pad;
            minPeak=blurred.at<float>(r-pad,0);
        }
    }
    if (topBaseline>center)
    {
        if (maxTop < center)
            topBaseline=maxTop;
        else
        {
            maxEdge=-999999;
            for (int r=pad; r<center+pad; r++)
            {
                float v = edges.at<float>(r,0);
                if (v>maxEdge)
                {
                    maxEdge=v;
                    topBaseline=r-pad;
                }
            }
        }
    }
    if (botBaseline<center)
    {
        if (maxBot > center)
            botBaseline=maxBot;
        else
        {
            minEdge=999999;
            for (int r=center+1; r<gray.rows+pad; r++)
            {
                float v = edges.at<float>(r,0);
                if (v<minEdge)
                {
                    minEdge=v;
                    botBaseline=r-pad;
                }
            }
        }
    }
    if (botBaseline < topBaseline)//If they fail this drastically, the others won't be much better.
    {
        topBaseline=maxTop;
        botBaseline=maxBot;
    }
    topBaseline += tly;
    botBaseline += tly;
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

    return retX+lxBound;
}
/*void Knowledge::computeInverseDistanceMap(const cv::Mat &src, int* out)
{
    int maxDist=0;
    int g[src.cols*src.rows];
    for (int x=0; x<src.cols; x++)
    {
        if (src.pixel(x,0))
        {
            g[x+0*src.cols]=0;
        }
        else
        {
            g[x+0*src.cols]=INT_POS_INFINITY;//src.cols*src.rows;
        }
        
        for (int y=1; y<src.rows; y++)
        {
            if (src.pixel(x,y))
            {
                g[x+y*src.cols]=0;
            }
            else
            {
                if (g[x+(y-1)*src.cols] != INT_POS_INFINITY)
                    g[x+y*src.cols]=1+g[x+(y-1)*src.cols];
                else
                    g[x+y*src.cols] = INT_POS_INFINITY;
            }
        }
        
        for (int y=src.rows-2; y>=0; y--)
        {
            if (g[x+(y+1)*src.cols]<g[x+y*src.cols])
            {
                if (g[x+(y+1)*src.cols] != INT_POS_INFINITY)
                    g[x+y*src.cols]=1+g[x+(y+1)*src.cols];
                else
                    g[x+y*src.cols] = INT_POS_INFINITY;
            }
        }
        
    }
    
    int q;
    int s[src.cols];
    int t[src.cols];
    int w;
    for (int y=0; y<src.rows; y++)
    {
        q=0;
        s[0]=0;
        t[0]=0;
        for (int u=1; u<src.cols;u++)
        {
            while (q>=0 && f(t[q],s[q],y,src.cols,g) > f(t[q],u,y,src.cols,g))
            {
                q--;
            }
            
            if (q<0)
            {
                q=0;
                s[0]=u;
            }
            else
            {
                w = SepPlusOne(s[q],u,y,src.cols,g);
                if (w<src.cols)
                {
                    q++;
                    s[q]=u;
                    t[q]=w;
                }
            }
        }
        
        for (int u=src.cols-1; u>=0; u--)
        {
            out[u+y*src.cols]= f(u,s[q],y,src.cols,g);
            if (out[u+y*src.cols] > maxDist)
                maxDist = out[u+y*src.cols];
            if (u==t[q])
                q--;
        }
    }
    
    
    //    QImage debug(src.cols,src.rows,src.format());
    //    debug.setColorTable(src.colorTable());
    //    for (int i=0; i<debug.width(); i++)
    //    {
    //        for (int j=0; j<debug.height(); j++)
    //            debug.setPixel(i,j,(int)((pow(out[i+j*debug.width()],.2)/((double)pow(maxDist,.2)))*255));
            
    //    }
    //    debug.save("./reg_dist_map.pgm");
    //    printf("image format:%d\n",debug.format());
    
    //invert
//    printf("maxDist=%d\n",maxDist);
    maxDist++;
//    double normalizer = (25.0/maxDist);
    double e = 10;
    double b = 25;
    double m = 2000;
    double a = INV_A;
    int max_cc_size=500;
    
//    double normalizer = (b/m);
    BImage mark = src.makeImage();
    QVector<QPoint> workingStack;
    QVector<QPoint> growingComponent;
    
    
    int newmax = 0;
    int newmax2 = 0;
    int newmin = INT_MAX;
    for (int q = 0; q < src.cols*src.rows; q++)
    {   
        //out[q] = pow(6,24-out[q]*normalizer)/pow(6,20);
        if (src.pixel(q%src.cols,q/src.cols) && mark.pixel(q%src.cols,q/src.cols))
        {
            //fill bias
            QPoint p(q%src.cols,q/src.cols);
            workingStack.push_back(p);
            mark.setPixel(p,false);
            while (!workingStack.isEmpty())
            {   
                QPoint cur = workingStack.back();
                workingStack.pop_back();
                growingComponent.append(cur);
                
                
                
                
                if (cur.x()>0 && mark.pixel(cur.x()-1,cur.y()))
                {
                    QPoint pp(cur.x()-1,cur.y());
                    workingStack.push_back(pp);
                    mark.setPixel(pp,false);
                }
                
                
                if (cur.x()<mark.width()-1 && mark.pixel(cur.x()+1,cur.y()))
                {
                    QPoint pp(cur.x()+1,cur.y());
                    workingStack.push_back(pp);
                    mark.setPixel(pp,false);
                    
                }
                if (cur.y()<mark.height()-1 && mark.pixel(cur.x(),cur.y()+1))
                {
                    QPoint pp(cur.x(),cur.y()+1);
                    workingStack.push_back(pp);
                    mark.setPixel(pp,false);
                }
                if (cur.y()>0 && mark.pixel(cur.x(),cur.y()-1))
                {
                    QPoint pp(cur.x(),cur.y()-1);
                    workingStack.push_back(pp);
                    mark.setPixel(pp,false);
                }
                //diagonals
                if (cur.x()>0 && cur.y()>0 && mark.pixel(cur.x()-1,cur.y()-1))
                {
                    QPoint pp(cur.x()-1,cur.y()-1);
                    workingStack.push_back(pp);
                    mark.setPixel(pp,false);
                }
                
                
                if (cur.x()<mark.width()-1 && cur.y()>0 && mark.pixel(cur.x()+1,cur.y()-1))
                {
                    QPoint pp(cur.x()+1,cur.y()-1);
                    workingStack.push_back(pp);
                    mark.setPixel(pp,false);
                    
                }
                if (cur.x()>0 && cur.y()<mark.height()-1 && mark.pixel(cur.x()-1,cur.y()+1))
                {
                    QPoint pp(cur.x()-1,cur.y()+1);
                    workingStack.push_back(pp);
                    mark.setPixel(pp,false);
                }
                if (cur.x()<mark.width()-1 && cur.y()>0 && mark.pixel(cur.x()+1,cur.y()-1))
                {
                    QPoint pp(cur.x()+1,cur.y()-1);
                    workingStack.push_back(pp);
                    mark.setPixel(pp,false);
                }
            }
            int cc_size = growingComponent.size();
            while (!growingComponent.isEmpty())
            {
                QPoint cur = growingComponent.back();
                growingComponent.pop_back();
                int index = cur.x()+src.cols*cur.y();
                out[index] = pow(b-std::min(out[index]*(b/m),b),e)*a/(pow(b,e)) + std::min(cc_size,max_cc_size) + 1;
                
                if (out[index]>newmax)
                    newmax=out[index];
                
                if (out[index]>newmax2 && out[index]<newmax)
                    newmax2=out[index];
                
                if (out[index]<newmin)
                    newmin=out[index];
            }
        }
        else if (!src.pixel(q%src.cols,q/src.cols))
        {
            out[q] = pow(b-std::min(out[q]*(b/m),b),e)*a/(pow(b,e)) + 1;
        }

        if (out[q]>newmax)
            newmax=out[q];
        if (out[q]>newmax2 && out[q]<newmax)
            newmax2=out[q];
        if (out[q]<newmin)
            newmin=out[q];
    }
    
}*/
cv::Mat Knowledge::inpainting(const cv::Mat& src, const cv::Mat& mask, double* avg, double* std, bool show)
{
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
    cv::waitKey();

    pthread_rwlock_unlock(&pagesLock);
}
void Knowledge::Corpus::showProgress(int height, int width)
{
    //I'll assume all page images are the same dimensions
    int pageH=pages.begin()->second->getImg()->rows;
    int pageW=pages.begin()->second->getImg()->cols;
    float resizeScale=.001;
    for (int scale=0.5; scale>.001; scale-=.001)
    {
        nAcross=floor(width/(pageW*scale));
        nDown=floor(height/(pageH*scale));
        if (nAcross*nDown>=pages.size())
        {
            resizeScale=scale;
            break;
        }
    }
    Mat draw = Mat::zeros(height,width,CV_8UC3);
    pthread_rwlock_rdlock(&pagesLock);
    int xPos=0;
    int yPos=0;
    int across=0;
    for (auto p : pages)
    {
        Page* page = p.second;
        Mat workingIm;
        cvtColor(*(page->getImg()), workingIm, CV_GRAY2RGB);
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
                    for (int x=tlx; x<=brx; x++)
                        for (int y=tly; y<=bry; y++)
                        {
                            workingIm.at<Vec3b>(y,x)[0] = 0.5*workingIm.at<Vec3b>(y,x)[0];
                            workingIm.at<Vec3b>(y,x)[1] = min(255,workingIm.at<Vec3b>(y,x)[1]+120);
                            workingIm.at<Vec3b>(y,x)[2] = 0.5*workingIm.at<Vec3b>(y,x)[2];
                        }
                }
                else
                {
                    for (Spotting& s : word->getSpottings())
                        for (int x=s.tlx; x<=s.brx; x++)
                            for (int y=s.tly; y<=s.bry; y++)
                            {
                                workingIm.at<Vec3b>(y,x)[0] = 0.5*workingIm.at<Vec3b>(y,x)[0];
                                workingIm.at<Vec3b>(y,x)[2] = min(255,workingIm.at<Vec3b>(y,x)[2]+120);
                                workingIm.at<Vec3b>(y,x)[1] = 0.5*workingIm.at<Vec3b>(y,x)[1];
                            }
                }
            }
        }
        resize(workingIm,workingIm,Size(),resizeScle,resizeScale);
        draw(Rect(xPos,yPos,workingIm.cols,workingIm.rows)) = workingIm;
        xPos+=workingIm.cols;
        if (++across >= nAcross)
        {
            xPos=0;
            yPos+=workingIm.rows;
            across=0;
        }
    }
    cv::imshow("progress",draw);
    cv::waitKey(100);

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
            page = new Page(imageLoc+"/"+imageFile,&averageCharWidth,&countCharWidth);
            pages[pageId] = page;
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
    
    double heightAvg=0;
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
#endif

    in.close();
}


const cv::Mat* Knowledge::Corpus::imgForPageId(int pageId) const
{

    const Page* page = pages.at(pageId);
    return page->getImg();
}

