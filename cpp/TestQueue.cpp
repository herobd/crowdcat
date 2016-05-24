#include "TestQueue.h"



TestQueue::TestQueue() {
    pthread_rwlock_init(&userQueuesLock,NULL);
    pthread_rwlock_init(&userResLock,NULL);
    
    int color=0;
    string pageLocation = "/home/brian/intel_index/data/gw_20p_wannot/";
    int numOfTestBatches=7;
    for (int i=0; i<numOfTestBatches; i++)
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
        if (ngrams.size()!=0 && ngrams.back().compare(ngram)!=0)
            color++;
        ngrams.push_back(ngram);
        colors.push_back(color);
        testBatches.push_back(batch0);
        //testGroundTruth.push_back(gt0);
        in.close();
    }
}



deque< SpottingsBatch* > TestQueue::getTestSpottings(unsigned int numberOfInstances, unsigned int maxWidth,int color)
{
    if (numberOfInstances<=0)
        numberOfInstances=5;
    deque< SpottingsBatch* > ret;
    SpottingsBatch* batch0 = NULL;//new SpottingsBatch(ngrams[0], 0);
    for (int i=0; i<testBatches.size(); i++)
    {
        
        if (batch0!=NULL)
        {
            if (ngrams[i].compare(ngrams[i-1])!=0)
            {
                ret.push_back(batch0);
                batch0 = NULL;
            }
        }
        for (Spotting& s : testBatches[i])
        {
            if (batch0==NULL)
                batch0=new SpottingsBatch(ngrams[i], 0);
            //string prevNgram="";
            //if (ret.size()>0)
            //    prevNgram=ret.back()->ngram;
            batch0->push_back(SpottingImage(s,maxWidth,colors[i],""));
            if (batch0->size()==numberOfInstances)
            {
                ret.push_back(batch0);
                batch0 = NULL; //new SpottingsBatch(ngrams[i], 0);
            }
        }
        
        
    }
    if (batch0!=NULL)
        ret.push_back(batch0);
    
    return ret;
}


SpottingsBatch* TestQueue::getBatch(unsigned int numberOfInstances, unsigned int maxWidth, int color, USERID userId) 
{
    SpottingsBatch* batch=NULL;
    //cout<<"getting rw lock"<<endl;
    pthread_rwlock_wrlock(&userQueuesLock);
    //cout<<"got rw lock"<<endl;
    
    if (userQueues.find(userId)==userQueues.end())
    {
        cout <<"new user "<<userId<<", color="<<color<<endl;
        userQueues[userId]=getTestSpottings(numberOfInstances,maxWidth,color);//we are assuming the number of instances will not change
        numTestBatches[userId]=userQueues[userId].size();
    }
    
    if (userQueues[userId].size()==0)
    {
        //pthread_rwlock_unlock(&userQueuesLock);
        //pthread_rwlock_wrlock(&userQueuesLock);
        userQueues.erase(userId);
        return NULL;
    }
    
    batch=userQueues[userId].front();//.back();
    
    //userQueues[userId].back().pop_back();   
    
    //if(userQueues[userId].back().size()==0)
    userQueues[userId].pop_front();
    
    
    pthread_rwlock_unlock(&userQueuesLock);
    
    
    return batch;
}



bool TestQueue::feedback(unsigned long id, const vector<string>& ids, const vector<int>& userClassifications, int resent, USERID userId, int* fp, int* fn)
{
    //map<unsigned long, char> classified;
    if (numTestBatches.find(userId) != numTestBatches.end())
    {
    
        pthread_rwlock_wrlock(&userResLock);
        assert(ids.size()>0);
        for (int i=0; i<ids.size(); i++)
        {
            unsigned long spot = stoul(ids[i]);
            if(testGroundTruth[spot] == userClassifications[i])
                classified[userId][spot]='c';//correct
            else if (testGroundTruth[spot])
                classified[userId][spot]='n';//fn
            else
                classified[userId][spot]='p';//fp
        }
        cout <<"Returned: "<<(++returned[userId])<<" of: "<<numTestBatches[userId]<<endl;
        //if (resent==0 && ++returned[userId]>=numTestBatches[userId])
        if (classified[userId].size() == testGroundTruth.size())
        {
            //assert(
            *fp=0;
            *fn=0;
            assert(classified[userId].size()>0);
            for (auto x : classified[userId])
            {
                if (x.second=='p')
                    (*fp)++;
                else if (x.second=='n')
                    (*fn)++;
            }
            classified.erase(userId);
            returned.erase(userId);
            numTestBatches.erase(userId);
            pthread_rwlock_unlock(&userResLock);
            return true;
        }
        //cout << userId<<" recieved "<<returned[userId]<<endl;
        pthread_rwlock_unlock(&userResLock);
    }
    else
    {
        cout <<"WARNING, non-existest user ("<<userId<<") trying to submit. "<<resent<<endl;
    }
    return false;
}

