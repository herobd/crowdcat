#include "TranscribeBatchQueue.h"

TranscribeBatchQueue::TranscribeBatchQueue()
{

}

void TranscribeBatchQueue::enqueue(TranscribeBatch* batch)
{
    lock();
    queue.push_back(batch);
    unlock();
}

TranscribeBatch* TranscribeBatchQueue::dequeue()
{
    lock();
    TranscibeBatch* ret = queue.front();
    queue.pop_front();

    returnMap[ret.getId()]=ret;
    timeMap[ret.getId()]=chrono::system_clock::now();
    unlock();
}

void TranscribeBatchQueue::feedback(unsigned long id, string transcription)
{
    lock();
    if (returnMap.find(id) != returnMap.end())
    {
        returnMap[id]->getBackPointer()->result(transcription);
        doneMap[id] = returnMap[id]->getBackPointer();
        delete returnMap[id]
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
