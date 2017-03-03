#include "SpottingResults.h"

#include <ctime>
#include <random>

atomic_ulong Spotting::_id;
atomic_ulong SpottingResults::_id;

SpottingResults::SpottingResults(string ngram, int contextPad) : 
    instancesByScore(scoreCompById(&(this->instancesById))),
    ngram(ngram), contextPad(contextPad)
{
    id = ++_id;
    //sem_init(&mutexSem,false,1);
    //numBatches=0;
    allBatchesSent=true;
    
    numberClassifiedTrue=0;
    numberClassifiedFalse=0;
    numberAccepted=0;
    numberRejected=0;
    maxScore=-999999;
    minScore=999999;
    
    
    trueMean=minScore;//How to initailize?
    trueVariance=1.0;
    falseMean=maxScore;
    falseVariance=1.0;
    
    acceptThreshold=-1;
    rejectThreshold=-1;
    numLeftInRange=-1;
    
    done=false;

    //pullFromScore=splitThreshold;
    momentum=1.2;
    lastDifPullFromScore=0;
    batchesSinceChange=0;
#ifdef GRAPH_SPOTTING_RESULTS
    undoneGraphName="save/graph/graph_undone_"+ngram+".png";
    fullGraphName="save/graph/graph_full_"+ngram+".png";
    //cv::namedWindow(undoneGraphName);
#endif
}

void SpottingResults::add(Spotting spotting) {
    ///debugState();
    assert(spotting.tlx!=-1);
    //sem_wait(&mutexSem);
    allBatchesSent=false;
    instancesById[spotting.id]=spotting;
    assert(spotting.ngram.compare(ngram)==0);
    assert(spotting.score==spotting.score && spotting.pageId<100000);//currption checking
    if (spotting.type==SPOTTING_TYPE_TRANS_TRUE)
    {
        classById[spotting.id]=true;
        numberClassifiedTrue++;
#ifdef NO_NAN
        //GlobalK::knowledge()->accepted();
#endif
    }
    else if (spotting.type==SPOTTING_TYPE_TRANS_FALSE)
    {
        classById[spotting.id]=false;
        numberClassifiedFalse++;
#ifdef NO_NAN
        //GlobalK::knowledge()->rejected();
#endif
    }
    else
    {
        instancesByScore.insert((spotting.id));
    }
    instancesByLocation.insert(&instancesById.at(spotting.id));
    tracer = instancesByScore.begin();
    if (spotting.score>maxScore)
    {
        if (falseMean==maxScore && falseVariance==1.0)
            falseMean=spotting.score;
        maxScore=spotting.score;
    }
    if (spotting.score<minScore)
    {
        if (trueMean==minScore && trueVariance==1.0)
            trueMean=spotting.score;
        minScore=spotting.score;
    }
    //sem_post(&mutexSem);
    batchesSinceChange=0;
    ///debugState();
}
void SpottingResults::addTrueNoScore(const SpottingExemplar& spotting) {
    ///debugState();
    assert(spotting.tlx!=-1);
    //sem_wait(&mutexSem);
    assert(spotting.score != spotting.score);
    instancesById[spotting.id]=spotting;
    instancesById.at(spotting.id).type=SPOTTING_TYPE_APPROVED;
    instancesByLocation.insert(&instancesById.at(spotting.id));
    classById[spotting.id]=true;
    //sem_post(&mutexSem);
    ///debugState();
}
/*SpottingsBatch* SpottingResults::getBatch(bool* done, unsigned int num, unsigned int maxWidth) {
    cout <<"getBatch, from:"<<pullFromScore<<endl;
    if (acceptThreshold==-1 && rejectThreshold==-1)
        EMThresholds(true);
    SpottingsBatch* ret = new SpottingsBatch(ngram,id);
    //sem_wait(&mutexSem);
    unsigned int toRet = ((((signed int)instancesByScore.size())-(signed int) num)>3)?num:instancesByScore.size();
    
    for (unsigned int i=0; i<toRet && !*done; i++) {
        SpottingImage tmp = getNextSpottingImage(done, maxWidth);
        ret->push_back(tmp);
        if (classById.size()<20)
            *done=false;
    }
    if (*done)
        allBatchesSent=true;
    //sem_post(&mutexSem);
    numBatches++;
    cout <<"["<<id<<"] sent batch of size "<<ret->size()<<", have "<<instancesByScore.size()<<" left"<<endl;
    cout <<"score: ";
    for (int i=0; i<ret->size(); i++)
    {
        cout<<ret->at(i).score<<"\t";
    }
    cout<<endl;
    return ret;
}*/

void SpottingResults::debugState() const
{
    for (auto iter=instancesByLocation.begin(); iter!=instancesByLocation.end(); iter++)
    {
        assert(instancesById.find((**iter).id) != instancesById.end());
    }
    for (auto iter=instancesByScore.begin(); iter!=instancesByScore.end(); iter++)
    {
        assert(instancesById.find(*iter) != instancesById.end());
    }
}

