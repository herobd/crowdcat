#include "CATTSS.h"


void checkIncompleteSleeper(CATTSS* cattss, MasterQueue* q, Knowledge::Corpus* c)
{
    //This is the lowest priority of the systems threads
    nice(3);
    //this_thread::sleep_for(chrono::hours(1));
    //cout <<"kill is "<<q->kill.load()<<endl;
    while(!q->kill.load()) {
        //cout <<"begin sleep"<<endl;
        this_thread::sleep_for(chrono::minutes(CHECK_SAVE_TIME));
        q->checkIncomplete();
        c->checkIncomplete();
        cattss->save();
    }
}
void showSleeper(MasterQueue* q, Knowledge::Corpus* c, int height, int width, int milli)
{
    //This is the lowest priority of the systems threads
    nice(3);
#ifdef NO_NAN
    int count=0;
#endif
    while(!q->kill.load()) {
        this_thread::sleep_for(chrono::milliseconds(milli));
        c->showProgress(height,width);
#ifdef NO_NAN
        if (count++ % 5==0) //every 20 seconds
        {
            float pWordsTrans;
            float pWords80_100, pWords60_80, pWords40_60, pWords20_40, pWords0_20, pWords0;
            c->getStats(&pWordsTrans, &pWords80_100, &pWords60_80, &pWords40_60, &pWords20_40, &pWords0_20, &pWords0);
            GlobalK::knowledge()->saveTrack(pWordsTrans, pWords80_100, pWords60_80, pWords40_60, pWords20_40, pWords0_20, pWords0);
        }
#endif
    }
}

