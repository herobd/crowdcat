#include "MasterQueue.h"



MasterQueue::MasterQueue() {
    sem_init(&semResultsQueue,false,1);
    sem_init(&semResults,false,1);
    atID=0;
    
    ///testing
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
    
    addSpottingResults(r0);
    ///end testing
}

SpottingsBatch* MasterQueue::getBatch(unsigned int numberOfInstances, unsigned int maxWidth) {
    SpottingsBatch* batch=NULL;
    sem_wait(&semResultsQueue);
    for (auto ele : resultsQueue)
    {
        sem_t* sem = ele.second.first;
        SpottingResults* res = ele.second.second;
        bool succ = 0==sem_trywait(sem);
        if (succ)
        {
            
            sem_post(&semResultsQueue);//I'm going to break out of the loop, so I'll release control
            
            bool done=false;
            batch = res->getBatch(&done,numberOfInstances,maxWidth);
            if (done)
            {cout <<"done in queue "<<endl;
                //TODO return the results that are above the accept threhsold
                
                sem_wait(&semResultsQueue);
                resultsQueue.erase(res->getId());
                
                sem_post(&semResultsQueue);
                
            }
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
        if (results.find(id)!=results.end())
        {
            sem_t* sem=results[id].first;
            SpottingResults* res = results[id].second;
            succ = 0==sem_trywait(sem);
            sem_post(&semResults);
            if (succ)
            {
                bool done=false;
                ret = res->feedback(&done,ids,userClassifications);
                
                if (done)
                {cout <<"done done "<<endl;
                    sem_wait(&semResults);
                    results.erase(res->getId());
                    
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

void MasterQueue::addSpottingResults(SpottingResults* res)
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
