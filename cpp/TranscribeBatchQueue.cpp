#include "TranscribeBatchQueue.h"

TranscribeBatchQueue::TranscribeBatchQueue()
{

}

/*void TranscribeBatchQueue::enqueue(TranscribeBatch* batch)
{
    lock();
    queue.push_back(batch);
    unlock();
}*/
void TranscribeBatchQueue::enqueueAll(vector<TranscribeBatch*> batches)
{
    lock();
    for (TranscribeBatch* b :  batches)
    {
        bool found=false;
        for (int i=0; i<batches.size(); i++)
        {
            if (b->getId() == queue[i]->getId())
            {
                delete queue[i];
                queue[i] = b;
                found=true;
                break;
            }
        }
        if (!found)
            queue.push_back(b);
    }
    unlock();
}

TranscribeBatch* TranscribeBatchQueue::dequeue(unsigned int maxWidth)
{
    lock();
    TranscribeBatch* ret=NULL;
    if (queue.size()>0)
    {
        ret = queue.front();
        ret->setWidth(maxWidth);
        queue.pop_front();

        returnMap[ret->getId()]=ret;
        timeMap[ret->getId()]=chrono::system_clock::now();
    }
    unlock();
    return ret;
}

void TranscribeBatchQueue::feedback(unsigned long id, string transcription)
{
    lock();
    if (returnMap.find(id) != returnMap.end())
    {
        returnMap[id]->getBackPointer()->result(transcription);
        doneMap[id] = returnMap[id]->getBackPointer();
        delete returnMap[id];
        returnMap.erase(id);
        timeMap.erase(id);
    }
    else
    {
        //This occurs on a resend
        
        doneMap[id]->result(transcription);
    }
    unlock();
}

void TranscribeBatchQueue::checkIncomplete()
{
    lock();
    for (auto start : timeMap)
    {
        unsigned long id = start.first;
        chrono::system_clock::duration d = chrono::system_clock::now()-start.second;
        chrono::minutes pass = chrono::duration_cast<chrono::minutes> (d);
        if (pass.count() > 20) //if 20 mins has passed
        {
            queue.push_front(returnMap[id]);

            returnMap.erase(id);
            timeMap.erase(id);
        }
    }
    unlock();
}