CATTSS::CATTSS( string lexiconFile, 
                string pageImageDir, 
                string segmentationFile, 
                string spottingModelPrefix,
                string savePrefix,
                int numSpottingThreads,
                int numTaskThreads,
                int showHeight,
                int showWidth,
                int showMilli,
                int contextPad ) : savePrefix(savePrefix)
{
    cont.store(1);
    sem_init(&semLock, 0, 0);

    ifstream in (savePrefix+"_CATTSS.sav");
    if (in.good())
    {
        Lexicon::instance()->load(in);
        corpus = new Knowledge::Corpus(in);
        corpus->loadSpotter(spottingModelPrefix);
        masterQueue = new MasterQueue(in,corpus->getCorpusRef(),corpus->getPageRef());
        spottingQueue = new SpottingQueue(in,masterQueue,corpus);

        string line;
        getline(in,line);
        int tSize = stoi(line);
        for (int i=0; i<tSize; i++)
        {

            taskQueue.push_back(new UpdateTask(in));
            sem_post(&semLock);
        }
        getline(in,line);
        SpottingsBatch::setIdCounter(stoul(line));
        getline(in,line);
        NewExemplarsBatch::setIdCounter(stoul(line));
        getline(in,line);
        TranscribeBatch::setIdCounter(stoul(line));
        getline(in,line);
        cout<<"Loaded. Begins from time: "<<line<<endl;
        in.close();
    }
    else
    {
    
        masterQueue = new MasterQueue(contextPad);
        Lexicon::instance()->readIn(lexiconFile);
        corpus = new Knowledge::Corpus(contextPad);
        corpus->addWordSegmentaionAndGT(pageImageDir, segmentationFile);
        corpus->loadSpotter(spottingModelPrefix);
        spottingQueue = new SpottingQueue(masterQueue,corpus);

#ifdef TEST_MODE_LONG
        int pageId=0;

        //Spotting er1 (433,14,472,36,pageId,corpus->imgForPageId(pageId),"er",0.0);//[1]
        //vector<Spotting > init = {er1};
        //masterQueue->updateSpottingResults(new vector<Spotting>(init));
        Spotting* er1 = new Spotting(811,18,842,40,pageId,corpus->imgForPageId(pageId),"er",0);//[1]
        vector<Spotting* > init = {er1};
        spottingQueue->addQueries(init);
#else
    //#ifdef TEST_MODE
        /*int pageId=2700270;
        
        Spotting* th1 = new Spotting(586,319,687,390,pageId,corpus->imgForPageId(pageId),"th",0);//[1]
        Spotting* he1 = new Spotting(462,588,535,646,pageId,corpus->imgForPageId(pageId),"he",0);//[1]
        Spotting* in1 = new Spotting(504,857,584,902,pageId,corpus->imgForPageId(pageId),"in",0);//[1]
        Spotting* er1 = new Spotting(593,621,652,645,pageId,corpus->imgForPageId(pageId),"er",0);//[1]
        Spotting* an1 = new Spotting(358,968,466,988,pageId,corpus->imgForPageId(pageId),"an",0);//[1]
        Spotting* re1 = new Spotting(1712,787,1766,815,pageId,corpus->imgForPageId(pageId),"re",0);//[1]
        Spotting* on1 = new Spotting(618,956,703,987,pageId,corpus->imgForPageId(pageId),"on",0);//[1]
        Spotting* at1 = new Spotting(774,1027,876,1072,pageId,corpus->imgForPageId(pageId),"at",0);//[1]
        Spotting* en1 = new Spotting(1589,452,1670,480,pageId,corpus->imgForPageId(pageId),"en",0);//[1]
        Spotting* nd1 = new Spotting(1609,600,1732,640,pageId,corpus->imgForPageId(pageId),"nd",0);//[1]
        Spotting* ti1 = new Spotting(1709,342,1776,388,pageId,corpus->imgForPageId(pageId),"ti",0);//[1]
        Spotting* es1 = new Spotting(1678,956,1729,988,pageId,corpus->imgForPageId(pageId),"es",0);//[1]
        Spotting* or1 = new Spotting(582,1561,632,1590,pageId,corpus->imgForPageId(pageId),"or",0);//[1]
        Spotting* te1 = new Spotting(1245,1618,1306,1668,pageId,corpus->imgForPageId(pageId),"te",0);//[1]
        Spotting* of1 = new Spotting(870,507,966,590,pageId,corpus->imgForPageId(pageId),"of",0);//[1]
        Spotting* ed1 = new Spotting(812,770,914,816,pageId,corpus->imgForPageId(pageId),"ed",0);//[1]
        Spotting* is1 = new Spotting(111,111,222,222,pageId,corpus->imgForPageId(pageId),"is",0);//[1]
        Spotting* it1 = new Spotting(111,111,222,222,pageId,corpus->imgForPageId(pageId),"it",0);//[1]
        Spotting* al1 = new Spotting(111,111,222,222,pageId,corpus->imgForPageId(pageId),"al",0);//[1]
        Spotting* ar1 = new Spotting(111,111,222,222,pageId,corpus->imgForPageId(pageId),"ar",0);//[1]
        Spotting* st1 = new Spotting(111,111,222,222,pageId,corpus->imgForPageId(pageId),"st",0);//[1]
        Spotting* to1 = new Spotting(111,111,222,222,pageId,corpus->imgForPageId(pageId),"to",0);//[1]
        Spotting* nt1 = new Spotting(111,111,222,222,pageId,corpus->imgForPageId(pageId),"nt",0);//[1]
        Spotting* ng1 = new Spotting(111,111,222,222,pageId,corpus->imgForPageId(pageId),"ng",0);//[1]
        Spotting* se1 = new Spotting(111,111,222,222,pageId,corpus->imgForPageId(pageId),"se",0);//[1]
        Spotting* ha1 = new Spotting(111,111,222,222,pageId,corpus->imgForPageId(pageId),"ha",0);//[1]
        Spotting* as1 = new Spotting(111,111,222,222,pageId,corpus->imgForPageId(pageId),"as",0);//[1]*/
        /*vector<Spotting* > init_first = {th1,at1,or1,er1};
        spottingQueue->addQueries(init_first);
        vector<Spotting* > init = {th1,he1,in1,er1,an1,re1,on1,at1,en1,nd1,ti1,es1,or1,te1,of1,ed1,is1,it1,al1,ar1,st1,to1,nt1,ng1,se1,ha1,as1};
        spottingQueue->addQueries(init);*/
        vector<string> top100Bigrams={"th","he","in","er","an","re","on","at","en","nd","ti","es","or","te","of","ed","is","it","al","ar","st","to","nt","ng","se","ha","as","ou","io","le","ve","co","me","de","hi","ri","ro","ic","ne","ea","ra","ce","li","ch","ll","be","ma","si","om","ur","ca","el","ta","la","ns","di","fo","ho","pe","ec","pr","no","ct","us","ac","ot","il","tr","ly","nc","et","ut","ss","so","rs","un","lo","wa","ge","ie","wh","ee","wi","em","ad","ol","rt","po","we","na","ul","ni","ts","mo","ow","pa","im","mi","ai","sh"};
        spottingQueue->addQueries(top100Bigrams);

    //#endif
#endif
    }
    
   
    //should be priority 77 
    incompleteChecker = new thread(checkIncompleteSleeper,this,masterQueue,corpus);//This could be done by a thread for each sr
    incompleteChecker->detach();
    showChecker = new thread(showSleeper,masterQueue,corpus,showHeight,showWidth,showMilli);
    showChecker->detach();
    spottingQueue->run(numSpottingThreads);
    run(numTaskThreads);
    //test
    /*
        Spotting s1(1000, 1458, 1154, 1497, 2720272, corpus->imgForPageId(2720272), "ma", 0.01);
        Spotting s2(1196, 1429, 1288, 1491, 2720272, corpus->imgForPageId(2720272), "ch", 0.01);
        Spotting s3(1114, 1465, 1182, 1496, 2720272, corpus->imgForPageId(2720272), "ar", 0.01);
        Spotting s4(345, 956, 415, 986, 2720272, corpus->imgForPageId(2720272), "or", 0.01);
        Spotting s5(472, 957, 530, 987, 2720272, corpus->imgForPageId(2720272), "er", 0.01);
        Spotting s6(535, 943, 634, 986, 2720272, corpus->imgForPageId(2720272), "ed", 0.01);
        Spotting s7(355, 1046, 455, 1071, 2720272, corpus->imgForPageId(2720272), "un", 0.01);
        Spotting s8(492, 1030, 553, 1069, 2720272, corpus->imgForPageId(2720272), "it", 0.01);
        Spotting s9(439, 1024, 507, 1096, 2720272, corpus->imgForPageId(2720272), "fi", 0.01);
        vector<Spotting> toAdd={s1,s2,s3,s4,s5,s6,s7,s8,s9};
        vector<TranscribeBatch*> newBatches = corpus->updateSpottings(&toAdd,NULL,NULL);
        assert(newBatches.size()>0);
        masterQueue->enqueueTranscriptionBatches(newBatches);
        if (toAdd.size()>9)
        {
            cout <<"harvested (init):"<<endl;
            for (int i=9; i<toAdd.size(); i++)
                imshow("har: "+toAdd[i].ngram,toAdd[i].img());
            waitKey();
        }

        vector<unsigned long> toRemoveSpottings={s7.id};
        vector<unsigned long> toRemoveBatches;        
        toAdd.clear();
        Spotting s10(356, 780, 423, 816, 2720272, corpus->imgForPageId(2720272), "on", 0.01);
        Spotting s11(429, 765, 494, 864, 2720272, corpus->imgForPageId(2720272), "ly", 0.01);
        toAdd.push_back(s10); toAdd.push_back(s11);
        newBatches = corpus->updateSpottings(&toAdd,&toRemoveSpottings,&toRemoveBatches);
        if (toAdd.size()>2)
        {
            cout <<"harvested (init):"<<endl;
            for (int i=9; i<toAdd.size(); i++)
                imshow("har: "+toAdd[i].ngram,toAdd[i].img());
            waitKey();
        }
        //vector<TranscribeBatch*> modBatches = corpus->removeSpottings(toRemoveSpottings,toRemoveBatches);
        masterQueue->enqueueTranscriptionBatches(newBatches,&toRemoveBatches);
        cout <<"Enqueued "<<newBatches.size()<<" new trans batches"<<endl;            
        if (toRemoveBatches.size()>0)
            cout <<"Removed "<<toRemoveBatches.size()<<" trans batches"<<endl;            
    /**/
    //test

}
BatchWraper* CATTSS::getBatch(int num, int width, int color, string prevNgram)
{
#ifndef TEST_MODE
    try
    {
#else
        //cout<<"getBatch, color:"<<color<<", prev:"<<prevNgram<<endl;
#endif
        bool hard=true;
        if (num==-1) {
            num=5;
            hard=false;

        }
        BatchWraper* ret= masterQueue->getBatch(num,hard,width,color,prevNgram);
        if (ret==NULL)
        {
            TranscribeBatch* bat = corpus->getManualBatch(width);
            if (bat!=NULL)
                ret = new BatchWraperTranscription(bat);
        }
#ifdef TEST_MODE_C
        return ret;
#endif
        if (ret!=NULL)
            return ret;
        else
            return new BatchWraperBlank();
#ifndef TEST_MODE
    }
    catch (exception& e)
    {
        cout <<"Exception in CATTSS::getBatch(), "<<e.what()<<endl;
    }
    catch (...)
    {
        cout <<"Exception in CATTSS::getBatch(), UNKNOWN"<<endl;
    }
#endif
    return new BatchWraperBlank();
}