#ifdef TEST_MODE
void SpottingResults::setDebugInfo(SpottingsBatch* b)
{
    double precAtPull=0;
    int countAtPull=0;
    bool atPull=true;
    double precAcceptT=0;
    int countAcceptT=0;
    bool acceptT=true;
    double precRejectT=0;
    int countRejectT=0;
    bool rejectT=false;
    double precBetweenT=0;
    int countBetweenT=0;
    bool betweenT=false;

    int tBetBefore=0;
    int fBetBefore=0;
    int tBetAfter=0;
    int fBetAfter=0;
#ifdef GRAPH_SPOTTING_RESULTS
    //cv::Mat newFullLine = cv::Mat::zeros(2,fullGraph.cols,CV_8UC3);
    //int inFull=0;
    if (undoneGraph.cols==0)
        undoneGraph = cv::Mat::zeros(1,instancesByScore.size()+3,CV_8UC3);
    if (undoneGraph.cols<instancesByScore.size()+3)
    {
        int dif = instancesByScore.size()+3-undoneGraph.cols;
        cv::hconcat(undoneGraph,cv::Mat::zeros(undoneGraph.rows,dif,CV_8UC3),undoneGraph);
    }
    cv::Mat newUndoneLine = cv::Mat::zeros(2,undoneGraph.cols,CV_8UC3);
    int inUndone=0;
#endif
    for (auto iter=instancesByLocation.begin(); iter!=instancesByLocation.end(); iter++)
    {
        assert(instancesById.find((**iter).id) != instancesById.end());
    }
    for (auto iter=instancesByScore.begin(); iter!=instancesByScore.end(); iter++)
    {
        assert(instancesById.find(*iter) != instancesById.end());
        Spotting& spotting = instancesById.at(*iter);
        bool t=false;
        if (spotting.gt==1)
            t=true;
        else if (spotting.gt!=0)
        {
            t = GlobalK::knowledge()->ngramAt(ngram, spotting.pageId, spotting.tlx, spotting.tly, spotting.brx, spotting.bry);
            spotting.gt=t;
        }

        if (spotting.score >=pullFromScore && atPull)
        {
            atPull=false;
#ifdef GRAPH_SPOTTING_RESULTS

            newUndoneLine.at<cv::Vec3b>(0,inUndone++) = cv::Vec3b(255,200,200);
#endif
        }
        if (spotting.score >=acceptThreshold && acceptT)
        {
            acceptT=false;
            betweenT=true;
#ifdef GRAPH_SPOTTING_RESULTS
            if (atn==0)
                newUndoneLine.at<cv::Vec3b>(0,inUndone++) = cv::Vec3b(255,0,0);
            else if (atn==1)
                newUndoneLine.at<cv::Vec3b>(0,inUndone++) = cv::Vec3b(255,200,0);
            else if (atn==2)
                newUndoneLine.at<cv::Vec3b>(0,inUndone++) = cv::Vec3b(255,0,200);
#endif
        }
        if (spotting.score >rejectThreshold && betweenT)
        {
            rejectT=true;
            betweenT=false;
#ifdef GRAPH_SPOTTING_RESULTS
            if (rtn==0)
                newUndoneLine.at<cv::Vec3b>(0,inUndone++) = cv::Vec3b(255,0,0);
            else if (rtn==1)
                newUndoneLine.at<cv::Vec3b>(0,inUndone++) = cv::Vec3b(255,200,0);
            else if (rtn==2)
                newUndoneLine.at<cv::Vec3b>(0,inUndone++) = cv::Vec3b(255,0,200);
#endif
        }

        if (atPull)
        {
            countAtPull++;
            precAtPull+=t?1:0;
        }
        if (acceptT)
        {
            countAcceptT++;
            precAcceptT+=t?1:0;
        }
        if (rejectT)
        {
            countRejectT++;
            precRejectT+=t?1:0;
        }
        if (betweenT)
        {
            countBetweenT++;
            precBetweenT+=t?1:0;
        }
        if (!acceptT && atPull)
        {
            if (t)
                tBetBefore++;
            else
                fBetBefore++;
        }

        if (!rejectT && !atPull)
        {
            if (t)
                tBetAfter++;
            else
                fBetAfter++;
        }
#ifdef GRAPH_SPOTTING_RESULTS
        if (t)
            newUndoneLine.at<cv::Vec3b>(0,inUndone++) = cv::Vec3b(0,205,0);
        else
            newUndoneLine.at<cv::Vec3b>(0,inUndone++) = cv::Vec3b(0,0,205);
#endif
    }
    cout<<"["<<id<<"]:"<<ngram<<" serving batch."<<endl;
    cout<<"("<<precAcceptT/countAcceptT<<", "<<(int)precAcceptT<<", "<<(int)(countAcceptT-precAcceptT)<<") A ";
    cout<<"("<<(0.0+tBetBefore)/(tBetBefore+fBetBefore)<<", "<<tBetBefore<<", "<<fBetBefore<<") P ";
    cout<<"("<<(0.0+tBetAfter)/(tBetAfter+fBetAfter)<<", "<<tBetAfter<<", "<<fBetAfter<<") R ";
    cout<<"("<<precRejectT/countRejectT<<", "<<(int)precRejectT<<", "<<(int)(countRejectT-precRejectT)<<")"<<endl;;

#ifdef GRAPH_SPOTTING_RESULTS
    undoneGraph.push_back(newUndoneLine);
    //if (undoneGraph.rows-1%4==0)
        cv::imwrite(undoneGraphName,undoneGraph);
    //cv::imshow(undoneGraphName,undoneGraph);
    //cv::waitKey(100);
    atPull=true;
    acceptT=true;
    rejectT=false;
    betweenT=false;
    if (fullInstancesByScore.size() < instancesById.size())
    {
        fullInstancesByScore.clear();
        for (auto p : instancesById)
            fullInstancesByScore.insert(&(instancesById.at(p.first)));
    }

    if (fullGraph.cols==0)
        fullGraph = cv::Mat::zeros(1,fullInstancesByScore.size()+3,CV_8UC3);
    if (fullGraph.cols<fullInstancesByScore.size()+3)
    {
        int dif = fullInstancesByScore.size()+3-fullGraph.cols;
        cv::hconcat(fullGraph,cv::Mat::zeros(fullGraph.rows,dif,CV_8UC3),fullGraph);
    }
    cv::Mat newFullLine = cv::Mat::zeros(2,fullGraph.cols,CV_8UC3);
    int inFull=0;
    for (auto iter=fullInstancesByScore.begin(); iter!=fullInstancesByScore.end(); iter++)
    {
        bool t=false;
        if ((*iter)->gt==1)
            t=true;
        else if ((*iter)->gt!=0)
        {
            t = GlobalK::knowledge()->ngramAt(ngram, (*iter)->pageId, (*iter)->tlx, (*iter)->tly, (*iter)->brx, (*iter)->bry);
            (*iter)->gt=t;
        }

        int color=205;
        if (instancesByScore.find((*iter)->id) == instancesByScore.end())
            color = 100;

        if ((*iter)->score >=pullFromScore && atPull)
        {
            atPull=false;
            newFullLine.at<cv::Vec3b>(0,inFull++) = cv::Vec3b(255,200,190);
        }
        if ((*iter)->score >=acceptThreshold && acceptT)
        {
            acceptT=false;
            betweenT=true;
            if (atn==0)
                newFullLine.at<cv::Vec3b>(0,inFull++) = cv::Vec3b(255,0,0);
            else if (atn==1)
                newFullLine.at<cv::Vec3b>(0,inFull++) = cv::Vec3b(255,200,0);
            else if (atn==2)
                newFullLine.at<cv::Vec3b>(0,inFull++) = cv::Vec3b(255,0,200);
        }
        if ((*iter)->score >rejectThreshold && betweenT)
        {
            rejectT=true;
            betweenT=false;
            if (rtn==0)
                newFullLine.at<cv::Vec3b>(0,inFull++) = cv::Vec3b(255,0,0);
            else if (rtn==1)
                newFullLine.at<cv::Vec3b>(0,inFull++) = cv::Vec3b(255,200,0);
            else if (rtn==2)
                newFullLine.at<cv::Vec3b>(0,inFull++) = cv::Vec3b(255,0,200);
        }
        if (t)
            newFullLine.at<cv::Vec3b>(0,inFull++) = cv::Vec3b(0,color,0);
        else
            newFullLine.at<cv::Vec3b>(0,inFull++) = cv::Vec3b(0,0,color);
    } 
    fullGraph.push_back(newFullLine);
    //if (fullGraph.rows-1%4==0)
        cv::imwrite(fullGraphName,fullGraph);
    //cv::imshow(fullGraphName,fullGraph);
    //cv::waitKey(100);
#endif

    precAtPull/=countAtPull;
    precAcceptT/=countAcceptT;
    precRejectT/=countRejectT;
    precBetweenT/=countBetweenT;

    b->addDebugInfo(precAtPull,precAcceptT,precRejectT,precBetweenT, countAcceptT, countRejectT, countBetweenT);
    //cout<<"precAtPull: "<<precAtPull<<"\t
}
#endif

