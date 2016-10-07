
#ifndef NE_BATCH_Q
#define NE_BATCH_Q

#include <queue>
#include <map>
#include <chrono>

#include <mutex>
#include <iostream>

#include "Global.h"
#include "spotting.h"
#include "batches.h"
#include "PageRef.h"

using namespace std;


class pcomparison
{
    public:
    pcomparison()
        {}
    bool operator() (Spotting* lhs, Spotting* rhs) const
    {
        if (lhs->ngramRank==-1)
            lhs->ngramRank = GlobalK::knowledge()->getNgramRank(lhs->ngram);
        if (rhs->ngramRank==-1)
            rhs->ngramRank = GlobalK::knowledge()->getNgramRank(rhs->ngram);
        return (lhs->ngramRank>rhs->ngramRank);
    }
};

/*template<typename T>
class removeable_priority_queue : public std::priority_queue<T, std::vector<T>>
{
  public:

      bool remove(const T& value) {
        auto it = std::find(this->c.begin(), this->c.end(), value);
        if (it != this->c.end()) {
            this->c.erase(it);
            std::make_heap(this->c.begin(), this->c.end(), this->comp);
            return true;
       }
       else {
        return false;
       }
 }
}*/

class NewExemplarsBatchQueue
{
    public:
        NewExemplarsBatchQueue();
        void save(ofstream& out);
        void load(ifstream& in, PageRef* pageRef);
        void enqueue(const vector<Spotting*>& batch, vector<pair<unsigned long, string> >* toRemoveExemplars);

        NewExemplarsBatch* dequeue(int batchSize, unsigned int maxWidth, int color, bool any=false);

        vector<SpottingExemplar*> feedback(unsigned long id, const vector<int>& userClassifications, vector<pair<unsigned long, string> >* toRemoveExemplars);

        void checkIncomplete();
        void needExemplar(string ngram);

    private:
        //priority_queue<Spotting*, vector<Spotting*>, pcomparison> queue;
        set<Spotting*, pcomparison> queue;
        map<unsigned long, NewExemplarsBatch*> returnMap;
        map<unsigned long, chrono::system_clock::time_point> timeMap;
        map<unsigned long, NewExemplarsBatch*> doneMap;
        map<unsigned long, chrono::system_clock::time_point> timeDoneMap;
        mutex mutLock;

        map<string,bool> need;

        void remove(unsigned long id);

        void lock() { mutLock.lock(); }
        void unlock() { mutLock.unlock(); }
};
#endif