void CATTSS::updateSpottings(string resultsId, vector<string> ids, vector<int> labels, int resent)
{
    enqueue(new UpdateTask(resultsId,ids,labels,resent));
}

void CATTSS::updateTranscription(string id, string transcription, bool manual)
{
    enqueue(new UpdateTask(id,transcription,manual));
}

void CATTSS::updateNewExemplars(string batchId,  vector<int> labels, int resent)
{
    enqueue(new UpdateTask(batchId,labels,resent));
}

void CATTSS::misc(string task)
{
#ifndef TEST_MODE
    try
    {
#endif
        if (task.compare("showCorpus")==0)
        {
            corpus->show();
        }
/*        else if (task.length()>13 && task.substr(0,13).compare("showProgress:")==0)
        {
            cout <<"CATTSS::showProgress()"<<endl;
            string dims = task.substr(13);
            int split = dims.find_first_of(',');
            int split2 = dims.find_last_of(',');
            int height = stoi(dims.substr(0,split));
            int width = stoi(dims.substr(1+split,split2));
            int milli = stoi(dims.substr(1+split2));
            showChecker = new thread(showSleeper,masterQueue,corpus,height,width,milli);
            showChecker->detach();
        }*/
        else if (task.compare("stopSpotting")==0)
        {
            spottingQueue->stop();
            stop();
        }
        else if (task.compare("manualFinish")==0)
        {
            masterQueue->setFinish(true);
        }
        /*else if (task.length()>14 && task.substr(0,14).compare("startSpotting:")==0)
        {
            int num = stoi(task.substr(14));
            //cout<<"startSpotting:"<<num<<endl;
            if (num>0)
                spottingQueue->run(num);
            else
                cout<<"ERROR: tried to start spotting with "<<num<<" threads"<<endl;
        }*/
#ifndef TEST_MODE
    }
    catch (exception& e)
    {
        cout <<"Exception in CATTSS::misc(), "<<e.what()<<endl;
    }
    catch (...)
    {
        cout <<"Exception in CATTSS::misc(), UNKNOWN"<<endl;
    }
#endif
}    