SpottingsBatch* SpottingResults::getBatch(bool* done, unsigned int num, bool hard, unsigned int maxWidth, int color, string prevNgram, bool need) {
    ///debugState();

    if (!need && (numLeftInRange<12 && numLeftInRange>=0) && starts.size()>1)
#ifndef NO_NAN
        return NULL;
#else
    {}
#endif

    if (acceptThreshold==-1 && rejectThreshold==-1)
        EMThresholds();
#ifdef TEST_MODE
    cout <<"\ngetBatch, from:"<<pullFromScore<<"\n"<<endl;

#endif
    //sem_wait(&mutexSem);
    
    unsigned int toRet = ((hard&&instancesByScore.size()>=num)||((((signed int)instancesByScore.size())-(signed int) num)>3))?num:instancesByScore.size();
    if (toRet==0)
    {
        //This occurs in a race condition when the spotting results is queried before it can be removed from the MasterQueue
        return NULL;
    }
    SpottingsBatch* ret = new SpottingsBatch(ngram,id);
#ifdef TEST_MODE
    setDebugInfo(ret);
#endif
    set<float> scoresToDraw;
    //normal_distribution<float> distribution(pullFromScore,((pullFromScore-acceptThreshold)+(rejectThreshold-pullFromScore))/4.0);
    uniform_real_distribution<float> distribution(acceptThreshold,rejectThreshold);
    for (int i=0; i<min(toRet,(unsigned int)instancesByScore.size()); i++)
    {
        float v = distribution(generator);
        if (v<=acceptThreshold)
            v=2*acceptThreshold-v;
        if (v>=rejectThreshold)
            v=2*rejectThreshold-v;
        scoresToDraw.insert(v);
    }

    auto iterR = instancesByScore.end();
    do
    {
        iterR--;
        assert(instancesById.find(*iterR) != instancesById.end());
    } while(iterR!=instancesByScore.begin() && instancesById.at(*iterR).score >rejectThreshold);
    for (float drawScore : scoresToDraw)
    {
        auto iter=iterR;
        assert(instancesById.find(*iter) != instancesById.end());
        while (instancesById.at(*iter).score >drawScore && iter!=instancesByScore.begin())
        {
            iter--;
        }
        assert(instancesById.find(*iter) != instancesById.end());
        if (instancesById.at(*iter).score<acceptThreshold && iter!=instancesByScore.end())
        {
            iter++;
            if (iter==instancesByScore.end())
                iter--;
        }
        if (iter==iterR)
        {
            if (iterR!=instancesByScore.begin())
            {
                iterR--;
            }
            else
            {
                iterR++;
            }
        }
    
        assert (updateMap.find(*iter)==updateMap.end());
        assert(instancesById.find(*iter) != instancesById.end());

        SpottingImage tmp(instancesById.at(*iter),maxWidth,contextPad,color,prevNgram);
        ret->push_back(tmp);

        assert(ret->at(ret->size()-1).id == (*iter));
        
        instancesByScore.erase(iter);
    }

    assert(instancesById.find(*iterR) != instancesById.end());
    //check if done
    if (instancesByScore.size()<=1)
        *done=true;
    else if (instancesById.at(*iterR).score > rejectThreshold)
    {
        if (iterR==instancesByScore.begin())
            *done=true;
        else
        {
            iterR--;
            assert(instancesById.find(*iterR) != instancesById.end());
            if (instancesById.at(*iterR).score < acceptThreshold)
                *done=true;
        }
    }
    else if (instancesById.at(*iterR).score < acceptThreshold)
        *done=true;

    
    if (batchesSinceChange++ > CHECK_IF_BAD_SPOTTING_START)
    {
        if (numberClassifiedTrue/(numberClassifiedTrue+numberClassifiedFalse+0.0) < CHECK_IF_BAD_SPOTTING_THRESH)
            this->done=true;
    }
    
    if (*done)
        allBatchesSent=true;
    
    //cout<<"batch is "<<ret->size()<<endl;
    //cout << "send: ";
    for (int i=0; i<ret->size(); i++)
    {   
        assert(instancesById.find(ret->at(i).id) != instancesById.end());
        starts[ret->at(i).id] = chrono::system_clock::now();
        
        //time_t tt= chrono::system_clock::to_time_t ( starts[ret->at(i).id] );
        //cout << ret->at(i).id<<", ";//" at time "<< ctime(&tt)<<endl;
    }
        //cout <<endl;
    //sem_post(&mutexSem);
    //numBatches++;
    
    ///debugState();
    return ret;
}

