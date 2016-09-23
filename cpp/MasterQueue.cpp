#include "MasterQueue.h"


void MasterQueue::checkIncomplete()
{
    transcribeBatchQueue.checkIncomplete();    
    newExemplarsBatchQueue.checkIncomplete();    
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
    finish.store(false);
    //sem_init(&semResultsQueue,false,1);
    //sem_init(&semResults,false,1);
    pthread_rwlock_init(&semResultsQueue,NULL);
    pthread_rwlock_init(&semResults,NULL);
    kill.store(false);
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
    //addTestSpottings();
    accuracyAvg= recallAvg= manualAvg= effortAvg= 0;
    done=0;
    numCFalse=numCTrue=0;

    
    ///end testing
}



void MasterQueue::addTestSpottings()
{
    string pageLocation = "/home/brian/intel_index/data/gw_20p_wannot/";
    
    map<string,SpottingResults*> spottingResults;
    
    
    ifstream in("./data/GW_agSpottings_fold1_0.100000.csv");
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
    if (tmp!=NULL)
        delete tmp;
    return true;
}

#ifndef TEST_MODE_C
BatchWraper* MasterQueue::getBatch(unsigned int numberOfInstances, bool hard, unsigned int maxWidth, int color, string prevNgram)
{
    int ngramQueueCount;
    pthread_rwlock_rdlock(&semResultsQueue);
    ngramQueueCount=resultsQueue.size();
    pthread_rwlock_unlock(&semResultsQueue);

    BatchWraper* ret=NULL;
    if (finish.load())
    {
        TranscribeBatch* batch = transcribeBatchQueue.dequeue(maxWidth);
        if (batch!=NULL) 
            ret = new BatchWraperTranscription(batch);
    }
    else
    {

        if (ngramQueueCount < NGRAM_Q_COUNT_THRESH_NEW)
        {
            NewExemplarsBatch* batch=newExemplarsBatchQueue.dequeue(numberOfInstances,maxWidth,color);
            if (batch!=NULL)
                ret = new BatchWraperNewExemplars(batch);
        }
        if (ret==NULL && (ngramQueueCount < NGRAM_Q_COUNT_THRESH_WORD))// || transcribeBatchQueue.size()>TRANS_READY_THRESH)
        {
            TranscribeBatch* batch = transcribeBatchQueue.dequeue(maxWidth);
            if (batch!=NULL) {
                ret = new BatchWraperTranscription(batch);
#ifdef TEST_MODE
//                finish=true;
#endif
            }
        }
        if (ret==NULL)
        {
            SpottingsBatch* batch = getSpottingsBatch(numberOfInstances,hard,maxWidth,color,prevNgram,false);
            if (batch!=NULL)
                ret = new BatchWraperSpottings(batch);
        }
        //a second pass without conditions
        if (ret==NULL)
        {
            TranscribeBatch* batch = transcribeBatchQueue.dequeue(maxWidth);
            if (batch!=NULL)
                ret = new BatchWraperTranscription(batch);
        }
        if (ret==NULL)
        {
            SpottingsBatch* batch = getSpottingsBatch(numberOfInstances,hard,maxWidth,color,prevNgram,true);
            if (batch!=NULL)
                ret = new BatchWraperSpottings(batch);
        }
        if (ret == NULL)
        {
            NewExemplarsBatch* batch=newExemplarsBatchQueue.dequeue(numberOfInstances,maxWidth,color,true);
            if (batch!=NULL)
                ret = new BatchWraperNewExemplars(batch);
        }
    }

    return ret;
} 
#endif

