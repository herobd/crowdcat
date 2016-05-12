#include "TestQueue.h"



TestQueue::TestQueue() {
    pthread_rwlock_init(&userQueuesLock,NULL);
    
    string pageLocation = "/home/brian/intel_index/data/gw_20p_wannot/";
    
    
    
    ifstream in("./data/testBatch0.csv");
    vector<Spotting> batch0;
    vector<bool> batch0;
    string line;
    
    
    
    //std::getline(in,line);
    float initSplit=0;//stof(line);//-0.52284769;
    
    while(std::getline(in,line))
    {
        vector<string> strV;
        //split(line,',',strV);
        std::stringstream ss(line);
        std::string item;
        while (std::getline(ss, item, ',')) {
            strV.push_back(item);
        }
        
        
        
        string ngram = strV[0];
        string spottingId = strV[0]+strV[1]+":"+to_string(testIter);
        if (spottingResults.find(spottingId)==spottingResults.end())
        {
            spottingResults[spottingId] = new SpottingResults(ngram,initSplit);
        }
        
        string page = strV[2];
        size_t startpos = page.find_first_not_of(" \t");
        if( string::npos != startpos )
        {
            page = page.substr( startpos );
        }
        if (pages.find(page)==pages.end())
        {
            pages[page] = cv::imread(pageLocation+page);
            assert(pages[page].cols!=0);
        }
        
        int tlx=stoi(strV[3]);
        int tly=stoi(strV[4]);
        int brx=stoi(strV[5]);
        int bry=stoi(strV[6]);
        
        float score=-1*stof(strV[7]);
        bool truth = strV[8].find("true")!= string::npos?true:false;
        
        Spotting spotting(tlx, tly, brx, bry, 0, &pages[page], ngram, score);
        
        batch0.push_back(spotting);
        gt0.push_back(truth);
        
    }
    
    testBatches.push_back(batch0);
    testGroundTruth.push_back(gt0);
    
}



 TestQueue::getTestSpottings(int maxWidth)
{
    vector< SpottingsBatch* > ret;
    for (auto batch : testBatches)
    {
        SpottingsBatch batch0 = new SpottingsBatch(string ngram, 0);
        for (Spotting& s : batch)
            batch0->push_back(SpottingImage(s,maxWidth));
    }
    
    
    return ret;
}


SpottingsBatch* TestQueue::getBatch(unsigned int numberOfInstances, unsigned int maxWidth, int userId) 
{
    SpottingsBatch* batch=NULL;
    //cout<<"getting rw lock"<<endl;
    pthread_rwlock_rdlock(&userQueuesLock);
    //cout<<"got rw lock"<<endl;
    
    if (userQueues.find(userId)==userQueues.end())
    {
        userQueues[userId]=getTestSpottings();
    }
    
    
    
    batch=userQueues[userId].back();//.back();
    
    //userQueues[userId].back().pop_back();   
    
    //if(userQueues[userId].back().size()==0)
    userQueues[userId].pop_back();
    
    if (userQueues[userId].size()==0)
    {
        pthread_rwlock_unlock(&userQueuesLock);
        pthread_rwlock_wrlock(&userQueuesLock);
        userQueues.erase(userId);
    }
    pthread_rwlock_unlock(&userQueuesLock);
    
    
    return batch;
}


//not thread safe
vector<Spotting>* MasterQueue::test_feedback(unsigned long id, const vector<string>& ids, const vector<int>& userClassifications)
{
    
    test_numDone[id]+=ids.size();
    vector<Spotting>* res = feedback(id, ids, userClassifications);
    for (Spotting s : *res)
    {
        
        if (test_groundTruth[id][s.id])
        {
            test_numTruePos[id]++;
        }
        else
        {
            test_numFalsePos[id]++;
        }
    }
    
    if (res->size()>ids.size())
        test_showResults(id,"");
    return res;
}

vector<Spotting>* MasterQueue::feedback(unsigned long id, const vector<string>& ids, const vector<int>& userClassifications)
{
    vector<Spotting>* ret = NULL;
    bool succ=false;
    while (!succ)
    {
        pthread_rwlock_rdlock(&semResults);
        if (results.find(id)!=results.end())
        {
            sem_t* sem=results[id].first;
            SpottingResults* res = results[id].second;
            succ = 0==sem_trywait(sem);
            pthread_rwlock_unlock(&semResults);
            if (succ)
            {
                bool done=false;
                ret = res->feedback(&done,ids,userClassifications);
                
                if (done)
                {cout <<"done done "<<endl;
                    
                    pthread_rwlock_wrlock(&semResults);
                    results.erase(res->getId());
                    
                    pthread_rwlock_unlock(&semResults);
                    delete res;
                    sem_destroy(sem);
                    delete sem;
                }
                else
                    sem_post(sem);
            }
        }
        else
        {
            pthread_rwlock_unlock(&semResults);
            break;
        }
    }
    return ret;
}

void MasterQueue::addSpottingResults(SpottingResults* res)
{
    sem_t* sem = new sem_t();
    sem_init(sem,false,1);
    auto p = make_pair(sem,res);
    pthread_rwlock_wrlock(&semResultsQueue);
    resultsQueue[res->getId()] = p;
    pthread_rwlock_unlock(&semResultsQueue);
    //This may be a race condition, but would require someone to get and finish a batch between here...
    pthread_rwlock_wrlock(&semResults);
    results[res->getId()] = p;
    pthread_rwlock_unlock(&semResults);
}
