#include "SpottingResults.h"

#include <ctime>


unsigned long Spotting::_id=0;
unsigned long SpottingsBatch::_batchId=0;
unsigned long SpottingResults::_id=0;

SpottingResults::SpottingResults(string ngram) : 
    ngram(ngram)
{
    id = ++_id;
    //sem_init(&mutexSem,false,1);
    //numBatches=0;
    allBatchesSent=false;
    
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
    
    //pullFromScore=splitThreshold;
    delta=0;
    //haveRe
}

void SpottingResults::add(Spotting spotting) {
    //sem_wait(&mutexSem);
    instancesById[spotting.id]=spotting;
    assert(spotting.score==spotting.score);
    instancesByScore.insert(&instancesById[spotting.id]);
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

SpottingsBatch* SpottingResults::getBatch(bool* done, unsigned int num, bool hard, unsigned int maxWidth, int color, string prevNgram) {
    //cout <<"getBatch, from:"<<pullFromScore<<endl;
    if (acceptThreshold==-1 && rejectThreshold==-1)
        EMThresholds(true);
    SpottingsBatch* ret = new SpottingsBatch(ngram,id);
    //sem_wait(&mutexSem);
    
    unsigned int toRet = (hard||((((signed int)instancesByScore.size())-(signed int) num)>3))?num:instancesByScore.size();
    if (toRet==0)
        return ret;
    
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

vector<Spotting>* SpottingResults::feedback(bool* done, const vector<string>& ids, const vector<int>& userClassifications, int resent, vector<pair<unsigned long,string> >* retRemove)
{
    /*cout << "fed: ";
    for (int i=0; i<ids.size(); i++)
    {   
        cout << ids[i]<<", ";
    }
    cout<<endl;*/
    
    vector<Spotting>* ret = new vector<Spotting>();
    
    for (unsigned int i=0; i< ids.size(); i++)
    {
        unsigned long id = stoul(ids[i]);
        starts.erase(id);
        if (resent)
        {
            if (classById[id])
                numberClassifiedTrue--;
            else
                numberClassifiedFalse--;
        }
        
        // adjust threshs
        if (userClassifications[i]>0)
        {
            numberClassifiedTrue++;
            if (!resent || !classById[id])
            {
                instancesById.at(id).type=SPOTTING_TYPE_APPROVED;
                ret->push_back(instancesById.at(id)); //otherwise we've already sent it
            }
            classById[id]=true;
        }
        else if (userClassifications[i]==0)
        {
            numberClassifiedFalse++;
            if (!resent || (retRemove && !classById[id]))
            {
                instancesById.at(id).type=SPOTTING_TYPE_NONE;
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
    EMThresholds();
    
    ///test
    cout<<"["<<id<<"]:"<<ngram<<", all sent: "<<allBatchesSent<<", waiting for "<<starts.size()<<", num left "<<instancesByScore.size()<<endl;
    ///test

    if (resent==0 && starts.size()==0 && allBatchesSent)
    {
        *done=true;
        //cout <<"all batches sent, cleaning up"<<endl;
        
        tracer = instancesByScore.begin();
        while (tracer!=instancesByScore.end() && (*tracer)->score < acceptThreshold)
        {
            (**tracer).type=SPOTTING_TYPE_THRESHED;
            ret->push_back(**tracer);
            tracer++;
            numberAccepted++;
        }
        //cout << "hit end "<<(tracer==instancesByScore.end())<<endl;
        numberRejected = distance(tracer,instancesByScore.end());
        
    }
    else 
    {
        
    }
    return ret;
}
    
   
    

void SpottingResults::EMThresholds(bool init)
{
    float oldMidScore = acceptThreshold + (rejectThreshold-acceptThreshold)/2.0;
    /*This will likely predict very narrow and distinct distributions
     *initailly. This should be fine as we sample from the middle of
     *the thresholds outward.
     */
     
    //test
    int displayLen=90;
    vector<int> histogramCP(displayLen);
    vector<int> histogramCN(displayLen);
    vector<int> histogramGP(displayLen);
    vector<int> histogramGN(displayLen);
    //test
    
    if (init)
    {
        //we initailize our split with Otsu, as we are assuming to distributions.
        //make histogram
        vector<int> histogram(256);
        for (auto p : instancesById)
        {
            unsigned long id = p.first;
            int bin = 255*(instancesById.at(id).score-minScore)/(maxScore-minScore);
            if (bin<0) bin=0;
            if (bin>histogram.size()-1) bin=histogram.size()-1;
            histogram[bin]++;
            
        }
        
        //otsu
        int total = instancesById.size();
        double sum =0;
        for (int i = 1; i < 256; ++i)
                sum += i * histogram[i];
        double sumB = 0;
        double wB = 0;
        double wF = 0;
        double mB;
        double mF;
        double max = 0.0;
        double between = 0.0;
        double threshold1 = 0.0;
        double threshold2 = 0.0;
        for (int i = 0; i < 256; ++i)
        {
            wB += histogram[i];
            if (wB == 0)
                continue;
            wF = total - wB;
            if (wF == 0)
                break;
            sumB += i * histogram[i];
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
        pullFromScore = (thresh/256)*(maxScore-minScore)+minScore;
    }
    
    //expectation
    //bool initV=init;
    //while (1)
    //{
        for (int i=0; i<displayLen; i++)
        {
            histogramCP[i]=0;
            histogramGP[i]=0;
            histogramCN[i]=0;
            histogramGN[i]=0;;
        }
        
        //map<unsigned long, bool> expected;
        vector<float> expectedTrue;
        float sumTrue=0;
        vector<float> expectedFalse;
        float sumFalse=0;
        
        
        
        
        
        for (auto p : instancesById)
        {
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
        acceptThreshold = max( min(acceptThreshold1,acceptThreshold2), minScore);
        rejectThreshold = min( min(rejectThreshold1,rejectThreshold2), maxScore);
        
        /*if (!init || !initV)
            break;
        
        if (fabs(pullFromScore-trueMean)<0.05)
            break;
        else
            initV=false;
        pullFromScore = trueMean;*/
    //}
    pullFromScore = trueMean-sqrt(trueVariance);
    
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
    
}


bool SpottingResults::checkIncomplete()
{
    bool incomp=false;
    //cout <<"checkIncomplete, starts is "<<starts.size()<<endl;
    vector<unsigned long> toRemove;
    for (auto start : starts)
    {
        chrono::system_clock::duration d = chrono::system_clock::now()-start.second;
        chrono::minutes pass = chrono::duration_cast<chrono::minutes> (d);
        //cout<<pass.count()<<" minutes has past for "<<(start.first)<<endl;
        if (pass.count() > 20) //if 20 mins has passed
        {
            instancesByScore.insert(&instancesById[start.first]);
            tracer = instancesByScore.begin();
            incomp=true;
            toRemove.push_back(start.first);
            
        }
    }
    for (unsigned long id : toRemove)
        starts.erase(id);
    if (incomp && allBatchesSent)
    {
        allBatchesSent=false;
        return true;
    }
    return false;
}

//combMin
void SpottingResults::updateSpottings(vector<Spotting>* spottings)
{
    for (Spotting& spotting : *spottings)
    {
        bool updated=false;
        int width = spotting.brx-spotting.tlx;
        int height = spotting.bry-spotting.tly;
        for (int tlx=spotting.tlx-width*(1-UPDATE_OVERLAP_THRESH); tlx<spotting.tlx+width*(1-UPDATE_OVERLAP_THRESH); tlx++)
        {
            Spotting lb(tlx,spotting.tly-height*(1-UPDATE_OVERLAP_THRESH));
            Spotting ub(tlx,spotting.tly+height*(1-UPDATE_OVERLAP_THRESH));
            auto itLow = instancesByLocation.lower_bound(&lb);
            auto itHigh = instancesByLocation.upper_bound(&ub);
            for (;itLow!=itHigh; itLow++)
            {
                int overlapArea = ( min(spotting.brx,(*itLow)->brx) - max(spotting.tlx,(*itLow)->tlx) ) * ( min(spotting.bry,(*itLow)->bry) - max(spotting.tly,(*itLow)->tly) );
                if (overlapArea/(0.0+ (spotting.brx-spotting.tlx)*(spotting.bry-spotting.tly))>UPDATE_OVERLAP_THRESH)
                {
                    updated=true;

                    if (spotting.score < (*itLow)->score)//then replace the spotting
                    {
                        //Remove from scores
                        instancesByScore.erase(*itLow); //erase by value (pointer)
                        //Remove from locations
                        instancesByLocation.erase(itLow); //erase by interator
                        //add
                        instancesById[spotting.id]=spotting;
                        instancesByScore.insert(&instancesById[spotting.id]);
                        instancesByLocation.insert(&instancesById[spotting.id]);
                        tracer = instancesByScore.begin();
                    }
                    tlx=spotting.tlx+width*(1-UPDATE_OVERLAP_THRESH);
                    break;
                }
            }
        }

        if (!updated)
        {
            instancesById[spotting.id]=spotting;
            instancesByScore.insert(&instancesById[spotting.id]);
            instancesByLocation.insert(&instancesById[spotting.id]);
            tracer = instancesByScore.begin();
        }
    }
    delete spottings;
    EMThresholds();
}