vector<Spotting>* SpottingResults::feedback(int* done, const vector<string>& ids, const vector<int>& userClassifications, int resent, vector<pair<unsigned long,string> >* retRemove)
{
    ///debugState();
    /*cout << "fed: ";
    for (int i=0; i<ids.size(); i++)
    {   
        cout << ids[i]<<", ";
    }
    cout<<endl;*/
    
    vector<Spotting>* ret = new vector<Spotting>();
    int swing=0;
    for (unsigned int i=0; i< ids.size(); i++)
    {
        unsigned long id = stoul(ids[i]);
        int check = starts.erase(id);
        //assert(check==1 || resent);

        //In the event this spotting has been updated
        while (updateMap.find(id)!=updateMap.end())
        {
            //unsigned long oldId=id;
            id=updateMap[id];
            //instancesById.erase(oldId);
        }

        auto iterClass = classById.find(id);
        if (!resent && iterClass!=classById.end())
            continue;//we'll skip processing if they waited to long and it got done. Unreliable

        if (resent)
        {
            if (iterClass->second)
                numberClassifiedTrue--;
            else
                numberClassifiedFalse--;
        }
        
        assert(instancesById.find(id) != instancesById.end());
        instancesById.at(id).type=SPOTTING_TYPE_APPROVED;
        // adjust threshs
        if (userClassifications[i]>0)
        {
            swing++;
            numberClassifiedTrue++;
#ifdef NO_NAN
        GlobalK::knowledge()->accepted();
#endif
            if (!resent || !iterClass->second)
            {
                ret->push_back(instancesById.at(id)); //otherwise we've already sent it
            }
            classById[id]=true;
        }
        else if (userClassifications[i]==0)
        {
            swing--;
            numberClassifiedFalse++;
#ifdef NO_NAN
        GlobalK::knowledge()->rejected();
#endif
            if (resent && (retRemove && iterClass->second))
            {
                retRemove->push_back(make_pair(id,instancesById.at(id).ngram));
            }
            classById[id]=false;
        }
        else if (!resent)//someone passed, so we need to add it again, unless this is resent, in whichcase we use the old.
        { 
            instancesByScore.insert(id);
            tracer = instancesByScore.begin();
        }
    }

    if (batchesSinceChange > CHECK_IF_BAD_SPOTTING_START)
    {
        if (numberClassifiedTrue/(numberClassifiedTrue+numberClassifiedFalse+0.0) < CHECK_IF_BAD_SPOTTING_THRESH)
            this->done=true;
    }

    if (this->done)
    {
        *done=true;
        return ret;
    }
    bool allWereSent = allBatchesSent; 
    EMThresholds(swing);
  


#ifdef TEST_MODE
    cout<<"["<<id<<"]:"<<ngram<<", all sent: "<<allBatchesSent<<", waiting for "<<starts.size()<<", num left "<<instancesByScore.size()<<endl;
    //for (auto i=instancesByScore.begin(); i!=instancesByScore.end(); i++)
    //    cout <<(**i).id<<"[tlx:"<<(**i).tlx<<" score:"<<(**i).score<<"]"<<endl;
#endif

    if (resent==0 && starts.size()==0 && allBatchesSent)
    {
        if (allWereSent)
            *done=1;
        else
            *done=2;
        this->done=true;
#ifdef TEST_MODE
        cout <<"["<<id<<"]:"<<ngram<<", all batches sent, cleaning up"<<endl;
        int numAutoAccepted=0;
        int trueAutoAccepted=0;
        int numAutoRejected=0;
        int trueAutoRejected=0;
#endif
        tracer = instancesByScore.begin();
        assert(instancesById.find(*tracer) != instancesById.end());
        while (tracer!=instancesByScore.end() && instancesById.at(*tracer).score <= acceptThreshold)
        {
            assert(instancesById.find(*tracer) != instancesById.end());

            instancesById.at(*tracer).type=SPOTTING_TYPE_THRESHED;
            ret->push_back(instancesById.at(*tracer));
            classById[instancesById.at(*tracer).id]=true;
            numberAccepted++;
#ifdef NO_NAN
            GlobalK::knowledge()->autoAccepted();
#endif
#ifdef TEST_MODE
            numAutoAccepted++;
            trueAutoAccepted += GlobalK::knowledge()->ngramAt(ngram, instancesById.at(*tracer).pageId, instancesById.at(*tracer).tlx, instancesById.at(*tracer).tly, instancesById.at(*tracer).brx, instancesById.at(*tracer).bry)?1:0;
#endif
#ifdef TEST_MODE_LONG
            cout <<" "<<instancesById.at(*tracer).id<<"[tlx:"<<instancesById.at(*tracer).tlx<<", score:"<<instancesById.at(*tracer).score<<"]: true"<<endl;
#endif
            tracer++;
        }
        while (tracer!=instancesByScore.end())
        {
            assert(instancesById.find(*tracer) != instancesById.end());

            instancesById.at(*tracer).type=SPOTTING_TYPE_THRESHED;
            classById[instancesById.at(*tracer).id]=false;
#ifdef NO_NAN
            GlobalK::knowledge()->autoRejected();
#endif
#ifdef TEST_MODE
            numAutoRejected++;
            trueAutoRejected += GlobalK::knowledge()->ngramAt(ngram, instancesById.at(*tracer).pageId, instancesById.at(*tracer).tlx, instancesById.at(*tracer).tly, instancesById.at(*tracer).brx, instancesById.at(*tracer).bry)?1:0;
#endif
#ifdef TEST_MODE_LONG
            cout <<" "<<instancesById.at(*tracer).id<<"[tlx:"<<instancesById.at(*tracer).tlx<<", score:"<<instancesById.at(*tracer).score<<"]: false"<<endl;
#endif
            tracer++;
        }
        instancesByScore.clear();
        //cout << "hit end "<<(tracer==instancesByScore.end())<<endl;
        numberRejected = distance(tracer,instancesByScore.end());
#ifdef TEST_MODE
        cout<<"["<<id<<"]:"<<ngram<<" AUTO accepted["<<numAutoAccepted<<"]: "<<(0.0+trueAutoAccepted)/numAutoAccepted<<"  rejected["<<numAutoRejected<<"]: "<<(0.0+trueAutoRejected)/numAutoRejected<<endl;
#endif        
        tracer = instancesByScore.begin();
    }
    else if (!allBatchesSent && allWereSent)
    {
        *done=-1;
#ifdef TEST_MODE
        cout<<"SpottingResults ["<<id<<"]:"<<ngram<<" has more batches and will be re-enqueued"<<endl;
#endif
    }
    ///debugState();
    return ret;
}
    
   
    