SpottingsBatch* MasterQueue::getSpottingsBatch(unsigned int numberOfInstances, bool hard, unsigned int maxWidth, int color, string prevNgram, bool need) 
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
            //cout << "getBatch   prev:"<<prevNgram<<endl;
            batch = res->getBatch(&done,numberOfInstances,hard,maxWidth,color,prevNgram,need);
            
            if (done)
            {   //cout <<"done in queue "<<endl;
                
                pthread_rwlock_wrlock(&semResultsQueue);
                resultsQueue.erase(res->getId());
                
                pthread_rwlock_unlock(&semResultsQueue);
                
                ///test
                //test_finish();
                ///test
            }
            sem_post(sem);
            if (batch!=NULL)
                break;
            else
                pthread_rwlock_rdlock(&semResultsQueue);
            
        }
        //else
        //{
        //    cout <<"couldn't get lock"<<endl;
        //}
    }
    if (batch==NULL)
    {
        pthread_rwlock_unlock(&semResultsQueue);//just in case
#ifdef TEST_MODE
        cout<<"no spotting batch from MasterQueue, need:"<<need<<endl;
#endif
    }
#ifdef TEST_MODE
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
#endif
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
    vector<pair<unsigned long,string> > toRemoveSpottings;
    vector<Spotting>* res = feedback(id, ids, userClassifications,0,&toRemoveSpottings);
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

vector<Spotting>* MasterQueue::feedback(unsigned long id, const vector<string>& ids, const vector<int>& userClassifications, int resent, vector<pair<unsigned long, string> >* remove)
{
    //cout <<"got feedback for: "<<id<<endl;
    vector<Spotting>* ret=NULL;
    pthread_rwlock_rdlock(&semResults);
    if (results.find(id)!=results.end())
    {
        sem_t* sem=results[id].first;
        SpottingResults* res = results[id].second;
        pthread_rwlock_unlock(&semResults);
        sem_wait(sem);
        int done=0;
        //cout <<"res feedback"<<endl;
        ret = res->feedback(&done,ids,userClassifications,resent,remove);
        //cout <<"END res feedback"<<endl;
        
        if (done==-1)
        {
            pthread_rwlock_wrlock(&semResultsQueue);
            resultsQueue[res->getId()] = make_pair(sem,res);
            pthread_rwlock_unlock(&semResultsQueue);
            
        }
        else if (done==2)
        {
            pthread_rwlock_wrlock(&semResultsQueue);
            resultsQueue.erase(res->getId());
            
            pthread_rwlock_unlock(&semResultsQueue);
        }
        sem_post(sem);
    }
    else
    {
        cout <<"Results not found for: "<<id<<endl;
        pthread_rwlock_unlock(&semResults);
    }
    return ret;
}

void MasterQueue::addSpottingResults(SpottingResults* res, bool hasSemResults, bool toQueue)
{
    sem_t* sem = new sem_t();
    sem_init(sem,false,1);
    auto p = make_pair(sem,res);
    if (toQueue)
    {
        pthread_rwlock_wrlock(&semResultsQueue);
        resultsQueue[res->getId()] = p;
        pthread_rwlock_unlock(&semResultsQueue);
    }
    //This may be a race condition, but would require someone to get and finish a batch between here...
    if (!hasSemResults)
        pthread_rwlock_wrlock(&semResults);
    results[res->getId()] = p;
    if (!hasSemResults)
        pthread_rwlock_unlock(&semResults);
}


void MasterQueue::updateSpottingsMix(const vector< SpottingExemplar*>* spottings)
{

#ifdef TEST_MODE_LONG
    cout<<"updateSpottingsMix: ";
    for (auto s : *spottings)
        cout <<s->ngram<<", ";
    cout<<endl;
#endif
    pthread_rwlock_rdlock(&semResults);
    for (SpottingExemplar* spotting : *spottings)
    {
        bool found=false;
        for (auto p : results)
        {
            
            sem_t* sem=p.second.first;
            SpottingResults* res = p.second.second;
            sem_wait(sem);
            if (res->ngram.compare(spotting->ngram) == 0)
            {
                found=true;
#ifdef TEST_MODE_LONG
                cout<<"update for new ex: "<<spotting->ngram<<endl;
#endif
                //pthread_rwlock_unlock(&semResults);
                //vector<Spotting>* toUpdate = new vector<Spotting>(1);
                //toUpdate->at(0)=*(Spotting*)spotting;
                res->updateSpottingTrueNoScore(*spotting);
                /*if (resurrect)
                {
#ifdef TEST_MODE
                    cout<<"Resurrect "<<res->ngram<<endl;
#endif
                    pthread_rwlock_wrlock(&semResultsQueue);
                    resultsQueue[res->getId()] = make_pair(sem,res);
                    pthread_rwlock_unlock(&semResultsQueue);
                }*/
            }

            
            sem_post(sem);
            
        }
        if (!found)
        {
            //if no id, no matching ngram, this will frequently be the case
#ifdef TEST_MODE
            cout <<"Creating SpottingResults for "<<spotting->ngram<<endl;
#endif
            SpottingResults *n = new SpottingResults(spotting->ngram);
            n->addTrueNoScore(*spotting);
            addSpottingResults(n,true,false);
        }

    }
    pthread_rwlock_unlock(&semResults);
}

