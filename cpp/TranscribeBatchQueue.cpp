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

vector<Spotting*> TranscribeBatchQueue::feedback(unsigned long id, string transcription, vector<pair<unsigned long, string> >* toRemoveExemplars)
{
    vector<Spotting*> newNgramExemplars;
    lock();
    WordBackPointer* backPointer=NULL;
    bool resend=false;
    if (returnMap.find(id) != returnMap.end())
    {
        backPointer=returnMap[id]->getBackPointer();
    }
    else if (doneMap.find(id) != doneMap.end())
    {
        //This occurs on a resend
        backPointer=doneMap[id];
        resend=true;
        //if (transcription.length()!=0 && transcription.compare("$PASS$")!=0)
        //    newNgramExemplars=doneMap[id]->result(transcription,toRemoveExemplars);
    }

    if (backPointer!=NULL)
    {
        if ((transcription[0]=='$' && transcription[transcription.length()-1]=='$') || transcription.length()==0)
        {
            if (transcription.compare("$ERROR$")==0)
            {//This probably will occur with bad segmentation
                backPointer->error(id,resend,toRemoveExemplars);//change into manual batch or remove spottings?
                cout<<"ERROR returned for trans "<<id<<endl;
                if (!resend)
                    delete returnMap[id];
            }
            else if (transcription.length()>9 && transcription.substr(0,8).compare("$REMOVE:")==0)
            {
                unsigned long sid; 
                //try
                //{
                    sid= stoul(transcription.substr(8,transcription.length()-9));

                    TranscribeBatch* newBatch = backPointer->removeSpotting(sid,id,resend,&newNgramExemplars,toRemoveExemplars);
                    if (newBatch!=NULL)
                        queue.push_back(newBatch);
                //}
                //catch (const invalid_argument& ia)
                //{
                //    cout << "Assumed stoul() failed with: "<<transcription.substr(8,transcription.length()-9)<< " ("<<transcription<<")"<<endl;
                //    cout << ia.what() << endl;
                //    assert(false);
                //}
                if (!resend)
                {
                    doneMap[id] = backPointer;
                    delete returnMap[id];
                }
            }
            else if (!resend && (transcription.length()==0 || transcription.compare("$PASS$")==0))
            {
                queue.push_front(returnMap[id]);
            }
            else
            {
                cout << "invalid_argument TranscribeBatchQueue::feedback(#,"<<transcription<<")"<<endl;
                if (!resend)
                    queue.push_front(returnMap[id]);
            }
        }
        else
        {
            newNgramExemplars=backPointer->result(transcription,id,resend,toRemoveExemplars);
            if (!resend)
            {
                doneMap[id] = backPointer;
                delete returnMap[id];
            }
        }

        if (!resend)
        {
            returnMap.erase(id);
            timeMap.erase(id);
        }
    }
    else
    {
        cout <<"ERROR: TranscribeBatchQueue::feedback unrecogized id: "<<id<<"   for trans: "<<transcription<<endl;
#ifdef TEST_MODE
//        assert(false);
#endif
    }
    unlock();
    return newNgramExemplars;
}

void TranscribeBatchQueue::checkIncomplete()
{
    lock();
    for (auto iter=timeMap.begin(); iter!=timeMap.end(); iter++)
    {
        unsigned long id = iter->first;
        chrono::system_clock::duration d = chrono::system_clock::now()-iter->second;
        chrono::minutes pass = chrono::duration_cast<chrono::minutes> (d);
        if (pass.count() > 20) //if 20 mins has passed
        {
            queue.push_front(returnMap[id]);

            returnMap.erase(id);
            iter = timeMap.erase(iter);
            if (iter!=timeMap.begin())
                iter--;

            if (iter==timeMap.end())
                break;
        }
    }
    unlock();
}

void TranscribeBatchQueue::save(ofstream& out)
{
    out<<"TRANSCRIBEBATCHQUEUE"<<endl;
    lock();
    //shortcut by saving the returnMap as part of the queue
    out<<(queue.size()+returnMap.size())<<"\n";
    for (auto p : returnMap) //returnmap goes first
    {
        p.second->save(out);
    }
    for (TranscribeBatch* t : queue)
    {
        t->save(out);
    } 
    //we skip the doneMap as it is only used on resends
    unlock();
}
void TranscribeBatchQueue::load(ifstream& in, CorpusRef* corpusRef)
{
    string line;
    getline(in,line);
    assert(line.compare("TRANSCRIBEBATCHQUEUE")==0);
    getline(in,line);
    int qSize = stoi(line);
    for (int i=0; i<qSize; i++)
    {
        queue.push_back(new TranscribeBatch(in,corpusRef));
    }
}
