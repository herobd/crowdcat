
#include "MasterQueue.h"
#include "TestQueue.h"
#include "Knowledge.h"
#include "Lexicon.h"
#include "TranscribeBatchQueue.h"
#include <assert.h>
#include <iostream>

int main() {
    //TEST
    Lexicon::instance()->readIn("/home/brian/intel_index/data/wordsEnWithNames.txt");
    Knowledge::Corpus c;
    TranscribeBatchQueue q;
    //Knowledge::Page* = new Knowledge::Page();
    int pageId=0;
    //c.addPage(pageId);
    cv::Mat page = cv::imread("/home/brian/intel_index/data/gw_20p_wannot/2700270.tif",CV_LOAD_IMAGE_GRAYSCALE);
    assert(page.rows>0);
    while(1)
    {
        int tlx, tly, brx, bry;
        string ngram;
        cout << "tlx: ";
        cin >> tlx;
        if (tlx==-1)
            break;
        cout << "tly: ";
        cin >> tly;
        if (tly==-1)
            break;
        cout << "brx: ";
        cin >> brx;
        if (brx==-1)
            break;
        cout << "bry: ";
        cin >> bry;
        if (bry==-1)
            break;
        cout << "ngram: ";
        cin >> ngram;
        
        Spotting s(tlx, tly, brx, bry, pageId, &page, ngram, 0.1);
        vector<TranscribeBatch*> bs = c.addSpotting(s);
        for (auto b : bs)
            if (b!=NULL)
            {
                cout<<"enqueued"<<endl;
                q.enqueue(b);
            }
    }
    
    while(1)
    {
        TranscribeBatch* b = q.dequeue();
        if (b==NULL)
            break;
        cout<<"Poss: "<<endl;
        for (string p : b->getPossibilities())
            cout<<"  "<<p<<endl;
        cv::imshow("word",b->getImage());
        cv::imshow("ngrams",b->getTextImage());
        cv::waitKey();
        string trans;
        cout << "transcription: ";
        cin >> trans;
        b->getBackPointer()->result(trans);
    }
    c.show();
    /*TestQueue t;
    int numUsers=8;
    for (int n=0; n<t.numTestBatches; n++)
    {
        vector<int> first(numUsers);
        #pragma omp parallel for num_threads(4)
        for (int i=0; i<numUsers; i++)
        {
            SpottingsBatch* b = t.getBatch(5, 400+i, i,0);
            first[i]=b->at(0).tlx;
            
            vector<string> ids;
            vector<int> labels;
            for (int ii=0; ii<b->size(); ii++)
            {
                ids.push_back(to_string(b->at(ii).id));
                labels.push_back(ii%2);
                
            }
            int fp, fn;
            assert(t.feedback(b->spottingResultsId,ids,labels,i, &fp, &fn) || n<t.numTestBatches-1);
            delete b;
        }
        
        for (int i=1; i<numUsers; i++)
        {
            assert(first[i-1] == first[i]);
        }
    }*/
    
    //MasterQueue q;
    //while (q.test_autoBatch()){}
    
    
    
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
