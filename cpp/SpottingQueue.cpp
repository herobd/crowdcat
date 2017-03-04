#include "SpottingQueue.h"
//#include <omp.h>
#include <time.h>
#include <unistd.h>

class SpottingQueue;
SpottingQueue::SpottingQueue(MasterQueue* masterQueue, Knowledge::Corpus* corpus)
{
    this->masterQueue=masterQueue;
    this->corpus=corpus;
    //TODO init spotting implementations from modelDir
    
    //int _setId=0;
    cont.store(1);
    sem_init(&semLock, 0, 0);
    //TODO enqueue the corpus
}
SpottingQueue::~SpottingQueue()
{
    sem_destroy(&semLock);
    for (thread* th : spottingThreads)
        delete th;
    for (auto s : onDeck)
        delete s;
    for (auto qu : ngramQueues) //pair<string,priority_queue<SpottingQuery*,vector<SpottingQuery*>,sqcomparison> >
    {
        while (qu.second.size()>0)
        {
            delete qu.second.top();
            qu.second.pop();
        }
        //for (auto s : qu.second)
        //    delete s;
    }
}

void SpottingQueue::stop()
{
    cont.store(0);
    for (int i=0; i<spottingThreads.size(); i++)
        sem_post(&semLock);
}

void* spottingThreadTask(void* queue)
{
    //signal(SIGPIPE, SIG_IGN);
    nice(2);
    ((SpottingQueue*)queue)->spottingLoop();
    //pthread_exit(NULL);
}

void SpottingQueue::run(int numThreads)
{

    /*pthread_attr_t tattr;
    pthread_t tid;
    int ret;
    sched_param param;
    int chk = pthread_attr_init (&tattr);
    assert(chk==0);
    chk = pthread_attr_getschedparam (&tattr, &param);
    assert(chk==0);
    chk = pthread_attr_setschedpolicy(&tattr, SCHED_RR);
    assert(chk==0);
    param.sched_priority = 78;
    chk = pthread_attr_setschedparam (&tattr, &param);
    assert(chk==0);
    chk=pthread_attr_setdetachstate(&tattr,PTHREAD_CREATE_DETACHED);
    assert(chk==0);
    //http://stackoverflow.com/questions/26243495/unable-to-set-pthread-priority-on-creation-with-sched-rr
    chk=pthread_attr_setinheritsched(&tattr, PTHREAD_EXPLICIT_SCHED);
    assert(chk==0);*/

    spottingThreads.resize(numThreads);
    for (int i=0; i<numThreads; i++)
    {
        /*Should be priority 78
        int err = pthread_create(&(spottingThreads[i]), &tattr, spottingThreadTask, (void*)this);
        if (err==EAGAIN)
            cout<<"EAGAIN"<<endl;
        if (err==EINVAL)
            cout<<"EINVAL"<<endl;
        if (err==EPERM)
            cout<<"EPERM"<<endl;
        assert(err==0);*/
        spottingThreads[i] = new thread(spottingThreadTask,this);
        spottingThreads[i]->detach();
        
    }
    //this->numThreads=numThreads;
    //omp_set_num_threads(numThreads);
    //cout<<"startSpotting with "<< numThreads <<" threads."<<endl;
    //#pragma omp parallel //num_threads(numThreads)
    //{
        //if (omp_get_thread_num()==0)
        //    cout<<"Started Spotting with "<< omp_get_num_threads() <<" threads."<<endl;
    
    //}
}

void SpottingQueue::spottingLoop()
{
    thread::id threadId = this_thread::get_id();
    while(1)
    {
        sem_wait(&semLock);
        if (!cont.load())
            break;
        mutLock.lock();
        SpottingQuery* query = dequeue();
        mutLock.unlock();
        if (query==NULL)
            break;
        

        progLock[threadId].lock();
        inProgress[threadId]=query;
        progLock[threadId].unlock();

#ifdef TEST_MODE
        cout<<"START spotting: "<<query->getNgram()<<" ["<<query->getId()<<"]"<<endl;
        clock_t t;
        t = clock();
#endif
        vector<Spotting>* results = corpus->runQuery(query);
#ifdef TEST_MODE
        t = clock() - t;
        cout<<"END spotting: "<<query->getNgram()<<" ["<<query->getId()<<"], took: "<<((float)t)/CLOCKS_PER_SEC<<" secs"<<endl;
#endif
        progLock[threadId].lock();
        inProgress[threadId]=NULL;
        if (results==NULL)
        {
            progLock[threadId].unlock();
            delete query;
            continue;
        }
        bool cont=true;
        emLock.lock();
        auto iter = emList.find(query->getId());
        if (iter !=emList.end()) //has the exemplar I used been revoked?
        {
            emList.erase(iter);
            cont=false;
        }
        emLock.unlock();
        if (cont)
        {
            masterQueue->updateSpottingResults(results);
        }
        progLock[threadId].unlock();

#ifdef NO_NAN
        if (query->getType() == SPOTTING_TYPE_EXEMPLAR)
            GlobalK::knowledge()->newExemplar();
#endif

#ifdef TEST_MODE
        //else
            //cout<<"Successful mid-run cancel."<<endl;
#endif
        delete query;
    }
}
    