bool SpottingResults::EMThresholds(int swing)
{
    ///debugState();
    assert(instancesById.size()>1);
    assert(maxScore!=-999999);
    bool init = acceptThreshold==-1 && rejectThreshold==-1;
    float oldMidScore = acceptThreshold + (rejectThreshold-acceptThreshold)/2.0;
    /*This will likely predict very narrow and distinct distributions
     *initailly. This should be fine as we sample from the middle of
     *the thresholds outward.
     */
    
#ifdef TEST_MODE 
    //test
    int displayLen=90;
    vector<int> histogramCP(displayLen);
    vector<int> histogramCN(displayLen);
    vector<int> histogramGP(displayLen);
    vector<int> histogramGN(displayLen);
    //test
#endif
    
    if (init)
    {
        //we initailize our split with Otsu, as we are assuming two distributions.
        //make histogram
        vector<int> histogram(256);
        //int total = 0;//instancesById.size();
        for (auto p : instancesById)
        {
            if (p.second.score==p.second.score)
            {
                //total++;
                unsigned long id = p.first;
                
                int bin = 255*(instancesById.at(id).score-minScore)/(maxScore-minScore);
                if (bin<0) bin=0;
                if (bin>histogram.size()-1) bin=histogram.size()-1;
                histogram[bin]++;
            }
        }
        
        
        double thresh = GlobalK::otsuThresh(histogram);//( threshold1 + threshold2 ) / 2.0;
        pullFromScore = (thresh/256)*(maxScore-minScore)+minScore;
#ifdef TEST_MODE_LONG
        if (ngram.compare("te")==0)
            cout<<"init pullFromScore = "<<pullFromScore<<endl;
#endif
    }
    
    //expectation
    //bool initV=init;
    //while (1)
    //{
#ifdef TEST_MODE
        for (int i=0; i<displayLen; i++)
        {
            histogramCP[i]=0;
            histogramGP[i]=0;
            histogramCN[i]=0;
            histogramGN[i]=0;;
        }
#endif        
        
        //map<unsigned long, bool> expected;
        vector<float> expectedTrue;
        float sumTrue=0;
        vector<float> expectedFalse;
        float sumFalse=0;
        
        
        
        
        numLeftInRange=0;
        for (auto p : instancesById)
        {
            if (p.second.score!=p.second.score)
                continue; 
            unsigned long id = p.first;
            
#ifdef TEST_MODE
            //test
            int bin=(displayLen-1)*(instancesById.at(id).score-minScore)/(maxScore-minScore);
#endif        
            
            if (classById.find(id)!=classById.end())
            {
                if (classById[id])
                {
                    expectedTrue.push_back(instancesById.at(id).score);
                    //expectedTrue.push_back(instancesById.at(id).score);
                    sumTrue+=1*instancesById.at(id).score;
                    
#ifdef TEST_MODE
                    //test
                    if (bin>=0) histogramCP.at(bin)++;
#endif        
                }
                else
                {
                    expectedFalse.push_back(instancesById.at(id).score);
                    //expectedFalse.push_back(instancesById.at(id).score);
                    sumFalse+=1*instancesById.at(id).score;
                    
#ifdef TEST_MODE
                    //test
                    if (bin>=0) histogramCN.at(bin)++;
#endif        
                }
            }
            else
            {
                if (instancesById.at(id).score >acceptThreshold && instancesById.at(id).score<rejectThreshold)
                    numLeftInRange++;
                double trueProb = exp(-1*pow(instancesById.at(id).score - trueMean,2)/(2*trueVariance));
                double falseProb = exp(-1*pow(instancesById.at(id).score - falseMean,2)/(2*falseVariance));
                if (init)
                {
                    trueProb = instancesById.at(id).score<pullFromScore;
                    falseProb = instancesById.at(id).score>pullFromScore;
                }
                if (trueProb>falseProb)
                {
                    sumTrue+=instancesById.at(id).score;
                    expectedTrue.push_back(instancesById.at(id).score);
                    
#ifdef TEST_MODE
                    //test
                    if (bin>=0) histogramGP.at(bin)++;
#endif        
                }
                else
                {
                    expectedFalse.push_back(instancesById.at(id).score);
                    sumFalse+=instancesById.at(id).score;
                    
#ifdef TEST_MODE
                    //test
                    if (bin>=0) histogramGN.at(bin)++;
#endif        
                }
            }
        }
        
        //maximization
        if (expectedTrue.size()!=0)
            trueMean=sumTrue/expectedTrue.size();
        if (expectedFalse.size()!=0)
            falseMean=sumFalse/expectedFalse.size();
        trueVariance=0;
        for (float score : expectedTrue)
            trueVariance += (score-trueMean)*(score-trueMean);
        if (expectedTrue.size()!=0)
            trueVariance/=expectedTrue.size();
        falseVariance=0;
        for (float score : expectedFalse)
            falseVariance += (score-falseMean)*(score-falseMean);
        if (expectedFalse.size()!=0)
            falseVariance/=expectedFalse.size();
        

        //set new thresholds
        float numStdDevs = 1;// + ((1.0+min((int)instancesById.size(),50))/(1.0+min((int)classById.size(),50)));//Use less as more are classified, we are more confident. Capped to reduce effect of many returned instances bogging us down.
        float acceptThreshold1 = falseMean-numStdDevs*sqrt(falseVariance);
        float rejectThreshold1 = trueMean+numStdDevs*sqrt(trueVariance);
        float acceptThreshold2 = trueMean-numStdDevs*sqrt(trueVariance);
        float rejectThreshold2 = falseMean-numStdDevs*sqrt(falseVariance);
        //float prevAcceptThreshold=acceptThreshold;
        //float prevRejectThreshold=rejectThreshold;
        if (falseVariance!=0 && trueVariance!=0)
        {
            acceptThreshold = max( min(acceptThreshold1,acceptThreshold2), minScore);
            //rejectThreshold = min( max(rejectThreshold1,rejectThreshold2), maxScore);
            rejectThreshold = min( rejectThreshold2, maxScore);//This seems to work better
        }
        else if (falseVariance==0)
        {
            acceptThreshold = acceptThreshold2;
            rejectThreshold = trueMean+2*numStdDevs*sqrt(trueVariance);
        }
        else
        {
            rejectThreshold = rejectThreshold2;
            acceptThreshold = falseMean-2*numStdDevs*sqrt(falseVariance);
        }

#ifdef TEST_MODE
        if (rejectThreshold==rejectThreshold1)
            rtn=1;
        else if (rejectThreshold==rejectThreshold2)
            rtn=2;
        else
            rtn=0;
        
        if (acceptThreshold==acceptThreshold1)
            atn=1;
        else if (acceptThreshold==acceptThreshold2)
            atn=2;
        else
            atn=0;
#endif //TEST_MODE

        /*if (!init)
        {
            float difAcceptThreshold = acceptThreshold-prevAcceptThreshold;
            if ( (difAcceptThreshold>0 && lastDifAcceptThreshold>0) || (difAcceptThreshold<0 && lastDifAcceptThreshold<0) )
            {
                acceptThreshold+=momentum*lastDifAcceptThreshold;
                difAcceptThreshold = acceptThreshold-prevAcceptThreshold;
            }
            lastDifAcceptThreshold=difAcceptThreshold;

            float difRejectThreshold = rejectThreshold-prevRejectThreshold;
            if ( (difRejectThreshold>0 && lastDifRejectThreshold>0) || (difRejectThreshold<0 && lastDifRejectThreshold<0) )
            {
                rejectThreshold+=momentum*lastDifRejectThreshold;
                difRejectThreshold = rejectThreshold-prevRejectThreshold;
            }
            lastDifRejectThreshold=difRejectThreshold;
        }*/
#ifdef TEST_MODE
        cout <<"adjusted threshs, now "<<acceptThreshold<<" <> "<<rejectThreshold<<"    computed with std devs of: f:"<<sqrt(falseVariance)<<", t:"<<sqrt(trueVariance)<<endl;
#endif        
        /*if (!init || !initV)
            break;
        
        if (fabs(pullFromScore-trueMean)<0.05)
            break;
        else
            initV=false;
        pullFromScore = trueMean;*/
    //}
    float prevPullFromScore = pullFromScore;
    pullFromScore = (acceptThreshold+rejectThreshold)/2.0;// -sqrt(trueVariance);
    //pullFromScore = trueMean-sqrt(trueVariance);
    /*if (!init)
    {
        float difPullFromScore = pullFromScore-prevPullFromScore;
#ifdef TEST_MODE
        cout<<"orig pull dif="<<difPullFromScore<<",  ";
#endif
        //if ( (pull>0 && lastDifPullFromScore>0) || (difPullFromScore<0 && lastDifPullFromScore<0) )
        //{
        //    pullFromScore+=momentum*lastDifPullFromScore;
        //    difPullFromScore = pullFromScore-prevPullFromScore;
        //}
        if ((swing>0 && difPullFromScore<0) || (swing<0 && difPullFromScore>0))
            pullFromScore=prevPullFromScore;
        if ((swing>0 && lastDifPullFromScore>0) || (swing<0 && lastDifPullFromScore<0))
        {
            pullFromScore+=momentum*lastDifPullFromScore;
        }
        else 
        {
            pullFromScore+= (swing/10.0)*fabs(difPullFromScore+lastDifPullFromScore);
        }
        difPullFromScore = pullFromScore-prevPullFromScore;
#ifdef TEST_MODE
        cout<<"final pull="<<pullFromScore<<" dif="<<difPullFromScore<<",  swing="<<swing<<",  lastDif="<<lastDifPullFromScore<<endl;
#endif
        lastDifPullFromScore=difPullFromScore;
    }*/

    //safe gaurd
    if (pullFromScore>rejectThreshold)
        pullFromScore=rejectThreshold;
    else if (pullFromScore<acceptThreshold)
        pullFromScore=acceptThreshold;

//cout <<"true mean "<<trueMean<<" true var "<<trueVariance<<endl;
//cout <<"false mean "<<falseMean<<" false var "<<falseVariance<<endl;


//cout <<"adjusted threshs, now "<<acceptThreshold<<" <> "<<rejectThreshold<<"    computed with std dev of: "<<numStdDevs<<endl;
/*//a historgram visualization

int falseMeanBin=(displayLen-1)*(falseMean-minScore)/(maxScore-minScore);
int falseVarianceBin1=(displayLen-1)*(falseMean-sqrt(falseVariance)-minScore)/(maxScore-minScore);
int falseVarianceBin2=(displayLen-1)*(falseMean+sqrt(falseVariance)-minScore)/(maxScore-minScore);
int trueMeanBin=(displayLen-1)*(trueMean-minScore)/(maxScore-minScore);
int trueVarianceBin1=(displayLen-1)*(trueMean-sqrt(trueVariance)-minScore)/(maxScore-minScore);
int trueVarianceBin2=(displayLen-1)*(trueMean+sqrt(trueVariance)-minScore)/(maxScore-minScore);
int acceptThresholdBin=(displayLen-1)*(acceptThreshold-minScore)/(maxScore-minScore);
int rejectThresholdBin=(displayLen-1)*(rejectThreshold-minScore)/(maxScore-minScore);
int pullFromScoreBin=(displayLen-1)*(pullFromScore-minScore)/(maxScore-minScore);
for (int bin=0; bin<displayLen; bin++)
{
    if (falseMeanBin==bin)
        cout <<"F";
    else
        cout <<" ";
    if (falseVarianceBin1==bin || falseVarianceBin2==bin)
        cout <<"f";
        else
            cout <<" ";
        if (trueMeanBin==bin)
            cout <<"T";
        else
            cout <<" ";
        if (trueVarianceBin1==bin || trueVarianceBin2==bin)
            cout <<"t";
        else
            cout <<" ";
        if (rejectThresholdBin==bin)
            cout <<"R";
        else
            cout <<" ";
        if (acceptThresholdBin==bin)
            cout <<"A";
        else
            cout <<" ";
        if (pullFromScoreBin==bin)
            cout <<"P";
        else
            cout <<" ";
        cout <<":";
        for (int c=0; c<histogramCP[bin]; c++)
            cout<<"#";
        for (int c=0; c<histogramGP[bin]; c++)
            cout<<"+";
        for (int c=0; c<histogramCN[bin]; c++)
            cout<<"=";
        for (int c=0; c<histogramGN[bin]; c++)
            cout<<"-";
        cout<<endl;
    }
    cout <<"oooooooooooooooooooooooooooooooooooooooooo"<<endl;
    //*/
    
    
    if(acceptThreshold>rejectThreshold) {//This is the case where the distributions are so far apart they "don't overlap"
        if (instancesById.size()/3 < classById.size()){//Be sure we aren't hitting this too early
            float mid = rejectThreshold + (acceptThreshold-rejectThreshold)/2.0;
            acceptThreshold = rejectThreshold = mid;//We finish here, by accepting and rejecting everything left.
            cout<<"cross threshed, finisheing"<<endl;
        } 
        else {
            acceptThreshold = trueMean;//This is an overcorrection, allowing a alater batch to fix us.
            rejectThreshold = falseMean;
            cout<<"cross threshed, correcting"<<endl;
        }
    }
    
    /*if (!init)
    {
        int ns=0;
        float innerTrue;
        float innerFalse;
        do
        {
            innerTrue=trueMean+ns*sqrt(trueVariance);
            innerFalse=falseMean-ns*sqrt(falseVariance);
            ns++;
        } while (innerTrue<innerFalse);
        innerTrue=trueMean+(ns-1)*sqrt(trueVariance);
        innerFalse=falseMean-(ns-1)*sqrt(falseVariance);
        float newMidScore = innerTrue+ (innerFalse-innerTrue)/2.0;
        pullFromScore = trueMean;//newMidScore;
        //cout << "pullFromScore: "<<pullFromScore<<endl;
    }*/
    if (init) {
        allBatchesSent=false;
        numLeftInRange=instancesByScore.size();
    }
    else
    {
        allBatchesSent=true;
        auto iter = instancesByScore.begin();
        while (iter!=instancesByScore.end() && instancesById.at(*iter).score < rejectThreshold)
        {
            assert(instancesById.find(*iter) != instancesById.end());
            if (instancesById.at(*iter).score > acceptThreshold && instancesById.at(*iter).score < rejectThreshold)
            {
                allBatchesSent=false;
                break;
            }
            iter++;
        } 
    }
    ///debugState();
    return allBatchesSent;
}


