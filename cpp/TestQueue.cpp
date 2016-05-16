#include "TestQueue.h"



TestQueue::TestQueue() {
    pthread_rwlock_init(&userQueuesLock,NULL);
    pthread_rwlock_init(&userResLock,NULL);
    
    string pageLocation = "/home/brian/intel_index/data/gw_20p_wannot/";
    numTestBatches=7;
    for (int i=0; i<numTestBatches; i++)
    {
    
        ifstream in("./data/testBatch"+to_string(i)+".csv");
        string line;
        vector<Spotting> batch0;
        
        
        
        
        //std::getline(in,line);
        float initSplit=0;//stof(line);//-0.52284769;
        string ngram;
        while(std::getline(in,line))
        {
            vector<string> strV;
            //split(line,',',strV);
            std::stringstream ss(line);
            std::string item;
            while (std::getline(ss, item, ',')) {
                strV.push_back(item);
            }
            
            
            
            ngram = strV[0];
            
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
            testGroundTruth[spotting.id]=truth;
            
        }
        ngrams.push_back(ngram);
        testBatches.push_back(batch0);
        //testGroundTruth.push_back(gt0);
        in.close();
    }
}



vector< SpottingsBatch* > TestQueue::getTestSpottings(int maxWidth)
{
    vector< SpottingsBatch* > ret;
    for (int i=0; i<testBatches.size(); i++)
    {
        SpottingsBatch* batch0 = new SpottingsBatch(ngrams[i], 0);
        for (Spotting& s : testBatches[i])
            batch0->push_back(SpottingImage(s,maxWidth));
        
        ret.push_back(batch0);
    }
    
    
    return ret;
}


SpottingsBatch* TestQueue::getBatch(unsigned int numberOfInstances, unsigned int maxWidth, USERID userId) 
{
    SpottingsBatch* batch=NULL;
    //cout<<"getting rw lock"<<endl;
    pthread_rwlock_wrlock(&userQueuesLock);
    //cout<<"got rw lock"<<endl;
    
    if (userQueues.find(userId)==userQueues.end())
    {
        userQueues[userId]=getTestSpottings(maxWidth);
    }
    
    
    
    batch=userQueues[userId].back();//.back();
    
    //userQueues[userId].back().pop_back();   
    
    //if(userQueues[userId].back().size()==0)
    userQueues[userId].pop_back();
    
    if (userQueues[userId].size()==0)
    {
        //pthread_rwlock_unlock(&userQueuesLock);
        //pthread_rwlock_wrlock(&userQueuesLock);
        userQueues.erase(userId);
    }
    pthread_rwlock_unlock(&userQueuesLock);
    
    
    return batch;
}



bool TestQueue::feedback(unsigned long id, const vector<string>& ids, const vector<int>& userClassifications, USERID userId, int* fp, int* fn)
{
    //map<unsigned long, char> classified;
    
    
    pthread_rwlock_wrlock(&userResLock);
    for (int i=0; i<ids.size(); i++)
    {
        unsigned long spot = stoul(ids[i]);
        if(testGroundTruth[spot] == userClassifications[i])
            classified[userId][spot]='c';//correct
        else if (testGroundTruth[spot])
            classified[userId][spot]='p';//fp
        else
            classified[userId][spot]='n';//fn
    }
    if (++returned[userId]==numTestBatches)
    {
        *fp=0;
        *fn=0;
        for (auto x : classified[userId])
        {
            if (x.second=='p')
                *fp++;
            else if (x.second=='n')
                *fn++;
        }
        classified.erase(userId);
        pthread_rwlock_unlock(&userResLock);
        return true;
    }
    //cout << userId<<" recieved "<<returned[userId]<<endl;
    pthread_rwlock_unlock(&userResLock);
    return false;
}

