#include "SpottingResults.h"



unsigned long Spotting::_id=0;
unsigned long SpottingsBatch::_batchId=0;
unsigned long SpottingResults::_id=0;

SpottingResults::SpottingResults(string ngram, double acceptThreshold, double rejectThreshold) : 
    ngram(ngram), acceptThreshold(acceptThreshold), rejectThreshold(rejectThreshold)
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
    if (acceptThreshold==rejectThreshold)
        EMThresholds();
    SpottingsBatch* ret = new SpottingsBatch(ngram,id);
    //sem_wait(&mutexSem);
    unsigned int toRet = ((((signed int)instancesByScore.size())-(signed int) num)>3)?num:instancesByScore.size();
    
    for (unsigned int i=0; i<toRet && !*done; i++) {
        ret->push_back(getNextSpottingImage(done, maxWidth));
        
    }
    if (*done)
        allBatchesSent=true;
    //sem_post(&mutexSem);
    numBatches++;
    cout <<"sent batch, have "<<instancesByScore.size()<<" left"<<endl;
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
        
        tracer = instancesByScore.begin();
        while (tracer!=instancesByScore.end() && (*tracer)->score < acceptThreshold)
        {
            ret->push_back(**tracer);
            tracer++;
            numberAccepted++;
        }
        numberRejected = distance(tracer,instancesByScore.end());
        
    }
    return ret;
}
    
   
    
SpottingImage SpottingResults::getNextSpottingImage(bool* done, int maxWidth)
{
    float midScore = acceptThreshold + (rejectThreshold-acceptThreshold)/2.0;
    if ((*tracer)->score < midScore)
        while(tracer!=instancesByScore.end() && (*tracer)->score<midScore)
            tracer++;
    else
        while((*tracer)->score>midScore && tracer!=instancesByScore.begin())
            tracer--;
    
    if(tracer==instancesByScore.end())
        tracer--;
    
    if ((*tracer)->score > rejectThreshold && tracer!=instancesByScore.begin())
        tracer--;
    
    SpottingImage toRet(**tracer,maxWidth);
    tracer = instancesByScore.erase(tracer);
    if (instancesByScore.size()==0)
    {
        *done=true;
        return toRet;
    }
    
    if (tracer != instancesByScore.begin())
    {
        tracer--;
    }
    
    
    if (tracer == instancesByScore.end())
    {
        *done=true;
        return toRet;
    }
    
    if ((*tracer)->score < acceptThreshold)
        tracer++;
    if ((*tracer)->score > rejectThreshold)
        *done = true;
    
    
    return toRet;
}

void SpottingResults::EMThresholds()
{
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
                sumTrue+=instancesById[id].score;
            }
            else
            {
                expectedFalse.push_back(instancesById[id].score);
                sumFalse+=instancesById[id].score;
            }
        }
        else
        {
            double trueProb = exp(-1*pow(instancesById[id].score - trueMean,2)/(2*trueVariance));
            double falseProb = exp(-1*pow(instancesById[id].score - falseMean,2)/(2*falseVariance));
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
    float numStdDevs = 1 + (instancesById.size()/(0.0+classById.size()));
    acceptThreshold = falseMean-numStdDevs*sqrt(falseVariance);
    rejectThreshold = trueMean+numStdDevs*sqrt(trueVariance);
    cout <<"true mean "<<trueMean<<" true var "<<trueVariance<<endl;
    cout <<"false mean "<<falseMean<<" false var "<<falseVariance<<endl;
    cout <<"adjusted threshs, now "<<acceptThreshold<<" "<<rejectThreshold<<endl;
    cout <<"expected true: ";
    for (float v : expectedTrue)
        cout <<v<<", ";
    cout <<endl;
    cout <<"expected false: ";
    for (float v : expectedFalse)
        cout <<v<<", ";
    cout <<endl;
    
    assert(acceptThreshold<rejectThreshold);
}