bool SpottingResults::checkIncomplete()
{
    ///debugState();
    bool incomp=false;
    //cout <<"checkIncomplete, starts is "<<starts.size()<<endl;
    //vector<unsigned long> toRemove;
    for (auto iter=starts.begin(); iter!=starts.end(); iter++)
    {
        auto start = *iter;
        chrono::system_clock::duration d = chrono::system_clock::now()-start.second;
        chrono::minutes pass = chrono::duration_cast<chrono::minutes> (d);
        //cout<<pass.count()<<" minutes has past for "<<(start.first)<<endl;
        if (pass.count() > 20) //if 20 mins has passed
        {
            unsigned long restartId=start.first;
            if (instancesById.find(restartId) == instancesById.end())
            {
                if (updateMap.find(restartId) != updateMap.end())
                    restartId = updateMap.at(start.first);
                else
                    assert(false && "SpottingResults::checkIncomplete() is trying to restart a non-existant spotting id");
            }
            instancesByScore.insert(restartId);
            tracer = instancesByScore.begin();
            incomp=true;
            //toRemove.push_back(start.first);
#ifdef TEST_MODE
            cout<<"Timeout ("<<pass.count()<<") on batch "<<start.first<<endl;
#endif     
            iter = starts.erase(iter);
            if (iter!=starts.begin())
                iter--;
            if (iter==starts.end())
                break;
        }
    }
    //for (unsigned long id : toRemove)
    //    starts.erase(id);
    if (incomp && allBatchesSent)
    {
        allBatchesSent=false;
        return true;
    }
    ///debugState();
    return false;
}


