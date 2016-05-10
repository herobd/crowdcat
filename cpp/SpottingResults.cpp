#include "SpottingResults.h"



unsigned long Spotting::_id=0;
unsigned long SpottingsBatch::_batchId=0;
unsigned long SpottingResults::_id=0;

SpottingResults::SpottingResults(string ngram, double splitThreshold, double momentumTerm) : 
    ngram(ngram), momentumTerm(momentumTerm)
{
    id = _id++;
    //sem_init(&mutexSem,false,1);
    numBatches=0;
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
    
    pullFromScore=splitThreshold;
    delta=0;
    //haveRe
}

void SpottingResults::add(Spotting spotting) {
    //sem_wait(&mutexSem);
    instancesById[spotting.id]=spotting;
    instancesByScore.insert(&instancesById[spotting.id]);
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
SpottingsBatch* SpottingResults::getBatch(bool* done, unsigned int num, unsigned int maxWidth) {
    cout <<"getBatch"<<endl;
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
    return ret;
}

vector<Spotting>* SpottingResults::feedback(bool* done, const vector<string>& ids, const vector<int>& userClassifications)
{
    
    
    vector<Spotting>* ret = new vector<Spotting>();
    
    for (unsigned int i=0; i< ids.size(); i++)
    {
        
        unsigned long id = stoul(ids[i]);
        
        // adjust threshs
        if (userClassifications[i])
        {
            numberClassifiedTrue++;
            classById[id]=true;
            ret->push_back(instancesById[id]);
        }
        else
        {
            numberClassifiedFalse++;
            classById[id]=false;
        }
    }
    EMThresholds();
    
    
    if (--numBatches==0 && allBatchesSent)
    {
        *done=true;
        //cout <<"all batches sent, cleaning up"<<endl;
        
        tracer = instancesByScore.begin();
        while (tracer!=instancesByScore.end() && (*tracer)->score < acceptThreshold)
        {
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
    
   
    
SpottingImage SpottingResults::getNextSpottingImage(bool* done, int maxWidth)
{
    //cout <<"getNextSpottingImage"<<endl;
    //float midScore = acceptThreshold + (rejectThreshold-acceptThreshold)/2.0;
    float midScore = pullFromScore;
    if ((*tracer)->score < midScore)
        while(tracer!=instancesByScore.end() && (*tracer)->score<midScore)
            tracer++;
    else
        while((*tracer)->score>midScore && tracer!=instancesByScore.begin())
            tracer--;
    
    //cout <<"1 getNextSpottingImage"<<endl;
    
    if(tracer==instancesByScore.end())
        tracer--;
    
    if ((*tracer)->score > rejectThreshold && tracer!=instancesByScore.begin())
        tracer--;
    
    //cout <<"2 getNextSpottingImage: "<<*tracer<<endl;
    
    SpottingImage toRet(**tracer,maxWidth);
    //cout <<"2.25 getNextSpottingImage"<<endl;
    
    tracer = instancesByScore.erase(tracer);
    //cout <<"2.5 getNextSpottingImage"<<endl;
    if (instancesByScore.size()==0)
    {
        *done=true;
        return toRet;
    }
    
    //cout <<"3 getNextSpottingImage"<<endl;
    
    if (tracer != instancesByScore.begin())
    {
        tracer--;
    }
    
    
    if (tracer == instancesByScore.end())
    {
        *done=true;
        return toRet;
    }
    //cout <<"4 getNextSpottingImage"<<endl;
    
    if ((*tracer)->score < acceptThreshold)
        tracer++;
    
    //cout <<"5 getNextSpottingImage"<<endl;
    
    if (tracer==instancesByScore.end() || (*tracer)->score > rejectThreshold)
        *done = true;
    
    //cout <<"fin getNextSpottingImage"<<endl;
    return toRet;
}

void SpottingResults::EMThresholds(bool init)
{
    float oldMidScore = acceptThreshold + (rejectThreshold-acceptThreshold)/2.0;
    /*This will likely predict very narrow and ditinct distributions
     *initailly. This should be fine as we sample from the middle of
     *the thresholds outward.
     */
    //expectation
    
    //map<unsigned long, bool> expected;
    vector<float> expectedTrue;
    float sumTrue=0;
    vector<float> expectedFalse;
    float sumFalse=0;
    for (auto p : instancesById)
    {
        unsigned long id = p.first;
        if (classById.find(id)!=classById.end())
        {
            if (classById[id])
            {
                expectedTrue.push_back(instancesById[id].score);
                expectedTrue.push_back(instancesById[id].score);
                sumTrue+=2*instancesById[id].score;
            }
            else
            {
                expectedFalse.push_back(instancesById[id].score);
                expectedFalse.push_back(instancesById[id].score);
                sumFalse+=2*instancesById[id].score;
            }
        }
        else
        {
            double trueProb = exp(-1*pow(instancesById[id].score - trueMean,2)/(2*trueVariance));
            double falseProb = exp(-1*pow(instancesById[id].score - falseMean,2)/(2*falseVariance));
            if (init)
            {
                trueProb = instancesById[id].score<pullFromScore;
                falseProb = instancesById[id].score>pullFromScore;
            }
            if (trueProb>falseProb)
            {
                sumTrue+=instancesById[id].score;
                expectedTrue.push_back(instancesById[id].score);
            }
            else
            {
                expectedFalse.push_back(instancesById[id].score);
                sumFalse+=instancesById[id].score;
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
    acceptThreshold = min(acceptThreshold1,acceptThreshold2);
    rejectThreshold = max(rejectThreshold1,rejectThreshold2);
    cout <<"true mean "<<trueMean<<" true var "<<trueVariance<<endl;
    cout <<"false mean "<<falseMean<<" false var "<<falseVariance<<endl;
    
    /*cout <<"expected true: ";
    for (float v : expectedTrue)
        cout <<v<<", ";
    cout <<endl;
    cout <<"expected false: ";
    for (float v : expectedFalse)
        cout <<v<<", ";
    cout <<endl;*/
    cout <<"adjusted threshs, now "<<acceptThreshold<<" <> "<<rejectThreshold<<"    computed with std dev of: "<<numStdDevs<<endl;
    
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
    
    if (!init)
    {
        float newMidScore = acceptThreshold + (rejectThreshold-acceptThreshold)/2.0;
        cout<<"oldMid: "<<oldMidScore<<" newmid: "<<newMidScore<<endl;
        delta = newMidScore-oldMidScore + delta*momentumTerm;
        pullFromScore += delta;
        cout << "pullFromScore: "<<pullFromScore<<"   delta: "<<delta<<endl;
    }
}

