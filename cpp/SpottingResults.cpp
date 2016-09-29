#include "SpottingResults.h"

#include <ctime>


atomic_ulong Spotting::_id;
atomic_ulong SpottingResults::_id;

SpottingResults::SpottingResults(string ngram) : 
    ngram(ngram)
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
    
    done=false;

    //pullFromScore=splitThreshold;
    delta=0;
    //haveRe
    momentum=0.8;
    lastDifPullFromScore=0;
}

void SpottingResults::add(Spotting spotting) {
    //sem_wait(&mutexSem);
    allBatchesSent=false;
    instancesById[spotting.id]=spotting;
    assert(spotting.score==spotting.score);
    if (spotting.type==SPOTTING_TYPE_TRANS_TRUE)
    {
        classById[spotting.id]=true;
        numberClassifiedTrue++;
    }
    else if (spotting.type==SPOTTING_TYPE_TRANS_FALSE)
    {
        classById[spotting.id]=false;
        numberClassifiedFalse++;
    }
    else
    {
        instancesByScore.insert(&instancesById[spotting.id]);
    }
    instancesByLocation.insert(&instancesById[spotting.id]);
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
}
void SpottingResults::addTrueNoScore(const SpottingExemplar& spotting) {
    //sem_wait(&mutexSem);
    assert(spotting.score != spotting.score);
    instancesById[spotting.id]=spotting;
    instancesById[spotting.id].type=SPOTTING_TYPE_APPROVED;
    instancesByLocation.insert(&instancesById[spotting.id]);
    classById[spotting.id]=true;
    //sem_post(&mutexSem);
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

SpottingsBatch* SpottingResults::getBatch(bool* done, unsigned int num, bool hard, unsigned int maxWidth, int color, string prevNgram, bool need) {

    if (!need && numLeftInRange<12 && starts.size()>1)
        return NULL;

    if (acceptThreshold==-1 && rejectThreshold==-1)
        EMThresholds();
#ifdef TEST_MODE
    cout <<"\ngetBatch, from:"<<pullFromScore<<"\n"<<endl;
#endif
    SpottingsBatch* ret = new SpottingsBatch(ngram,id);
    //sem_wait(&mutexSem);
    
    unsigned int toRet = ((hard&&instancesByScore.size()>=num)||((((signed int)instancesByScore.size())-(signed int) num)>3))?num:instancesByScore.size();
    if (toRet==0)
    {
        //This occurs in a race condition when the spotting results is queried before it can be removed from the MasterQueue
        return NULL;
    }
    
    if ((*tracer)->score < pullFromScore)
        while(tracer!=instancesByScore.end() && (*tracer)->score<=pullFromScore)
            tracer++;
    else
        while((*tracer)->score>=pullFromScore && tracer!=instancesByScore.begin())
            tracer--;
    
    if ((tracer!=instancesByScore.end() && (*tracer)->score>=rejectThreshold) && tracer!=instancesByScore.begin())
        tracer--;
    
    //*The commented out regoins seem prinicpled, but empirical results showed that it was better to leave them out
    //*if ((*tracer)->score<=acceptThreshold)
    //*    tracer++;
    if (tracer==instancesByScore.end())
        tracer--;
    
    for (unsigned int i=0; i<toRet && !*done; i++) {
        SpottingImage tmp(**tracer,maxWidth,color,prevNgram);
        ret->push_back(tmp);
        
        tracer = instancesByScore.erase(tracer);
        if (instancesByScore.size()==0)
        {
            *done=true;
        }
        
        if ((i%2==0 || tracer==instancesByScore.end() || (*tracer)->score>=rejectThreshold) && tracer!=instancesByScore.begin() )//*&& (*tracer)->score>acceptThreshold)
        {
            tracer--;
        }
        
        
    }
    
    if (!*done)
    {
        //cout<<"i'm "<<(*tracer)->score<<", accept "<<acceptThreshold<<", reject "<<rejectThreshold<<endl;
        if ((*tracer)->score>=rejectThreshold)
        {
            if (tracer==instancesByScore.begin())
            {
                *done=true;
            }
            else
            {
                while((*tracer)->score>=rejectThreshold && tracer!=instancesByScore.begin())
                    tracer--;
                if ((*tracer)->score<=acceptThreshold || (*tracer)->score>=rejectThreshold)
                {
                    *done=true;
                }
            }
            //cout<<"now i'm "<<(*tracer)->score<<", accept "<<acceptThreshold<<", reject "<<rejectThreshold<<endl;
        }
        else  if ((*tracer)->score<=acceptThreshold)
        {
            while((*tracer)->score<=acceptThreshold && tracer!=instancesByScore.end())
                tracer++;
            if (tracer==instancesByScore.end() || (*tracer)->score>=rejectThreshold)
            {
                *done=true;
            }
            //cout<<"now i'm "<<(*tracer)->score<<", accept "<<acceptThreshold<<", reject "<<rejectThreshold<<endl;
        }
    }
    
    
    
    if (*done)
        allBatchesSent=true;
    
    //cout<<"batch is "<<ret->size()<<endl;
    //cout << "send: ";
    for (int i=0; i<ret->size(); i++)
    {   
        starts[ret->at(i).id] = chrono::system_clock::now();
        
        //time_t tt= chrono::system_clock::to_time_t ( starts[ret->at(i).id] );
        //cout << ret->at(i).id<<", ";//" at time "<< ctime(&tt)<<endl;
    }
        //cout <<endl;
    //sem_post(&mutexSem);
    //numBatches++;
    
    return ret;
}

vector<Spotting>* SpottingResults::feedback(int* done, const vector<string>& ids, const vector<int>& userClassifications, int resent, vector<pair<unsigned long,string> >* retRemove)
{
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
        
        instancesById.at(id).type=SPOTTING_TYPE_APPROVED;
        // adjust threshs
        if (userClassifications[i]>0)
        {
            swing++;
            numberClassifiedTrue++;
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
            if (resent && (retRemove && iterClass->second))
            {
                retRemove->push_back(make_pair(id,instancesById.at(id).ngram));
            }
            classById[id]=false;
        }
        else if (!resent)//someone passed, so we need to add it again, unless this is resent, in whichcase we use the old.
        { 
            instancesByScore.insert(&instancesById[id]);
            tracer = instancesByScore.begin();
        }
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
#endif
        tracer = instancesByScore.begin();
        while (tracer!=instancesByScore.end() && (*tracer)->score <= acceptThreshold)
        {
            (**tracer).type=SPOTTING_TYPE_THRESHED;
            ret->push_back(**tracer);
            classById[(**tracer).id]=true;
            numberAccepted++;
#ifdef TEST_MODE_LONG
            cout <<" "<<(**tracer).id<<"[tlx:"<<(**tracer).tlx<<", score:"<<(**tracer).score<<"]: true"<<endl;
#endif
            tracer++;
        }
        while (tracer!=instancesByScore.end())
        {
            (**tracer).type=SPOTTING_TYPE_THRESHED;
            classById[(**tracer).id]=false;
#ifdef TEST_MODE_LONG
            cout <<" "<<(**tracer).id<<"[tlx:"<<(**tracer).tlx<<", score:"<<(**tracer).score<<"]: false"<<endl;
#endif
            tracer++;
        }
        instancesByScore.clear();
        //cout << "hit end "<<(tracer==instancesByScore.end())<<endl;
        numberRejected = distance(tracer,instancesByScore.end());
        
    }
    else if (!allBatchesSent && allWereSent)
    {
        *done=-1;
#ifdef TEST_MODE
        cout<<"SpottingResults ["<<id<<"]:"<<ngram<<" has more batches and will be re-enqueued"<<endl;
#endif
    }
    return ret;
}
    
   
    

