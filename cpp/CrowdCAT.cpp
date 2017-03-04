#include "CrowdCAT.h"
//

void checkIncompleteSleeper(CrowdCAT* cattss, MasterQueue* q, Knowledge::Corpus* c)
{
    //This is the lowest priority of the systems threads
    nice(3);
    //this_thread::sleep_for(chrono::hours(1));
    //cout <<"kill is "<<q->kill.load()<<endl;
#ifdef NO_NAN
    int count=0;
#endif
    while(!q->kill.load() && cattss->getCont()) {
        //cout <<"begin sleep"<<endl;
        this_thread::sleep_for(chrono::minutes(CHECK_SAVE_TIME));
        if (!q->kill.load() && cattss->getCont())
        {
            cattss->save();
#ifdef NO_NAN
            GlobalK::knowledge()->writeTrack();
            if (++count % 39 == 0)
            {
                q->checkIncomplete();
                c->checkIncomplete();
            }
#else
            q->checkIncomplete();
            c->checkIncomplete();
#endif
        }
    }
}
void showSleeper(CrowdCAT* cattss, MasterQueue* q, Knowledge::Corpus* c, int height, int width, int milli)
{
    //This is the lowest priority of the systems threads
    nice(3);
#ifdef NO_NAN
    int count=0;
#endif
    while(!q->kill.load() && cattss->getCont()) {
        this_thread::sleep_for(chrono::milliseconds(milli));
        if (!q->kill.load() && cattss->getCont())
        {
            c->showProgress(height,width);
#ifdef NO_NAN
            if (count++ % 5==0) //every 20 seconds
            {
                string misTrans;
                float accTrans,pWordsTrans;
                float pWords80_100, pWords60_80, pWords40_60, pWords20_40, pWords0_20, pWords0;
                string misTrans_IV;
                float accTrans_IV,pWordsTrans_IV;
                float pWords80_100_IV, pWords60_80_IV, pWords40_60_IV, pWords20_40_IV, pWords0_20_IV, pWords0_IV;
                c->getStats(&accTrans,&pWordsTrans, &pWords80_100, &pWords60_80, &pWords40_60, &pWords20_40, &pWords0_20, &pWords0, &misTrans,
                            &accTrans_IV,&pWordsTrans_IV, &pWords80_100_IV, &pWords60_80_IV, &pWords40_60_IV, &pWords20_40_IV, &pWords0_20_IV, &pWords0_IV, &misTrans_IV);
                GlobalK::knowledge()->saveTrack(accTrans,pWordsTrans, pWords80_100, pWords60_80, pWords40_60, pWords20_40, pWords0_20, pWords0, misTrans,
                                                accTrans_IV,pWordsTrans_IV, pWords80_100_IV, pWords60_80_IV, pWords40_60_IV, pWords20_40_IV, pWords0_20_IV, pWords0_IV, misTrans_IV);
            }
#endif
        }
    }
}