void SpottingQueue::addQueries(vector<SpottingExemplar*>& exemplars)
{
    //int setId = ++_setId;
    mutLock.lock();
    for (SpottingExemplar* exemplar : exemplars)
    {
        if (exemplar->type != SPOTTING_TYPE_THRESHED) //It's probably best not to entirely trust the threshed ones
        {
            SpottingQuery* query = new SpottingQuery(exemplar);
            enqueue(query);
            sem_post(&semLock);
        }
    }
    mutLock.unlock();
}
void SpottingQueue::addQueries(vector<Spotting*>& exemplars)
{
    //int setId = ++_setId;
    mutLock.lock();
    for (Spotting* exemplar : exemplars)
    {
        if (exemplar->type != SPOTTING_TYPE_THRESHED) //It's probably best not to entirely trust the threshed ones
        {
            SpottingQuery* query = new SpottingQuery(exemplar);
            enqueue(query);
            sem_post(&semLock);
        }
    }
    mutLock.unlock();
}
void SpottingQueue::addQueries(vector<Spotting>& exemplars)
{
    //int setId = ++_setId;
    mutLock.lock();
    for (Spotting& exemplar : exemplars)
    {
        if (exemplar.type != SPOTTING_TYPE_THRESHED) //It's probably best not to entirely trust the threshed ones
        {
            SpottingQuery* query = new SpottingQuery(exemplar);
            enqueue(query);
            sem_post(&semLock);
        }
    }
    mutLock.unlock();
}
void SpottingQueue::addQueries(vector<string>& ngrams)
{
    mutLock.lock();
    for (string ngram : ngrams)
    {
        SpottingQuery* query = new SpottingQuery(ngram);
        enqueue(query);
        sem_post(&semLock);
    }
    mutLock.unlock();
}

void SpottingQueue::removeQueries(vector<pair<unsigned long,string> >* toRemove)
{
    if (toRemove->size()==0)
        return;
#ifdef TEST_MODE
    cout <<"spotter removing query "<<toRemove->front().first<<":"<<toRemove->front().second<<"..."<<endl;
#endif
    mutLock.lock();
    for (auto r : *toRemove)
    {
        if(sem_trywait(&semLock)==0)//try to remove resource
        {
            bool found=false;
            /*for (auto iter = ngramQueues[r.second].begin(); iter!=ngramQueues[r.second].end(); iter++)
            {
                if ((*iter)->getId() == r.first)
                {
#ifdef TEST_MODE
                    cout<<"found in ngram queue"<<endl;
#endif
                    sem_wait(&semLock);
                    ngramQueues[r.second].erase(iter);
                    found=true;
                    break;
                }
            }*/
            //A big pain
            priority_queue<SpottingQuery*,vector<SpottingQuery*>,sqcomparison> temp;
            while (ngramQueues[r.second].size()>0)
            {
                SpottingQuery* s = ngramQueues[r.second].top();
                ngramQueues[r.second].pop();
                if (!found && s->getId() == r.first)
                {
                    //sem_wait(&semLock);
                    found=true;
                    //break;
                }
                else
                    temp.push(s);
            }
            ngramQueues[r.second].swap(temp);
            if (!found)
            {
                for (auto iter=onDeck.begin(); iter!=onDeck.end(); iter++)
                {
#ifdef TEST_MODE
                    cout<<"scanning onDeck: "<<(*iter)->getId()<<":"<<(*iter)->getNgram()<<endl;
#endif
                    if ((*iter)->getId() == r.first)
                    {
#ifdef TEST_MODE
                        cout<<"found in onDeck"<<endl;
#endif
                        //sem_wait(&semLock);
                        onDeck.erase(iter);
                        found=true;
                        if (ngramQueues[r.second].size()>0)
                        {
                            //onDeck.push_back(ngramQueues[r.second].front());
                            //ngramQueues[r.second].pop_front();
                            onDeck.push_back(ngramQueues[r.second].top());
                            ngramQueues[r.second].pop();
                        }
                        break;
                    }
                }
                if (!found)
                {
                    //add back resourve since we didn't actually remove one
                    sem_wait(&semLock);
#ifdef TEST_MODE
                    cout<<"not found, adding to emList"<<endl;
#endif
                    //Oh, dear! We've spotted or are spotting it.
                    emLock.lock();
                    emList.insert(r.first);
                    emLock.unlock();
                }
            }
        }
        else
        {
            //Oh, dear! We've spotted or are spotting it.
            emLock.lock();
            emList.insert(r.first);
            emLock.unlock();
        }
    }
    mutLock.unlock();
}


