
#include "MasterQueue.h"
#include <assert.h>

int main() {
    //TEST
    
    MasterQueue q;
    vector<SpottingsBatch*> todo;
    for (int i=0; i<3; i++) {
        SpottingsBatch* s = q.getBatch(5,300);
        todo.push_back(s);
        
    }
    for (SpottingsBatch* s : todo)
    {
        if (s!=NULL)
        {
            assert(s->size()<9);
            
            vector<string> ids;
            vector<int> labels;
            for (int i=0; i<s->size(); i++)
            {
                ids.push_back(to_string(s->at(i).id));
                labels.push_back(i%2);
                cv::imshow("ttt",s->at(i).img());
                cv::waitKey(250);
            }
            vector<Spotting>* res = q.feedback(s->spottingResultsId,ids,labels);
            //assert(res->size() == s->size()/2);
        }
    }
    
    MasterQueue q2;
    for (int i=0; i<3; i++) {
        SpottingsBatch* s = q2.getBatch(5,300);
        if (s!=NULL)
        {
            assert(s->size()<9);
            
            vector<string> ids;
            vector<int> labels;
            for (int ii=0; ii<s->size(); ii++)
            {
                ids.push_back(to_string(s->at(ii).id));
                labels.push_back(ii%2);
                cv::imshow("ttt",s->at(ii).img());
                cv::waitKey(100);
            }
            vector<Spotting>* res = q2.feedback(s->spottingResultsId,ids,labels);
            //assert(res->size() == s->size()/2);
        }
    }
    
    MasterQueue q3;
    #pragma omp parallel num_threads(3)
    {
        vector<string> ids;
        vector<int> labels;
        SpottingsBatch* s = q3.getBatch(5,300);
        if (s!=NULL)
        {
            assert(s->size()<9);
            
            
            for (int i=0; i<s->size(); i++)
            {
                ids.push_back(to_string(s->at(i).id));
                labels.push_back(i%2);
            }
        }
        #pragma omp barrier
        if (s!=NULL)
            vector<Spotting>* res = q3.feedback(s->spottingResultsId,ids,labels);
            //assert(res->size() == s->size()/2);
        
    }
}
