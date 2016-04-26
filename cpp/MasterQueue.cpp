#include "MasterQueue.h"

MasterQueue::MasterQueue() {
    sem_init(&semResultsQueue,false,1);
    sem_init(&semResults,false,1);
    atID=0;
}

SpottingsBatch MasterQueue::getBatch(unsigned int numberOfInstances, unsigned int maxWidth) {
    SpottingsBatch* batch=NULL;
    sem_wait(&semResultsQueue);
    for (auto ele : resultsQueue)
    {
        sem_t* sem = ele.second.first;
        SpottingsResults* res = ele.second.second;
        bool succ = 0==sem_trywait(sem);
        if (succ)
        {
            
            sem_post(&semResultsQueue);//I'm going to break out of the loop, so I'll release control
            
            bool done=false;
            batch = res->getBatch(&done,numberOfInstances,maxWidth);
            if (done)
            {
                //TODO return the results that are above the accept threhsold
                
                sem_wait(&semResultsQueue);
                resultsQueue.remove(res->getId());
                
                sem_post(&semResultsQueue);
                delete res;
                sem_destroy(sem);
                delete sem;
            }
            else
                sem_post(sem);
            break;
            
        }
    }
    if (batch==NULL)
        sem_post(&semResultsQueue);//just in case
    
    return batch;
}

vector<Spotting>* MasterQueue::feedback(unsigned long id, const vector<string>& ids, const vector<int>& userClassifications)
{
    vector<Spotting>* ret = NULL;
    bool succ=false;
    while (!succ)
    {
        sem_wait(&semResults);
        if (results.hasKey(id)
        {
            sem_t* sem=results[id].first;
            SpottingsResults* res = results[id].second;
            succ = 0==sem_trywait(sem);
            sem_post(&semResults);
            if (succ)
            {
                bool done=false;
                ret = res->feedback(&done,ids,userClassifications);
                
                if (done)
                {
                    sem_wait(&semResults);
                    results.remove(res->getId());
                    
                    sem_post(&semResults);
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
            sem_post(&semResults);
            break;
        }
    }
    return ret;
}

void MasterQueue::addSpottingsResults(SpottingsResults* res)
{
    sem_t* sem = new sem_t();
    sem_init(sem,false,1);
    auto p = make_pair(sem,res);
    sem_wait(&semResultsQueue);
    resultsQueue[res->getId()] = p;
    sem_post(&semResultsQueue);
    
    sem_wait(&semResults);
    results[res->getId()] = p;
    sem_post(&semResults);
}
