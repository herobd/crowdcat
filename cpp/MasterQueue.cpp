#include "MasterQueue.h"

void checkIncompleteSleeper(MasterQueue* q)
{
    //this_thread::sleep_for(chrono::hours(1));
    //cout <<"kill is "<<q->kill.load()<<endl;
    while(!q->kill.load()) {
        //cout <<"begin sleep"<<endl;
        this_thread::sleep_for(chrono::hours(1));
        q->checkIncomplete();
    }
}

void MasterQueue::checkIncomplete()
{
    transcribeBatchQueue.checkIncomplete();    
    
    pthread_rwlock_rdlock(&semResults);
    for (auto ele : results)
    {
        
        sem_t* sem = ele.second.first;
        SpottingResults* res = ele.second.second;
        sem_wait(sem);
            
        if (res->checkIncomplete())
        {
            pthread_rwlock_wrlock(&semResultsQueue);
            resultsQueue[res->getId()] = ele.second;
            pthread_rwlock_unlock(&semResultsQueue);
        }
        
        sem_post(sem);
            

    }
    pthread_rwlock_unlock(&semResults);
}

MasterQueue::MasterQueue() {
    //sem_init(&semResultsQueue,false,1);
    //sem_init(&semResults,false,1);
    pthread_rwlock_init(&semResultsQueue,NULL);
    pthread_rwlock_init(&semResults,NULL);
    kill.store(false);
    incompleteChecker = new thread(checkIncompleteSleeper,this);//This could be done by a thread for each sr
    incompleteChecker->detach();
    //atID=0;
    
    ///testing
    
    /*
    //cv::Mat* &page = new cv::Mat();
    page = cv::imread("/home/brian/intel_index/data/gw_20p_wannot/2700270.tif");//,CV_LOAD_IMAGE_GRAYSCALE
    
    SpottingResults* r0 = new SpottingResults("an",-1,-1);//0.1,0.9);
    
    r0->add(Spotting(1416, 186, 1518, 225, 0, &page, "an", 0.5));
    r0->add(Spotting(258, 360, 342, 399, 0, &page, "an", 0.4));
    r0->add(Spotting(1626, 366, 1704, 396, 0, &page, "an", 0.45));
    
    r0->add(Spotting(801, 189, 927, 231, 0, &page, "an", 0.2));
    r0->add(Spotting(693, 618, 837, 657, 0, &page, "an", 0.3));
    r0->add(Spotting(1563, 612, 1677, 651, 0, &page, "an", 0.35));
    r0->add(Spotting(1260, 702, 1356, 735, 0, &page, "an", 0.41));
    
    r0->add(Spotting(1416, 186, 1518, 225, 0, &page, "an", 0.55));
    r0->add(Spotting(258, 360, 342, 399, 0, &page, "an", 0.44));
    r0->add(Spotting(1626, 366, 1704, 396, 0, &page, "an", 0.47));
    
    r0->add(Spotting(801, 189, 927, 231, 0, &page, "an", 0.22));
    r0->add(Spotting(693, 618, 837, 657, 0, &page, "an", 0.33));
    r0->add(Spotting(1563, 612, 1677, 651, 0, &page, "an", 0.42));
    r0->add(Spotting(1260, 702, 1356, 735, 0, &page, "an", 0.46));
    
    r0->add(Spotting(1416, 186, 1518, 225, 0, &page, "an", 0.7));
    r0->add(Spotting(258, 360, 342, 399, 0, &page, "an", 0.64));
    r0->add(Spotting(1626, 366, 1704, 396, 0, &page, "an", 0.445));
    
    r0->add(Spotting(801, 189, 927, 231, 0, &page, "an", 0.1));
    r0->add(Spotting(693, 618, 837, 657, 0, &page, "an", 0.15));
    r0->add(Spotting(1563, 612, 1677, 651, 0, &page, "an", 0.35));
    r0->add(Spotting(1260, 702, 1356, 735, 0, &page, "an", 0.433));
    
    r0->add(Spotting(1416, 186, 1518, 225, 0, &page, "an", 0.665));
    r0->add(Spotting(258, 360, 342, 399, 0, &page, "an", 0.477));
    r0->add(Spotting(1626, 366, 1704, 396, 0, &page, "an", 0.475));
    
    r0->add(Spotting(801, 189, 927, 231, 0, &page, "an", 0.19));
    r0->add(Spotting(693, 618, 837, 657, 0, &page, "an", 0.222));
    r0->add(Spotting(1563, 612, 1677, 651, 0, &page, "an", 0.399));
    r0->add(Spotting(1260, 702, 1356, 735, 0, &page, "an", 0.388));
    
    //boundary cases
    r0->add(Spotting(24, 3180, 72, 3234, 0, &page, "an", 0.401));
    r0->add(Spotting(1935, 951, 1992, 1005, 0, &page, "an", 0.402));
    
    //false
    r0->add(Spotting(1416, 186, 1518, 225, 0, &page, "an", 0.88));
    r0->add(Spotting(258, 360, 342, 399, 0, &page, "an", 0.423));
    r0->add(Spotting(1626, 366, 1704, 396, 0, &page, "an", 0.77));
    
    r0->add(Spotting(1416, 186, 1518, 225, 0, &page, "an", 0.555));
    r0->add(Spotting(258, 360, 342, 399, 0, &page, "an", 0.433));
    r0->add(Spotting(1626, 366, 1704, 396, 0, &page, "an", 0.444));
    
    r0->add(Spotting(1416, 186, 1518, 225, 0, &page, "an", 0.631));
    r0->add(Spotting(258, 360, 342, 399, 0, &page, "an", 0.51));
    r0->add(Spotting(1626, 366, 1704, 396, 0, &page, "an", 0.53));
    
    addSpottingResults(r0);*/
    
    testIter=0;	
    test_rotate=0;
    addTestSpottings();
    accuracyAvg= recallAvg= manualAvg= effortAvg= 0;
    done=0;
    numCFalse=numCTrue=0;

    
    ///end testing
}