bool SpottingResults::EMThresholds(int swing)
{
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
            
            //test
            int bin=(displayLen-1)*(instancesById.at(id).score-minScore)/(maxScore-minScore);
            
            if (classById.find(id)!=classById.end())
            {
                if (classById[id])
                {
                    expectedTrue.push_back(instancesById.at(id).score);
                    expectedTrue.push_back(instancesById.at(id).score);
                    sumTrue+=2*instancesById.at(id).score;
                    
                    //test
                    if (bin>=0) histogramCP.at(bin)++;
                }
                else
                {
                    expectedFalse.push_back(instancesById.at(id).score);
                    expectedFalse.push_back(instancesById.at(id).score);
                    sumFalse+=2*instancesById.at(id).score;
                    
                    //test
                    if (bin>=0) histogramCN.at(bin)++;
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
                    
                    //test
                    if (bin>=0) histogramGP.at(bin)++;
                }
                else
                {
                    expectedFalse.push_back(instancesById.at(id).score);
                    sumFalse+=instancesById.at(id).score;
                    
                    //test
                    if (bin>=0) histogramGN.at(bin)++;
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
        float rejectThreshold2 = falseMean+numStdDevs*sqrt(falseVariance);
        //float prevAcceptThreshold=acceptThreshold;
        //float prevRejectThreshold=rejectThreshold;
        if (falseVariance!=0 && trueVariance!=0)
        {
            acceptThreshold = max( min(acceptThreshold1,acceptThreshold2), minScore);
            rejectThreshold = min( min(rejectThreshold1,rejectThreshold2), maxScore);
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
    pullFromScore = trueMean-sqrt(trueVariance);
    if (!init)
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
        {  ???
            pullFromScore= (swing/10.0)*fabs(difPullFromScore+lastDifPullFromScore);
        }
        difPullFromScore = pullFromScore-prevPullFromScore;
#ifdef TEST_MODE
        cout<<"final pull dif="<<difPullFromScore<<",  swing="<<swing<<",  lastDif="<<lastDifPullFromScore<<endl;
#endif
        lastDifPullFromScore=difPullFromScore;
    }

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
        auto iter = tracer;
        while (iter!=instancesByScore.end())
        {
            if ((**iter).score > acceptThreshold && (**iter).score < rejectThreshold)
            {
                allBatchesSent=false;
                break;
            }
            if (iter==instancesByScore.begin() || (**iter).score>acceptThreshold)
                break;
            iter--;
        } 
        if (allBatchesSent)
        {
            iter = tracer;
            while (iter!=instancesByScore.end() && (**iter).score < rejectThreshold)
            {
                if ((**iter).score > acceptThreshold && (**iter).score < rejectThreshold)
                {
                    allBatchesSent=false;
                    break;
                }
                iter++;
            } 
        }
    }
    return allBatchesSent;
}


bool SpottingResults::checkIncomplete()
{
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
            instancesByScore.insert(&instancesById[start.first]);
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
    return false;
}


multiset<Spotting*,tlComp>::iterator SpottingResults::findOverlap(const Spotting& spotting) const
{
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
        Spotting lb(tlx,spotting.tly-height*(1-UPDATE_OVERLAP_THRESH));
        Spotting ub(tlx,spotting.tly+height*(1-UPDATE_OVERLAP_THRESH));
        auto itLow = instancesByLocation.lower_bound(&lb);
        auto itHigh = instancesByLocation.upper_bound(&ub);


        for (;itLow!=itHigh; itLow++)
        {
            int overlapArea = ( min(spotting.brx,(*itLow)->brx) - max(spotting.tlx,(*itLow)->tlx) ) * ( min(spotting.bry,(*itLow)->bry) - max(spotting.tly,(*itLow)->tly) );
            double thresh = UPDATE_OVERLAP_THRESH;
            bool updateWhenInBatch = (*itLow)->type!=SPOTTING_TYPE_THRESHED && instancesByScore.find(*itLow)==instancesByScore.end();
            if (updateWhenInBatch)
                thresh=UPDATE_OVERLAP_THRESH_TIGHT;
            if (overlapArea/spottingArea > thresh)
            {
                if (overlapArea > bestOverlap)
                {
                    bestOverlap=overlapArea;
                    //bestSoFar=*itLow;
                    bestSoFarIter=itLow;
                }
                else
                {
                    tlx=spotting.tlx+width*(1-UPDATE_OVERLAP_THRESH);
                    break;
                }


                
            }
        }
    }
    return bestSoFarIter;
}