void* threadTask(void* cattss)
{
    //signal(SIGPIPE, SIG_IGN);
    nice(1);
    ((CATTSS*)cattss)->threadLoop();
    //pthread_exit(NULL);
}

void CATTSS::run(int numThreads)
{

    taskThreads.resize(numThreads);
    for (int i=0; i<numThreads; i++)
    {
        taskThreads[i] = new thread(threadTask,this);
        taskThreads[i]->detach();
        
    }
}
void CATTSS::stop()
{
    cont.store(0);
    for (int i=0; i<taskThreads.size(); i++)
        sem_post(&semLock);
}

//For data collection, when I deleted all my trans... :(
void CATTSS::resetAllWords_()
{
    vector<TranscribeBatch*> bs = corpus->resetAllWords_();
    vector<unsigned long> toRemoveBatches;        
    masterQueue->enqueueTranscriptionBatches(bs,&toRemoveBatches);
}

void CATTSS::threadLoop()
{
    UpdateTask* updateTask;
    while(1)
    {
        
#ifndef TEST_MODE
        try
        {
#endif
            updateTask = dequeue();
            if (!cont.load())
                break; //END
            
            if (updateTask!=NULL)
            {
                if (updateTask->type==NEW_EXEMPLAR_TASK)
                {
#ifdef TEST_MODE
                    //cout<<"START NewExemplarsTask: ["<<updateTask->id<<"]"<<endl;
                    //clock_t t;
                    //t = clock();
#endif
                    vector<pair<unsigned long,string> > toRemoveExemplars;        
                    vector<SpottingExemplar*> toSpot = masterQueue->newExemplarsFeedback(stoul(updateTask->id),
                                                                                         updateTask->labels,
                                                                                         &toRemoveExemplars);

                    spottingQueue->removeQueries(&toRemoveExemplars);
                    spottingQueue->addQueries(toSpot);
#ifdef TEST_MODE
                    //t = clock() - t;
                    //cout<<"END NewExemplarsTask: ["<<updateTask->id<<"], took: "<<((float)t)/CLOCKS_PER_SEC<<" secs"<<endl;
#endif
                }
                else if (updateTask->type==TRANSCRIPTION_TASK)
                {
#ifdef TEST_MODE
                    //cout<<"START TranscriptionTask: ["<<updateTask->id<<"]"<<endl;
                    //clock_t t;
                    //t = clock();
#endif
                    vector<pair<unsigned long, string> > toRemoveExemplars;
                    if (updateTask->resent_manual_bool)
                    {
                        vector<Spotting*> newExemplars = corpus->transcriptionFeedback(stoul(updateTask->id),updateTask->strings.front(),&toRemoveExemplars);
                        masterQueue->enqueueNewExemplars(newExemplars,&toRemoveExemplars);
                    }
                    else
                    {
                        masterQueue->transcriptionFeedback(stoul(updateTask->id),updateTask->strings.front(),&toRemoveExemplars);
                    }
                    spottingQueue->removeQueries(&toRemoveExemplars);
#ifdef TEST_MODE
                    //t = clock() - t;
                    //cout<<"END TranscriptionTask: ["<<updateTask->id<<"], took: "<<((float)t)/CLOCKS_PER_SEC<<" secs"<<endl;
#endif
                }
                else if (updateTask->type==SPOTTINGS_TASK)
                {
#ifdef TEST_MODE
                    //cout<<"START SpottingsTask: ["<<updateTask->id<<"]"<<endl;
                    //clock_t t;
                    //t = clock();
#endif
                    vector<pair<unsigned long,string> > toRemoveSpottings;        
                    vector<unsigned long> toRemoveBatches;        
                    vector<Spotting>* toAdd = masterQueue->feedback(stoul(updateTask->id),updateTask->strings,updateTask->labels,updateTask->resent_manual_bool,&toRemoveSpottings);
                    if (toAdd!=NULL)
                    {
                        vector<Spotting*> newExemplars;
                        vector<pair<unsigned long,string> > toRemoveExemplars;        
                        vector<TranscribeBatch*> newBatches = corpus->updateSpottings(toAdd,&toRemoveSpottings,&toRemoveBatches,&newExemplars,&toRemoveExemplars);
                        masterQueue->enqueueTranscriptionBatches(newBatches,&toRemoveBatches);
                        masterQueue->enqueueNewExemplars(newExemplars,&toRemoveExemplars);
                        //cout <<"Enqueued "<<newBatches.size()<<" new trans batches"<<endl;            
                        //if (toRemoveBatches.size()>0)
                        //    cout <<"Removed "<<toRemoveBatches.size()<<" trans batches"<<endl;     
                        toRemoveExemplars.insert(toRemoveExemplars.end(),toRemoveSpottings.begin(),toRemoveSpottings.end());
                        spottingQueue->removeQueries(&toRemoveExemplars);
                        spottingQueue->addQueries(*toAdd);
                        delete toAdd;
                    }
#ifdef TEST_MODE
                    //t = clock() - t;
                    //cout<<"END SpottingsTask: ["<<updateTask->id<<"], took: "<<((float)t)/CLOCKS_PER_SEC<<" secs"<<endl;
#endif
                }
                delete updateTask;
            }
            else
                break; //END
#ifndef TEST_MODE
        }
        catch (exception& e)
        {
            cout <<"Exception in CATTSS:threadLoop ["<<updateTask->type<<"], "<<e.what()<<endl;
        }
        catch (...)
        {
            cout <<"Exception in CATTSS::threadLoop ["<<updateTask->type<<"], UNKNOWN"<<endl;
        }
#endif
    }
}