multiset<Spotting*,tlComp>::iterator SpottingResults::findOverlap(const Spotting& spotting, float* ratioOff) const
{
    ///debugState();
    bool updated=false;
    int width = spotting.brx-spotting.tlx;
    int height = spotting.bry-spotting.tly;
    //Spotting* bestSoFar=NULL;
    auto bestSoFarIter = instancesByLocation.end();
    int bestOverlap=0;
    double spottingArea = (spotting.brx-spotting.tlx)*(spotting.bry-spotting.tly);
    for (int tlx=spotting.tlx-width*(1-UPDATE_OVERLAP_THRESH); tlx<spotting.tlx+width*(1-UPDATE_OVERLAP_THRESH); tlx++)
    {
        //Find all spottings for given tlx
        Spotting lb(spotting.pageId,tlx,spotting.tly-height*(1-UPDATE_OVERLAP_THRESH));
        Spotting ub(spotting.pageId,tlx,spotting.tly+height*(1-UPDATE_OVERLAP_THRESH));
        auto itLow = instancesByLocation.lower_bound(&lb);
        auto itHigh = instancesByLocation.upper_bound(&ub);


        for (;itLow!=itHigh; itLow++)
        {
            int overlapArea = ( min(spotting.brx,(*itLow)->brx) - max(spotting.tlx,(*itLow)->tlx) ) * ( min(spotting.bry,(*itLow)->bry) - max(spotting.tly,(*itLow)->tly) );
            double thresh = UPDATE_OVERLAP_THRESH;

            assert(instancesById.find((*itLow)->id) != instancesById.end());

            bool updateWhenInBatch = (*itLow)->type!=SPOTTING_TYPE_THRESHED && instancesByScore.find((*itLow)->id)==instancesByScore.end();
            if (updateWhenInBatch)
                thresh=UPDATE_OVERLAP_THRESH_TIGHT;
            double ratio = overlapArea/max(spottingArea,1.0*((*itLow)->brx-(*itLow)->tlx)*((*itLow)->bry-(*itLow)->tly));
            if (ratio > thresh)
            {
                if (overlapArea > bestOverlap)
                {
                    bestOverlap=overlapArea;
                    //bestSoFar=*itLow;
                    bestSoFarIter=itLow;
                    if (ratioOff!=NULL)
                        *ratioOff = 1.0 - (ratio-thresh)/(1.0-thresh);
                }
                else
                {
                    tlx=spotting.tlx+width*(1-UPDATE_OVERLAP_THRESH);
                    break;
                }


                
            }
        }
    }
    ///debugState();
    return bestSoFarIter;
}

//This method is to check to see if we actually have this exemplar already and then prevent is from being re-approved
void SpottingResults::updateSpottingTrueNoScore(const SpottingExemplar& spotting)
{
    ///debugState();
    assert(spotting.tlx!=-1);
    assert(spotting.score != spotting.score);

    //Scan for possibly overlapping (the same) spottings
    Spotting* best=NULL;
    auto bestIter = findOverlap(spotting,NULL);
    if (bestIter != instancesByLocation.end())
        best=*bestIter;
    if (best!=NULL)
    {

        bool updateWhenInBatch = (best)->type!=SPOTTING_TYPE_THRESHED && instancesByScore.find(best->id)==instancesByScore.end();
        if (updateWhenInBatch)
            updateMap[best->id]=spotting.id;
#ifdef TEST_MODE
        else
            testUpdateMap[best->id]=spotting.id;
#endif
        //Add this spotting
        assert(spotting.pageId>=0);
        instancesById[spotting.id]=spotting;
        instancesById.at(spotting.id).type=SPOTTING_TYPE_APPROVED;
        instancesById.at(spotting.id).score=best->score;//we replace the score so that we contribute to the thresholding
        instancesByLocation.insert(&instancesById.at(spotting.id));
        classById[spotting.id]=true;
        instancesByLocation.erase(bestIter); //erase by iterator

        //remove the old one
        int removed = instancesByScore.erase(best->id); //erase by value (pointer)
        if (removed)
        {
            tracer = instancesByScore.begin();
        }
        instancesById.erase(best->id);
    }
    else
    {
        addTrueNoScore(spotting);
    }
    ///debugState();
}

//combMin
bool SpottingResults::updateSpottings(vector<Spotting>* spottings)
{
    ///debugState();
    for (Spotting& spotting : *spottings)
    {
        assert(spotting.tlx!=-1);
        //Update max and min
        if (spotting.score>maxScore)
        {
            if (falseMean==maxScore && falseVariance==1.0)
                falseMean=spotting.score;
            maxScore=spotting.score;
        }
        if (spotting.score<minScore)
        {
            if (trueMean==minScore && trueVariance==1.0)
                trueMean=spotting.score;
            minScore=spotting.score;
        }

        if (spotting.type==SPOTTING_TYPE_TRANS_TRUE)
        {
            classById[spotting.id]=true;
            numberClassifiedTrue++;
            assert(spotting.pageId>=0);
            instancesById[spotting.id]=spotting;
            instancesByLocation.insert(&instancesById.at(spotting.id));
        }
        else if (spotting.type==SPOTTING_TYPE_TRANS_FALSE)
        {
            classById[spotting.id]=false;
            numberClassifiedFalse++;
            assert(spotting.pageId>=0);
            instancesById[spotting.id]=spotting;
            instancesByLocation.insert(&instancesById.at(spotting.id));
        }
        else
        {

            //Scan for possibly overlapping (the same) spottings
            Spotting* best=NULL;
            float ratioOff;//1 at threshold, 0 exactly the same
            auto bestIter = findOverlap(spotting, &ratioOff);
            if (bestIter != instancesByLocation.end())
                best=*bestIter;
            if (best!=NULL)
            {
                //Zagoris et al. A Framework for Efficient Transcription of Historical Documents Using Keyword Spotting would indicate that taking the worse (max) score would yield the best combintation results.
                //However, we choose to do a weighted averaging based on how far spatially the spottings are from one another.
                //If they are percisely on the same location, we take the max.
                //If they are maximally off (as allowed by threshold), the min score (selected spotting) is used.
                //Interpolate between
                //float worseScore = max(spotting.score, (best)->score);
                //float bestScore = min(spotting.score, (best)->score);
                //float combScore = (1.0f-ratioOff)*worseScore + (ratioOff)*bestScore;

                //Actually, averaging seems to do better
                float combScore = (spotting.score + (best)->score)/2.0f;
                //prevent incorrect changes to score
                auto iii = classById.find(best->id);
                if (iii != classById.end())
                {
                    if ( (iii->second && combScore>(best)->score) ||
                            ((!iii->second) && combScore<(best)->score)
                       )
                    {
                        combScore = (best)->score;
                    }
                }

                if (spotting.score < (best)->score)//then replace the spotting, this happens to skip NaN in the case of a harvested exemplar
                {
                    ///debugState();
                    spotting.score = combScore;
                    bool updateWhenInBatch = (best)->type!=SPOTTING_TYPE_THRESHED && instancesByScore.find(best->id)==instancesByScore.end();
                    if (updateWhenInBatch)
                        updateMap[best->id]=spotting.id;
#ifdef TEST_MODE
                    else
                        testUpdateMap[best->id]=spotting.id;
#endif
                    //Add this spotting
                    assert(spotting.pageId>=0);
                    instancesById[spotting.id]=spotting;
                    instancesByLocation.insert(&instancesById.at(spotting.id));
                    instancesByLocation.erase(bestIter); //erase by iterator

                    //remove the old one
                    int removed = instancesByScore.erase(best->id); //erase by value (pointer)
                    if (!removed)
                        assert(instancesByScore.find(best->id)==instancesByScore.end());
                    if (removed)
                    {
                        //add
                        instancesByScore.insert(spotting.id);
                        tracer = instancesByScore.begin();
                    }
                    else if ((best)->type==SPOTTING_TYPE_THRESHED && classById[(best)->id]==false)
                    {//This occurs after being resurrected. We'll give a better score another chance.
                        //cout<<"{} replaced false spotting "<<spotting.score<<endl;
                        //(best)->type=SPOTTING_TYPE_NONE;
                        classById.erase((best)->id);
                        instancesByScore.insert(spotting.id);
                        tracer = instancesByScore.begin();
                    }
                    instancesById.erase(best->id);
                    ///debugState();
                }
                else
                {
                    //becuase we're changing what its indexed by, we need to readd it
                    int removed = instancesByScore.erase(best->id);
                    (best)->score = combScore;
                    if (removed)
                        instancesByScore.insert(best->id);
                }
                //Zagoris et al. A Framework for Efficient Transcription of Historical Documents Using Keyword Spotting would indicate that taking the worse (max) score would yield the best combintation results.
                //However, we choose to do a weighted averaging based on how far spatially the spottings are from one another.
                //If they are percisely on the same location, we take the max.
                //If they are maximally off (as allowed by threshold), the min score (selected spotting) is used.
                //Interpolate between
            }
            else
            {
                assert(spotting.pageId>=0);
                instancesById[spotting.id]=spotting;
                instancesByScore.insert(spotting.id);
                instancesByLocation.insert(&instancesById.at(spotting.id));
                tracer = instancesByScore.begin();
            }
        }
    }
    delete spottings;
    bool allWereSent = allBatchesSent;
    acceptThreshold=-1;
    rejectThreshold=-1;
    EMThresholds();
    if (allWereSent && !allBatchesSent)
    {
#ifdef TEST_MODE_LONG
        cout <<"Should ressurect spottingresults: "<<ngram<<endl;
#endif
        //RESURRECT!
        allBatchesSent=false;
        return true;
    }
        //Go through spottings
        //If intersection, use previous label and label it instantly, but use the given score, dont put in "queue" (byScores)
        //Else, add it in normally
        //All non-intersected old ones need to have their score not count anymore (but save in case of later resurrection)
    ///debugState();
    return false;
}