void SpottingQueue::enqueue(SpottingQuery* q)
{
    bool isOther=false;
    for (SpottingQuery* oq : onDeck)
    {
        if (oq->getNgram().compare(q->getNgram())==0)
        {
            isOther=true;
            break;
        }
    }
    if (isOther)
    {
        //ngramQueues[q->getNgram()].push_back(q);
        ngramQueues[q->getNgram()].push(q);
    }
    else
    {
        onDeck.push_back(q);
    }
}

SpottingQuery* SpottingQueue::dequeue()
{
    SpottingQuery* ret = NULL;
    if (onDeck.size()>0) //A null return should only occur when stopping.
    {
        ret = onDeck.front();
        onDeck.pop_front();

        if (ngramQueues[ret->getNgram()].size()>0)
        {
            //onDeck.push_back(ngramQueues[ret->getNgram()].front());
            //ngramQueues[ret->getNgram()].pop_front();
            onDeck.push_back(ngramQueues[ret->getNgram()].top());
            ngramQueues[ret->getNgram()].pop();

            //this is to inform the new exempalr queue to try and get these out.
            if (ngramQueues[ret->getNgram()].top()->getType()!=SPOTTING_TYPE_EXEMPLAR)
                masterQueue->needExemplar(ret->getNgram());
        }
        else
            masterQueue->needExemplar(ret->getNgram());
    }
    return ret;
}

void SpottingQueue::save(ofstream& out)
{
    //ofstream out (savePrefix);
    out<<"SPOTTINGQUEUE"<<endl;
    mutLock.lock();
   
    vector<SpottingQuery*> saveProg;
    for (auto p : inProgress)
    {
        progLock[p.first].lock();
        if (inProgress[p.first]!=NULL && emList.find(inProgress[p.first]->getId())==emList.end())
            saveProg.push_back(inProgress[p.first]);
        progLock[p.first].unlock();
    }
    //out<<saveProg.size()<<"\n";

    out<<onDeck.size()+saveProg.size()<<"\n";
    for (SpottingQuery* s : saveProg)
    {
        s->save(out);
    }
    for (SpottingQuery* s : onDeck)
    {
        s->save(out);
    }

    int ngramQueuesSize=0;
    for (auto p : ngramQueues)
    {
        if (p.second.size()>0)
        {
            ngramQueuesSize++;
        }
    }
    out<<ngramQueuesSize<<"\n";
    for (auto p : ngramQueues)
    {
        if (p.second.size()>0)
        {
            out<<p.first<<"\n";
            out<<p.second.size()<<"\n";
            priority_queue<SpottingQuery*, std::vector<SpottingQuery*>, sqcomparison> copy( p.second);
            while(copy.size()>0)
            {
                SpottingQuery* s = copy.top();
                copy.pop();
                s->save(out);
            }
        }
    }
    mutLock.unlock();
    //out.close();
}
SpottingQueue::SpottingQueue(ifstream& in, MasterQueue* masterQueue, Knowledge::Corpus* corpus) : masterQueue(masterQueue), corpus(corpus)
{
    cont.store(1);
    sem_init(&semLock, 0, 0);

    //ifstream in (loadPrefix);
    
    string line;
    getline(in,line);
    assert(line.compare("SPOTTINGQUEUE")==0);
    getline(in,line);
    int size = stoi(line);
    for (int i=0; i<size; i++)
    {
        bool add=true;
        SpottingQuery* loading= new SpottingQuery(in);
        for (SpottingQuery* s : onDeck)
            if (s->getNgram().compare(loading->getNgram()) == 0)
            {
                add=false;
                break;
            }
        if (add)
            onDeck.push_back(loading);
        else
            ngramQueues[loading->getNgram()].push(loading);
        sem_post(&semLock);
    }

    getline(in,line);
    size = stoi(line);
    for (int i=0; i<size; i++)
    {
        string ngram;
        getline(in,ngram);
        getline(in,line);
        int nSize = stoi(line);
        for (int j=0; j<nSize; j++)
        {
            ngramQueues[ngram].push(new SpottingQuery(in));
            sem_post(&semLock);
        }
    }


    //in.close();
}
    
