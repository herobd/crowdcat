
#include "MasterQueue.h"
#include <assert.h>

int main() {
    //TEST
    
    MasterQueue q;
    for (int iter=0; iter<100; iter++) {
        q.test_autoBatch();
    }
    /*vector<SpottingsBatch*> todo;
    for (int iter=0; iter<100; iter++) {
        cout <<"batch"<<endl;
        SpottingsBatch* s = q.getBatch(5,300);
        if (s==NULL)
            break;
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
                //cv::imshow("ttt",s->at(i).img());
                //cv::waitKey(250);
            }
            vector<Spotting>* res = q.feedback(s->spottingResultsId,ids,labels);
            delete res;
            //assert(res->size() == s->size()/2);
            
            delete s;
        }
    }
    
    MasterQueue q2;
    for (int iter=0; iter<100; iter++) {
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
                //cv::imshow("ttt",s->at(ii).img());
                //cv::waitKey(100);
            }
            vector<Spotting>* res = q2.feedback(s->spottingResultsId,ids,labels);
            //assert(res->size() == s->size()/2);
            delete res;
            delete s;
        }
        else 
            break;
    }
    
    MasterQueue q3;
    #pragma omp parallel
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
        {
            vector<Spotting>* res = q3.feedback(s->spottingResultsId,ids,labels);
            //assert(res->size() == s->size()/2);
            delete res;
            delete s;
        }
        
    }
    
    MasterQueue q4;
    for (int iter=0; iter<100; iter++) {
        SpottingsBatch* s = q4.getBatch(5,300);
        if (s!=NULL)
        {
            assert(s->size()<9);
            
            vector<string> ids;
            vector<int> labels;
            for (int ii=0; ii<s->size(); ii++)
            {
                ids.push_back(to_string(s->at(ii).id));
                labels.push_back(1);
            }
            vector<Spotting>* res = q4.feedback(s->spottingResultsId,ids,labels);
            //assert(res->size() == s->size()/2);
            delete res;
            delete s;
        }
        else 
            break;
    }
    
    MasterQueue q5;
    for (int iter=0; iter<100; iter++) {
        SpottingsBatch* s = q5.getBatch(5,300);
        if (s!=NULL)
        {
            assert(s->size()<9);
            
            vector<string> ids;
            vector<int> labels;
            for (int ii=0; ii<s->size(); ii++)
            {
                ids.push_back(to_string(s->at(ii).id));
                labels.push_back(0);
            }
            vector<Spotting>* res = q5.feedback(s->spottingResultsId,ids,labels);
            //assert(res->size() == s->size()/2);
            delete res;
            delete s;
        }
        else 
            break;
    }
    
    
    MasterQueue q6;
    for (int iter=0; iter<100; iter++) {
        todo.clear();
        for (int i=0; i<3; i++) {
            SpottingsBatch* s = q6.getBatch(5,300);
            if (s==NULL)
                break;
            todo.push_back(s);
        }
        if (todo.size()==0)
            break;
        for (SpottingsBatch* s : todo) {
            if (s!=NULL)
            {
                assert(s->size()<9);
                
                vector<string> ids;
                vector<int> labels;
                for (int i=0; i<s->size(); i++)
                {
                    ids.push_back(to_string(s->at(i).id));
                    labels.push_back(i%2);
                }
                vector<Spotting>* res = q6.feedback(s->spottingResultsId,ids,labels);
                //assert(res->size() == s->size()/2);
                delete res;
                delete s;
            }
        }
    }
    
    MasterQueue q7;
    for (int iter=0; iter<100; iter++) {
        todo.clear();
        for (int i=0; i<3; i++) {
            SpottingsBatch* s = q7.getBatch(5,700);
            if (s==NULL)
                break;
            todo.push_back(s);
        }
        if (todo.size()==0)
            break;
        for (int i=0; i<todo.size(); i++) {
            SpottingsBatch* s = todo[i];
            if (s!=NULL)
            {
                assert(s->size()<9);
                
                vector<string> ids;
                vector<int> labels;
                for (int i=0; i<s->size(); i++)
                {
                    ids.push_back(to_string(s->at(i).id));
                    labels.push_back(0);
                }
                vector<Spotting>* res = q7.feedback(s->spottingResultsId,ids,labels);
                //assert(res->size() == s->size()/2);
                delete res;
                delete s;
                SpottingsBatch* s2 = q7.getBatch(5,700);
                if (s2!=NULL && i<120)
                    todo.push_back(s2);
            }
        }
    }
    
    MasterQueue q8;
    #pragma omp parallel num_threads(3)
    {
        SpottingsBatch* s;
        for (int iter=0; iter<100 && s!=NULL; iter++) {
            vector<string> ids;
            vector<int> labels;
            //#pragma omp barrier
            s = q8.getBatch(5,300);
            if (s!=NULL)
            {
                assert(s->size()<9);
                
                
                for (int i=0; i<s->size(); i++)
                {
                    ids.push_back(to_string(s->at(i).id));
                    labels.push_back(i%2);
                }
            }
            //#pragma omp barrier
            if (s!=NULL)
            {
                vector<Spotting>* res = q8.feedback(s->spottingResultsId,ids,labels);
                //assert(res->size() == s->size()/2);
                delete res;
                delete s;
            }
        } 
    }*/
}