unsigned long MasterQueue::updateSpottingResults(vector<Spotting>* spottings, unsigned long id)
{
#ifdef TEST_MODE
    cout<<"updateSpottingResults called from MasterQueue. "<<spottings->front().ngram<<endl;
#endif
    pthread_rwlock_rdlock(&semResults);
    if (id>0)
    {
        if (results.find(id)!=results.end())
        {
            sem_t* sem=results[id].first;
            SpottingResults* res = results[id].second;
            pthread_rwlock_unlock(&semResults);
            sem_wait(sem);
            bool resurrect = res->updateSpottings(spottings);
            if (resurrect)
            {
                pthread_rwlock_wrlock(&semResultsQueue);
                resultsQueue[res->getId()] = results[id];
                pthread_rwlock_unlock(&semResultsQueue);
            }
            sem_post(sem);
            return id;
        }
        else
        {
            cout <<"Resultss not found for: "<<id<<endl;
            //pthread_rwlock_unlock(&semResults);
        }
    }
    
    for (auto p : results)
    {
        
        sem_t* sem=p.second.first;
        SpottingResults* res = p.second.second;
        sem_wait(sem);
        if (res->ngram.compare(spottings->front().ngram) == 0)
        {
            pthread_rwlock_unlock(&semResults);
            bool resurrect = res->updateSpottings(spottings);
            if (resurrect)
            {
#ifdef TEST_MODE
                cout<<"Resurrect "<<res->ngram<<endl;
#endif
                pthread_rwlock_wrlock(&semResultsQueue);
                resultsQueue[res->getId()] = make_pair(sem,res);
                pthread_rwlock_unlock(&semResultsQueue);
            }
            sem_post(sem);
            return res->getId();
            
        }
        else
        {
            sem_post(sem);
        }
    }
        
    //if no id, no matching ngram, or if somethign goes wrong
#ifdef TEST_MODE
    cout <<"Creating SpottingResults for "<<spottings->front().ngram<<endl;
#endif
    SpottingResults *n = new SpottingResults(spottings->front().ngram);
    assert(spottings->size()>1);
    for (Spotting& s : *spottings)
    {
        n->add(s);
        //cout <<"added spotting : "<<s.id<<endl;
    }
    delete spottings;
    unsigned long ret = n->getId();
    addSpottingResults(n,true);
    pthread_rwlock_unlock(&semResults);
    return ret;
}

void MasterQueue::transcriptionFeedback(unsigned long id, string transcription, vector<pair<unsigned long, string> >* toRemoveExemplars) 
{
    vector<Spotting*> newExemplars = transcribeBatchQueue.feedback(id, transcription, toRemoveExemplars);

    //enqueue these for approval
    //if (newExemplars.size()>0)
        newExemplarsBatchQueue.enqueue(newExemplars,toRemoveExemplars);
    //delete newExemplars;
}
void MasterQueue::enqueueNewExemplars(const vector<Spotting*>& newExemplars, vector<pair<unsigned long, string> >* toRemoveExemplars)
{
    newExemplarsBatchQueue.enqueue(newExemplars,toRemoveExemplars);
}

void MasterQueue::needExemplar(string ngram)
{
    newExemplarsBatchQueue.needExemplar(ngram);
}