void CATTSS::enqueue(UpdateTask* task)
{
     
    taskQueueLock.lock();
    taskQueue.push_back(task);
    sem_post(&semLock);
    taskQueueLock.unlock();
}
UpdateTask* CATTSS::dequeue()
{
    sem_wait(&semLock);
    UpdateTask* ret=NULL; 
    taskQueueLock.lock();
    if (taskQueue.size()>0)
    {
        ret = taskQueue.front();
        taskQueue.pop_front();
    }
    taskQueueLock.unlock();
    return ret;
}

void CATTSS::save()
{
    if (savePrefix.length()>0)
    {
#ifdef TEST_MODE
        cout<<"START save"<<endl;
        clock_t t;
        t = clock();
#endif

        time_t timeSec;
        time(&timeSec);

        string saveName = savePrefix+"_CATTSS.sav";
        //In the event of a crash while saveing, keep a backup of the last save
        rename( saveName.c_str() , (saveName+".bck").c_str() );

        ofstream out (saveName);

        Lexicon::instance()->save(out);
        corpus->save(out);
        masterQueue->save(out);
        spottingQueue->save(out);

        taskQueueLock.lock();
        out<<taskQueue.size()<<"\n";
        for (UpdateTask* u : taskQueue)
        {
            u->save(out);
        }
        taskQueueLock.unlock();

        out<<SpottingsBatch::getIdCounter()<<"\n";
        out<<NewExemplarsBatch::getIdCounter()<<"\n";
        out<<TranscribeBatch::getIdCounter()<<"\n";
        out<<timeSec<<"\n";
        out.close();
#ifdef TEST_MODE
        t = clock() - t;
        cout<<"END save: "<<((float)t)/CLOCKS_PER_SEC<<" secs"<<endl;
#endif
    }
}
