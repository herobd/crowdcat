
#include "MasterQueue.h"
#include "TestQueue.h"
#include "Knowledge.h"
#include "Lexicon.h"
#include "TranscribeBatchQueue.h"
#include <assert.h>
#include <iostream>
#include "FacadeSpotter.h"

int main() {
    //TEST

    Knowledge::Corpus c;
    c.addWordSegmentaionAndGT("/home/brian/intel_index/data/gw_20p_wannot", "data/queries.gtp");
    MasterQueue q;
    int pageId = 2700270;
    
    Spotting s1(950,786,1014,823,pageId,c.imgForPageId(pageId),"of",0.3);
    Spotting s2(953,767,1020,835,pageId,c.imgForPageId(pageId),"of",0.1);
    Spotting s3(1692,790,1738,815,pageId,c.imgForPageId(pageId),"er",0.3);
    Spotting s4(1715,788,1761,815,pageId,c.imgForPageId(pageId),"re",0.1);
    vector<Spotting> sr1 = {s1};
    q.updateSpottingResults(new vector<Spotting>(sr1));
    vector<Spotting> sr2 = {s2};
    q.updateSpottingResults(new vector<Spotting>(sr2));
    SpottingsBatch* sb1 = q.getSpottingsBatch(5,false,400,0,"");
    SpottingsBatch* sb2 = q.getSpottingsBatch(5,false,400,0,"");
    assert(sb1->size()==1);
    assert(fabs(sb1->at(0).score-.1)<0.0001);
    assert(sb2==NULL);
    vector<Spotting> sr3 = {s3};
    q.updateSpottingResults(new vector<Spotting>(sr3));
    vector<Spotting> sr4 = {s4};
    q.updateSpottingResults(new vector<Spotting>(sr4));
    SpottingsBatch* sb3 = q.getSpottingsBatch(5,false,400,0,"");
    SpottingsBatch* sb4 = q.getSpottingsBatch(5,false,400,0,"");
    assert(sb3->size()==1);
    assert(fabs(sb3->at(0).score-.3)<0.0001);
    assert(sb4->size()==1);
    assert(fabs(sb4->at(0).score-.1)<0.0001);

    TestMasterQueue tq;//This has the test results in it
    FacadeSpotter spotter(&tq,&c,"",1);

#pragma omp parallel sections num_threads(2)
{
#pragma omp section
    {
    spotter.run(2);
    }
#pragma omp section
    {
    Spotting* e1= new Spotting(0,0,10,10,pageId,c.imgForPageId(pageId),"an",0);
    Spotting* e2= new Spotting(0,0,10,10,pageId,c.imgForPageId(pageId),"an",0);
    Spotting* e3= new Spotting(0,0,10,10,pageId,c.imgForPageId(pageId),"ar",0);
    Spotting* e4= new Spotting(0,0,10,10,pageId,c.imgForPageId(pageId),"at",0);
    vector<Spotting*> exemplars1 = {e1,e2,e3,e4};
    spotter.addQueries(exemplars1);
    
    this_thread::sleep_for (::chrono::seconds(4));

    Spotting* e5= new Spotting(0,0,10,10,pageId,c.imgForPageId(pageId),"an",0);
    Spotting* e6= new Spotting(0,0,10,10,pageId,c.imgForPageId(pageId),"an",0);
    Spotting* e7= new Spotting(0,0,10,10,pageId,c.imgForPageId(pageId),"ar",0);
    unsigned long e7id = e7->id;
    Spotting* e8= new Spotting(0,0,10,10,pageId,c.imgForPageId(pageId),"at",0);
    vector<Spotting*> exemplars2 = {e5,e6,e7,e8};
    spotter.addQueries(exemplars1);
    vector<pair<unsigned long,string> > remove1 = {make_pair(e7id,"ar")};
    spotter.removeQueries(&remove1);

    this_thread::sleep_for (::chrono::seconds(4));

    Spotting* e9= new Spotting(0,0,10,10,pageId,c.imgForPageId(pageId),"an",0);
    unsigned long e9id = e9->id;
    Spotting* e10= new Spotting(0,0,10,10,pageId,c.imgForPageId(pageId),"ar",0);
    Spotting* e11= new Spotting(0,0,10,10,pageId,c.imgForPageId(pageId),"at",0);
    vector<Spotting*> exemplars3 = {e9,e10,e11};
    spotter.addQueries(exemplars3);
    vector<pair<unsigned long,string> > remove2 = {make_pair(e9id,"an")};
    spotter.removeQueries(&remove2);
    
    spotter.stop();
    }
}
    /*//Manual testbed
    Lexicon::instance()->readIn("/home/brian/intel_index/data/wordsEnWithNames.txt");
    Knowledge::Corpus c;
    c.addWordSegmentaionAndGT("/home/brian/intel_index/data/gw_20p_wannot", "data/queries.gtp");
    TranscribeBatchQueue q;
    int pageId = 2700270;//c.addPage("/home/brian/intel_index/data/gw_20p_wannot/2700270.tif");
    assert(c.imgForPageId(pageId)->rows>0);
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
        
        Spotting s(tlx, tly, brx, bry, pageId, c.imgForPageId(pageId), ngram, 0.1);
        vector<Spotting> toAdd={s};
        vector<Spotting*> newExemplars;
        vector<TranscribeBatch*> bs = c.updateSpottings(&toAdd,NULL,NULL,&newExemplars);
        q.enqueueAll(bs);
        if (newExemplars.size()>0)
        {
            cout <<"harvested:"<<endl;
            for (int i=1; i<newExemplars.size(); i++)
                imshow("har: "+newExemplars[i]->ngram,newExemplars[i]->img());
            cv::waitKey();
        }
    }
    
    while(1)
    {
        TranscribeBatch* b = q.dequeue(500);
        if (b==NULL)
            break;
        cout<<"Spotted : ";
        for (SpottingPoint sp : b->getSpottingPoints())
            cout<<sp.getNgram()<<", ";
        cout<<endl;
        cout<<"Poss: "<<endl;
        for (string p : b->getPossibilities())
            cout<<"  "<<p<<endl;
        cv::imshow("word",b->getImage());
        //cv::imshow("ngrams",b->getTextImage());
        cv::waitKey();
        string trans;
        cout << "transcription: ";
        cin >> trans;
        vector<Spotting*> harvested = b->getBackPointer()->result(trans);
        for (Spotting* s: harvested)
        {
            imshow("har: "+s->ngram,s->img());
            delete s;
        }
        cv::waitKey();
    }
    c.show();
*/



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
            vector<Spotting> res = q.feedback(s->spottingResultsId,ids,labels);
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
            vector<Spotting> res = q2.feedback(s->spottingResultsId,ids,labels);
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
            vector<Spotting> res = q3.feedback(s->spottingResultsId,ids,labels);
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
            vector<Spotting> res = q4.feedback(s->spottingResultsId,ids,labels);
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
            vector<Spotting> res = q5.feedback(s->spottingResultsId,ids,labels);
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
                vector<Spotting> res = q6.feedback(s->spottingResultsId,ids,labels);
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
                vector<Spotting> res = q7.feedback(s->spottingResultsId,ids,labels);
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
                vector<Spotting> res = q8.feedback(s->spottingResultsId,ids,labels);
                //assert(res->size() == s->size()/2);
                delete res;
                delete s;
            }
        } 
    }*/
}