void SpottingResults::save(ofstream& out)
{
    ///debugState();
    out<<"SPOTTINGRESULTS"<<endl;
    out<<ngram<<"\n";
    out<<batchesSinceChange<<"\n";
    out<<numberClassifiedTrue<<"\n";
    out<<numberClassifiedFalse<<"\n";
    out<<numberAccepted<<"\n";
    out<<numberRejected<<"\n";
    out<<_id.load()<<"\n";
    out<<id<<"\n";
    out<<acceptThreshold<<"\n"<<rejectThreshold<<"\n";
    out<<allBatchesSent<<"\n"<<done<<"\n";
    out<<trueMean<<"\n"<<trueVariance<<"\n";
    out<<falseMean<<"\n"<<falseVariance<<"\n";
    out<<lastDifPullFromScore<<"\n"<<momentum<<"\n";
    out<<pullFromScore<<"\n";
    out<<maxScore<<"\n"<<minScore<<"\n";
    out<<numLeftInRange<<"\n";

    out<<instancesById.size()<<"\n";
    for (auto& p : instancesById)
    {
        assert(p.second.tlx!=-1);
        p.second.save(out);
    }

    out<<instancesByScore.size()+starts.size()<<"\n";
    out.flush();
    for (unsigned long sid : instancesByScore)
    {
        //unsigned long sid = s->id;
        if (instancesById.find(sid) == instancesById.end())
        {
            if (updateMap.find(sid) != updateMap.end())
                sid=updateMap.at(sid);
            else
            {
                //for (auto& p : instancesById)
                //{
                //    if (&(instancesById[p.first]) == s)
                //        assert(false && "SpottingResults::save ran into non-existant spotting id, though pointer exists in instancesByScore.");
                //}
                assert(false && "SpottingResults::save ran into non-existant spotting id (in instancesByScore)");
            }
        }
        out<<sid<<"\n";
        out.flush();
    }
    for (auto& p : starts)
    {
        unsigned long sid = p.first;
        if (instancesById.find(sid) == instancesById.end())
        {
            if (updateMap.find(sid) != updateMap.end())
                sid=updateMap.at(p.first);
            else
                assert(false && "SpottingResults::save ran into non-existant spotting id (in starts)");
        }
        out<<sid<<"\n";
    }

    out<<classById.size()<<"\n";
    for (auto p : classById)
    {
        out<<p.first<<"\n";
        out<<p.second<<"\n";
    }
    //skip updateMap as no feedback will be recieved.
    //skip tracer as we will just refind it
    out<<contextPad<<"\n";
    ///debugState();
}

SpottingResults::SpottingResults(ifstream& in, PageRef* pageRef) :
    instancesByScore(scoreCompById(&(this->instancesById)))
{
    string line;
    getline(in,line);
    assert(line.compare("SPOTTINGRESULTS")==0);
    getline(in,ngram);
    getline(in,line);
    batchesSinceChange = stoi(line);
    getline(in,line);
    numberClassifiedTrue = stoi(line);
    getline(in,line);
    numberClassifiedFalse = stoi(line);
    getline(in,line);
    numberAccepted = stoi(line);
    getline(in,line);
    numberRejected = stoi(line);
    getline(in,line);
    _id.store(stoul(line));
    getline(in,line);
    id = stoul(line);
    getline(in,line);
    acceptThreshold = stod(line);
    getline(in,line);
    rejectThreshold = stod(line);
    getline(in,line);
    allBatchesSent = stoi(line);
    getline(in,line);
    done = stoi(line);
    getline(in,line);
    trueMean = stod(line);
    getline(in,line);
    trueVariance = stod(line);
    getline(in,line);
    falseMean = stod(line);
    getline(in,line);
    falseVariance = stod(line);
    getline(in,line);
    lastDifPullFromScore = stod(line);
    getline(in,line);
    momentum = stod(line);
    getline(in,line);
    pullFromScore = stod(line);
    getline(in,line);
    maxScore = stod(line);
    getline(in,line);
    minScore = stod(line);
    getline(in,line);
    numLeftInRange = stoi(line);

    getline(in,line);
    int size = stoi(line);
    for (int i=0; i<size; i++)
    {
        Spotting s(in,pageRef);
        assert(pageRef->verify(s.pageId,s.tlx,s.tly,s.brx,s.bry));
        instancesById[s.id]=s;
        instancesByLocation.insert(&instancesById.at(s.id));
    }
    getline(in,line);
    size = stoi(line);
    for (int i=0; i<size; i++)
    {
        getline(in,line);
        unsigned long sid = stoul(line);
        instancesByScore.insert(sid);
    }
    getline(in,line);
    size = stoi(line);
    for (int i=0; i<size; i++)
    {
        getline(in,line);
        unsigned long sid = stoul(line);
        getline(in,line);
        classById[sid] = stoi(line);
    }
    tracer = instancesByScore.begin();
    getline(in,line);
    contextPad = stoi(line);
#ifdef GRAPH_SPOTTING_RESULTS
    undoneGraphName="save/graph/graph_undone_"+ngram+".png";
    //cv::namedWindow(undoneGraphName);
    fullGraphName="save/graph/graph_full_"+ngram+".png";
#endif
#ifdef TEST_MODE
    rtn=0;
    atn=0;
#endif //TEST_MODE
    ///debugState();
    
}