CrowdCAT::CrowdCAT( string lexiconFile, 
                string pageImageDir, 
                string segmentationFile, 
                string spottingModelPrefix,
                string savePrefix,
                int avgCharWidth,
                int numSpottingThreads,
                int numTaskThreads,
                int showHeight,
                int showWidth,
                int showMilli,
                int contextPad ) : savePrefix(savePrefix)
{
    cont.store(1);
    sem_init(&semLock, 0, 0);

    ifstream in (savePrefix+"_CrowdCAT.sav");
    if (in.good())
    {
        cout<<"Load file found."<<endl;
        Lexicon::instance()->load(in);
        corpus = new Knowledge::Corpus(in);
        corpus->loadSpotter(spottingModelPrefix);
        CorpusRef* corpusRef = corpus->getCorpusRef();
        PageRef* pageRef = corpus->getPageRef();
        masterQueue = new MasterQueue(in,corpusRef,pageRef);
        delete corpusRef;
        delete pageRef;
        spottingQueue = new SpottingQueue(in,masterQueue,corpus);


       
        cout<<"Loaded. Begins from time: "<<line<<endl;
        in.close();
    }
    else
    {
    
        masterQueue = new MasterQueue(contextPad);
        Lexicon::instance()->readIn(lexiconFile);
        corpus = new Knowledge::Corpus(contextPad, avgCharWidth);
        corpus->addWordSegmentaionAndGT(pageImageDir, segmentationFile);
        corpus->loadSpotter(spottingModelPrefix);
        spottingQueue = new SpottingQueue(masterQueue,corpus); //should enqueue the corpus
        //??spottingQueue->transcribeCorpus();

    //#endif
#endif
    }

#ifdef TEST_MODE
    /*
    in.open(segmentationFile+".char");
    string line;
    //getline(in,line);//header
    while (getline(in,line))
    {
        string s;
        std::stringstream ss(line);
        getline(ss,s,',');
        string word=s;
        getline(ss,s,',');
        int pageId = (stoi(s)+1); //+1 is correction from my creation of segmentation file
        getline(ss,s,',');//x1
        int x1=stoi(s);
        getline(ss,s,',');
        int y1=stoi(s);
        getline(ss,s,',');//x2
        int x2=stoi(s);
        getline(ss,s,',');
        int y2=stoi(s);
        vector<int> lettersStart, lettersEnd;
        //vector<int> lettersStartRel, lettersEndRel;

        while (getline(ss,s,','))
        {
            lettersStart.push_back(stoi(s));
            //lettersStartRel.push_back(stoi(s)-x1);
            getline(ss,s,',');
            lettersEnd.push_back(stoi(s));
            //lettersEndRel.push_back(stoi(s)-x1);
            //getline(ss,s,',');//conf
        }
        GlobalK::knowledge()->addWordBound(word,pageId,x1,y1,x2,y2,lettersStart,lettersEnd);
    }
    in.close();
    */
#endif
    
   
    //should be priority 77 
    incompleteChecker = new thread(checkIncompleteSleeper,this,masterQueue,corpus);//This could be done by a thread for each sr
    incompleteChecker->detach();
    showChecker = new thread(showSleeper,this,masterQueue,corpus,showHeight,showWidth,showMilli);
    showChecker->detach();
//#ifndef GRAPH_SPOTTING_RESULTS
    spottingQueue->run(numSpottingThreads);
//#endif
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
BatchWraper* CrowdCAT::getBatch(int width)
{
#if !defined(TEST_MODE) && !defined(NO_NAN)
    try
    {
#else
        //cout<<"getBatch, color:"<<color<<", prev:"<<prevNgram<<endl;
#endif
        BatchWraper* ret= masterQueue->getBatch(width);
        if (ret==NULL)
        {
            //TODO, I don't think this will be needed
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
#if !defined(TEST_MODE) && !defined(NO_NAN)
    }
    catch (exception& e)
    {
        cout <<"Exception in CrowdCAT::getBatch(), "<<e.what()<<endl;
    }
    catch (...)
    {
        cout <<"Exception in CrowdCAT::getBatch(), UNKNOWN"<<endl;
    }
#endif
    return new BatchWraperBlank();
}



void CrowdCAT::updateTranscription(string id, string transcription, bool manual)
{
    enqueue(new UpdateTask(id,transcription,manual));
}


void CrowdCAT::misc(string task)
{
#if !defined(TEST_MODE) && !defined(NO_NAN)
    try
    {
#endif
        if (task.compare("showCorpus")==0)
        {
            corpus->show();
        }
        else if (task.length()>16 && task.substr(0,16).compare("showInteractive:")==0)
        {
            int pageNum = stoi(task.substr(16));
            corpus->showInteractive(pageNum);
        }
/*        else if (task.length()>13 && task.substr(0,13).compare("showProgress:")==0)
        {
            cout <<"CrowdCAT::showProgress()"<<endl;
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
            cout<<"Manual Finish engaged."<<endl;
        }
        else if (task.compare("save")==0)
        {
            save();
        }
#ifdef TEST_MODE
        else if (task.length()>11 && task.substr(0,11).compare("forceNgram:")==0)
        {
            masterQueue->forceNgram = task.substr(11);
        }
        else if (task.compare("unforce")==0 || task.compare("unforceNgram")==0)
        {
            masterQueue->forceNgram="";
        }
#endif
        /*else if (task.length()>14 && task.substr(0,14).compare("startSpotting:")==0)
        {
            int num = stoi(task.substr(14));
            //cout<<"startSpotting:"<<num<<endl;
            if (num>0)
                spottingQueue->run(num);
            else
                cout<<"ERROR: tried to start spotting with "<<num<<" threads"<<endl;
        }*/
#if !defined(TEST_MODE) && !defined(NO_NAN)
    }
    catch (exception& e)
    {
        cout <<"Exception in CrowdCAT::misc(), "<<e.what()<<endl;
    }
    catch (...)
    {
        cout <<"Exception in CrowdCAT::misc(), UNKNOWN"<<endl;
    }
#endif
}    

void* threadTask(void* cattss)
{
    //signal(SIGPIPE, SIG_IGN);
    nice(1);
    ((CrowdCAT*)cattss)->threadLoop();
    //pthread_exit(NULL);
}

void CrowdCAT::run(int numThreads)
{

    taskThreads.resize(numThreads);
    for (int i=0; i<numThreads; i++)
    {
        taskThreads[i] = new thread(threadTask,this);
        taskThreads[i]->detach();
        
    }
}
void CrowdCAT::stop()
{
    cont.store(0);
    for (int i=0; i<taskThreads.size(); i++)
        sem_post(&semLock);
}

//For data collection, when I deleted all my trans... :(
void CrowdCAT::resetAllWords_()
{
    vector<TranscribeBatch*> bs = corpus->resetAllWords_();
    vector<unsigned long> toRemoveBatches;        
    masterQueue->enqueueTranscriptionBatches(bs,&toRemoveBatches);
}

void CrowdCAT::threadLoop()
{
    UpdateTask* updateTask;
    while(1)
    {
        
#if !defined(TEST_MODE) && !defined(NO_NAN)
        try
        {
#endif
            updateTask = dequeue();
            if (!cont.load())
            {
                if (updateTask!=NULL)
                    delete updateTask;
                break; //END
            }
            
            if (updateTask!=NULL)
            {
                if (updateTask->type==TRANSCRIPTION_TASK)
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
                else
                {
                    cout<<"Error, unknown update type"<<endl;
                    assert(false && "Error, unknown update type");
                }
                delete updateTask;
            }
            else
                break; //END
#if !defined(TEST_MODE) && !defined(NO_NAN)
        }
        catch (exception& e)
        {
            cout <<"Exception in CrowdCAT:threadLoop ["<<updateTask->type<<"], "<<e.what()<<endl;
        }
        catch (...)
        {
            cout <<"Exception in CrowdCAT::threadLoop ["<<updateTask->type<<"], UNKNOWN"<<endl;
        }
#endif
    }
}

void CrowdCAT::enqueue(UpdateTask* task)
{
     
    taskQueueLock.lock();
    taskQueue.push_back(task);
    sem_post(&semLock);
    taskQueueLock.unlock();
}
UpdateTask* CrowdCAT::dequeue()
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

void CrowdCAT::save()
{
    if (savePrefix.length()>0)
    {
#ifdef TEST_MODE
        cout<<"START save.    "<<GlobalK::currentDateTime()<<endl;
        clock_t t;
        t = clock();
#endif

        time_t timeSec;
        time(&timeSec);

        string saveName = savePrefix+"_CrowdCAT.sav";
        //In the event of a crash while saveing, keep a backup of the last save
        rename( saveName.c_str() , (saveName+".bck").c_str() );

        ofstream out (saveName);

        Lexicon::instance()->save(out);
        corpus->save(out);
        masterQueue->save(out);
        spottingQueue->save(out);

        out<<timeSec<<"\n";
        out.close();
#ifdef TEST_MODE
        t = clock() - t;
        cout<<"END save: "<<((float)t)/CLOCKS_PER_SEC<<" secs.    "<<endl;
#endif
    }
}