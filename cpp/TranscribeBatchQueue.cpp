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
void TranscribeBatchQueue::enqueueAll(vector<TranscribeBatch*> batches, vector<unsigned long>* remove)
{
    cout <<"enqueueAll"<<endl;
    lock();
    for (TranscribeBatch* b :  batches)
    {
        bool found=false;
        for (int i=0; i<queue.size(); i++)
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
    if (remove)
    for (unsigned long id :  *remove)
    {
        for (auto i=queue.begin(); i!=queue.end(); i++)
        {
            if (id == (*i)->getId())
            {
                delete (*i);
                queue.erase(i);
                break;
            }
        }
    }
    unlock();
    cout <<"END enqueueAll"<<endl;
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

multimap<string,const cv::Mat> TranscribeBatchQueue::feedback(unsigned long id, string transcription)
{
    multimap<string,const cv::Mat> newNgramExemplars;
    lock();
    if (returnMap.find(id) != returnMap.end())
    {
        if (transcription[0]=='$' && transcription[transcription.length()-1]=='$')
        {
            if (transcription.compare("$ERROR$")==0)
            {
                returnMap[id]->getBackPointer()->error();//change into manual batch or remove spottings?
                cout<<"ERROR returned for trans "<<id<<endl;
                delete returnMap[id];
            }
            else if (transcription.length()>9 && transcription.substr(0,8).compare("$REMOVE:")==0)
            {
                unsigned long sid; 
                //try
                //{
                    sid= stoul(transcription.substr(8,transcription.length()-9));

                    TranscribeBatch* newBatch = returnMap[id]->getBackPointer()->removeSpotting(sid);
                    if (newBatch!=NULL)
                        queue.push_back(newBatch);
                //}
                //catch (const invalid_argument& ia)
                //{
                //    cout << "Assumed stoul() failed with: "<<transcription.substr(8,transcription.length()-9)<< " ("<<transcription<<")"<<endl;
                //    cout << ia.what() << endl;
                //    assert(false);
                //}
                delete returnMap[id];
            }
            else if (transcription.compare("$PASS$")==0)
            {
                queue.push_front(returnMap[id]);
            }
            else
            {
                cout << "invalid_argument TranscribeBatchQueue::feedback(#,"<<transcription<<")"<<endl;
            }
        }
        else
        {
            newNgramExemplars=returnMap[id]->getBackPointer()->result(transcription);
            doneMap[id] = returnMap[id]->getBackPointer();
            delete returnMap[id];
        }
        returnMap.erase(id);
        timeMap.erase(id);
    }
    else
    {
        //This occurs on a resend
        
        if (transcription.compare("$PASS$")!=0)
            newNgramExemplars=doneMap[id]->result(transcription,&harvestedToRetract);
    }
    unlock();
    return newNgramExemplars;
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