void MasterQueue::addTestSpottings()
{
    string pageLocation = "/home/brian/intel_index/data/gw_20p_wannot/";
    
    map<string,SpottingResults*> spottingResults;
    
    
    ifstream in("./data/GW_spottings_fold1_0.100000.csv");
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
        while (std::getline(ss, item, ',')) {
            strV.push_back(item);
        }
        
        
        
        string ngram = strV[0];
        string spottingId = strV[0]+strV[1]+":"+to_string(testIter);
        if (spottingResults.find(spottingId)==spottingResults.end())
        {
            spottingResults[spottingId] = new SpottingResults(ngram);
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
        
        Spotting spotting(tlx, tly, brx, bry, stoi(page), &pages[page], ngram, score);
        spottingResults[spottingId]->add(spotting);
        test_groundTruth[spottingResults[spottingId]->getId()][spotting.id]=truth;
        test_total[spottingResults[spottingId]->getId()]++;
        if (truth)
            test_totalPos[spottingResults[spottingId]->getId()]++;
    }
    
    for (auto p : spottingResults)
    {
        addSpottingResults(p.second);
        //cout << "added "<<p.first<<endl;
    }
    
    
    testIter++;
}

bool MasterQueue::test_autoBatch()
{
    SpottingsBatch* b = getSpottingsBatch(5, false, 300,0,"");
    if (b==NULL)
        return false;
    vector<string> ids;
    vector<int> userClassifications;
    assert(b->size()>0);
    //cout<<b->ngram<<": ";
    
    for (int i=0; i<b->size(); i++)
    {
        
        unsigned long id = b->at(i).id;
        ids.push_back(to_string(id));
        if (test_groundTruth[b->spottingResultsId][id])
            userClassifications.push_back(1);
        else
            userClassifications.push_back(0);
        
        //cout << b->at(i).score <<" "<<test_groundTruth[b->spottingResultsId][id]<< ", ";
    }
    //cout<<endl;
    vector<Spotting>* tmp = test_feedback(b->spottingResultsId, ids, userClassifications);
    delete tmp;
    return true;
}
SpottingsBatch* MasterQueue::getSpottingsBatch(unsigned int numberOfInstances, bool hard, unsigned int maxWidth, int color, string prevNgram) 
{
    SpottingsBatch* batch=NULL;
    //cout<<"getting rw lock"<<endl;
    pthread_rwlock_rdlock(&semResultsQueue);
    //cout<<"got rw lock"<<endl;
#if ROTATE
    int test_loc=0;
#endif
    for (auto ele : resultsQueue)
    {
#if ROTATE
        if (test_loc++<test_rotate/2)
            continue;
#endif
        
        sem_t* sem = ele.second.first;
        SpottingResults* res = ele.second.second;
        bool succ = 0==sem_trywait(sem);
        if (succ)
        {
            
            pthread_rwlock_unlock(&semResultsQueue);//I'm going to break out of the loop, so I'll release control
            
            //test
#if ROTATE
            pthread_rwlock_wrlock(&semResultsQueue);
            if (test_rotate++>2*(resultsQueue.size()-1))
                test_rotate=0;
            pthread_rwlock_unlock(&semResultsQueue);
#endif
            //test
            
            bool done=false;
            cout << "getBatch   prev:"<<prevNgram<<endl;
            batch = res->getBatch(&done,numberOfInstances,hard,maxWidth,color,prevNgram);
            if (done)
            {   cout <<"done in queue "<<endl;
                //TODO return the results that are above the accept threhsold
                
                pthread_rwlock_wrlock(&semResultsQueue);
                resultsQueue.erase(res->getId());
                
                
                
                pthread_rwlock_unlock(&semResultsQueue);
                
                ///test
                //test_finish();
                ///test
            }
            sem_post(sem);
            break;
            
        }
        //else
        //{
        //    cout <<"couldn't get lock"<<endl;
        //}
    }
    if (batch==NULL)
    {
        pthread_rwlock_unlock(&semResultsQueue);//just in case
        //cout<<"null b"<<endl;
    }
    else //test
    {   
        /*cout<<"batch: ";
        for (int i=0; i<batch->size(); i++)
        {
            if (test_groundTruth[batch->spottingResultsId][batch->at(i).id])
                cout << "true\t";
            else
                cout << "false\t";
        }   
        cout<<endl;*/
    }
    return batch;
}