//This method is to check to see if we actually have this exemplar already and then prevent is from being re-approved
void SpottingResults::updateSpottingTrueNoScore(const SpottingExemplar& spotting)
{
    assert(spotting.score != spotting.score);

    //Scan for possibly overlapping (the same) spottings
    Spotting* best=NULL;
    auto bestIter = findOverlap(spotting);
    if (bestIter != instancesByLocation.end())
        best=*bestIter;
    if (best!=NULL)
    {

        bool updateWhenInBatch = (best)->type!=SPOTTING_TYPE_THRESHED && instancesByScore.find(best)==instancesByScore.end();
        if (updateWhenInBatch)
            updateMap[best->id]=spotting.id;
#ifdef TEST_MODE
        else
            testUpdateMap[best->id]=spotting.id;
#endif
        //Add this spotting
        instancesById[spotting.id]=spotting;
        instancesById[spotting.id].type=SPOTTING_TYPE_APPROVED;
        instancesById[spotting.id].score=best->score;//we replace the score so that we contribute to the thresholding
        instancesByLocation.insert(&instancesById[spotting.id]);
        classById[spotting.id]=true;
        instancesByLocation.erase(bestIter); //erase by iterator

        //remove the old one
        int removed = instancesByScore.erase(best); //erase by value (pointer)
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
}

//combMin
bool SpottingResults::updateSpottings(vector<Spotting>* spottings)
{
    for (Spotting& spotting : *spottings)
    {
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
            instancesById[spotting.id]=spotting;
            instancesByLocation.insert(&instancesById[spotting.id]);
        }
        else if (spotting.type==SPOTTING_TYPE_TRANS_FALSE)
        {
            classById[spotting.id]=false;
            numberClassifiedFalse++;
            instancesById[spotting.id]=spotting;
            instancesByLocation.insert(&instancesById[spotting.id]);
        }
        else
        {

            //Scan for possibly overlapping (the same) spottings
            Spotting* best=NULL;
            auto bestIter = findOverlap(spotting);
            if (bestIter != instancesByLocation.end())
                best=*bestIter;
            if (best!=NULL)
            {

                if (spotting.score < (best)->score)//then replace the spotting, this happens to skip NaN in the case of a harvested exemplar
                {
                    bool updateWhenInBatch = (best)->type!=SPOTTING_TYPE_THRESHED && instancesByScore.find(best)==instancesByScore.end();
                    if (updateWhenInBatch)
                        updateMap[best->id]=spotting.id;
#ifdef TEST_MODE
                    else
                        testUpdateMap[best->id]=spotting.id;
#endif
                    //Add this spotting
                    instancesById[spotting.id]=spotting;
                    instancesByLocation.insert(&instancesById[spotting.id]);
                    instancesByLocation.erase(bestIter); //erase by iterator

                    //remove the old one
                    int removed = instancesByScore.erase(best); //erase by value (pointer)
                    if (removed)
                    {
                        //add
                        instancesByScore.insert(&instancesById[spotting.id]);
                        tracer = instancesByScore.begin();
                    }
                    else if ((best)->type==SPOTTING_TYPE_THRESHED && classById[(best)->id]==false)
                    {//This occurs after being resurrected. We'll give a better score another chance.
                        //cout<<"{} replaced false spotting "<<spotting.score<<endl;
                        //(best)->type=SPOTTING_TYPE_NONE;
                        classById.erase((best)->id);
                        instancesByScore.insert(&instancesById[spotting.id]);
                        tracer = instancesByScore.begin();
                    }
                    instancesById.erase(best->id);
                }
            }
            else
            {
                instancesById[spotting.id]=spotting;
                instancesByScore.insert(&instancesById[spotting.id]);
                instancesByLocation.insert(&instancesById[spotting.id]);
                tracer = instancesByScore.begin();
            }
        }
    }
    delete spottings;
    bool allWereSent = allBatchesSent;
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
    return false;
}