void MasterQueue::test_finish()
{
    if (resultsQueue.size()==0)
    {
        
        test_numDone.clear();
        test_totalPos.clear();
        test_numTruePos.clear();
        test_numFalsePos.clear();
        addTestSpottings();
    }
    
}
void MasterQueue::test_showResults(unsigned long id,string ngram)
{
    cout << "*for "<<id<<" "<<ngram<<endl;
    cout << "* accuracy: "<<(0.0+test_numTruePos[id])/(test_numTruePos[id]+test_numFalsePos[id])<<endl;
    cout << "* recall: "<<(0.0+test_numTruePos[id])/(test_totalPos[id])<<endl;
    cout << "* manual: "<<test_numDone[id]/(0.0+test_total[id])<<endl;
    cout << "* effort: "<<(0.0+test_numTruePos[id])/test_numDone[id]<<endl;
    cout << "* true pos: "<<test_numTruePos[id]<<" false pos: "<<test_numFalsePos[id]<<" total true: "<<test_totalPos[id]<<" total all: "<<test_total[id]<<endl;
    
    accuracyAvg+=(0.0+test_numTruePos[id])/(test_numTruePos[id]+test_numFalsePos[id]);
    recallAvg+=(0.0+test_numTruePos[id])/(test_totalPos[id]);
    manualAvg+=test_numDone[id]/(0.0+test_total[id]);
    effortAvg+=(0.0+test_numTruePos[id])/test_numDone[id];
    done++;
}

//not thread safe
vector<Spotting>* MasterQueue::test_feedback(unsigned long id, const vector<string>& ids, const vector<int>& userClassifications)
{
    assert(ids.size()>0 && userClassifications.size()>0);
    for (int c : userClassifications)
    {
        if (c)
            numCTrue++;
        else
            numCFalse++;
    }
    test_numDone[id]+=ids.size();
    vector<Spotting>* res = feedback(id, ids, userClassifications,0);
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
    
    if (results.find(id)==results.end())
        test_showResults(id,"");
    return res;
}

vector<Spotting>* MasterQueue::feedback(unsigned long id, const vector<string>& ids, const vector<int>& userClassifications, int resent)
{
    cout <<"got feedback for: "<<id<<endl;
    vector<Spotting>* ret = NULL;
    bool succ=false;
    int test=0;
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
                cout <<"res feedback"<<endl;
                ret = res->feedback(&done,ids,userClassifications,resent);
                cout <<"END res feedback"<<endl;
                
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
            else
                cout<<"Failed to get lock for: "<<id<<endl;
        }
        else
        {
            cout <<"Results not found for: "<<id<<endl;
            pthread_rwlock_unlock(&semResults);
            break;
        }
        if (test++>2000)
        {
            cout<<"ERROR, unable to find match"<<endl;
            assert(false);
            break;
        }
    }
    cout<<"END feedback"<<endl;
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
