
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
/**/
    Lexicon::instance()->readIn("/home/brian/intel_index/data/wordsEnWithNames.txt");
    
    
    Knowledge::Corpus* c0=new Knowledge::Corpus();
    c0->addWordSegmentaionAndGT("/home/brian/intel_index/data/gw_20p_wannot", "data/queries.gtp");
    MasterQueue* q0=new MasterQueue();
    int pageId = 2700270;
    
    vector<pair<unsigned long,string> > toRemoveSpottings;    
    Spotting s0(1588,851,1704,902,pageId,c0->imgForPageId(pageId),"th",0.3);
    vector<Spotting> sr0 = {s0};
    q0->updateSpottingResults(new vector<Spotting>(sr0));
    SpottingsBatch* sb0 = q0->getSpottingsBatch(5,false,400,0,"");
    vector<string> ids0= {to_string(sb0->at(0).id)};
    vector<int> labels0= {1};
    vector<Spotting>* toAdd;
    toAdd = q0->feedback(sb0->spottingResultsId, ids0, labels0, 0,&toRemoveSpottings);
    assert(toRemoveSpottings.size()==0 && toAdd->size()==1);
    vector<string> ids0r= {to_string(sb0->at(0).id)};
    vector<int> labels0r= {0};
    toAdd= q0->feedback(sb0->spottingResultsId, ids0r, labels0r, 1,&toRemoveSpottings);
    assert(toAdd && toAdd->size()==0 && toRemoveSpottings.size()==1 && toRemoveSpottings[0].second.compare("th")==0); 

    toRemoveSpottings.clear();

    Spotting s01(1644,851,1779,902,pageId,c0->imgForPageId(pageId),"he",0.3);
    vector<Spotting> sr01 = {s01};
    q0->updateSpottingResults(new vector<Spotting>(sr01));
    SpottingsBatch* sb01 = q0->getSpottingsBatch(5,false,400,0,"");
    vector<string> ids01= {to_string(sb01->at(0).id)};
    vector<int> labels01= {0};
    toAdd= q0->feedback(sb01->spottingResultsId, ids01, labels01, 0,&toRemoveSpottings);
    assert(toAdd && toAdd->size()==0 && toRemoveSpottings.size()==0);
    vector<string> ids01r= {to_string(sb01->at(0).id)};
    vector<int> labels01r= {1};
    toAdd = q0->feedback(sb01->spottingResultsId, ids01r, labels01r, 1,&toRemoveSpottings);
    assert(toRemoveSpottings.size()==0 && toAdd->size()==1); 
    //TODO add more
    delete c0;
    delete q0;

    c0=new Knowledge::Corpus();
    c0->addWordSegmentaionAndGT("/home/brian/intel_index/data/gw_20p_wannot", "data/queries.gtp");
    
    
    Spotting s02(1588,851,1704,902,pageId,c0->imgForPageId(pageId),"th",0.3);
    vector<Spotting> sb02 = {s02};
    vector<Spotting*> newEx0;
    vector<TranscribeBatch*> newBatches0 = c0->updateSpottings(&sb02,NULL,NULL,&newEx0);
    assert(newBatches0.size()==0 && newEx0.size()==0);

    Spotting s03(1644,851,1779,902,pageId,c0->imgForPageId(pageId),"he",0.3);
    vector<Spotting> sb03 = {s03};
    newBatches0 = c0->updateSpottings(&sb03,NULL,NULL,&newEx0);
    assert(newBatches0.size()==0 && newEx0.size()==0);

    Spotting s04(1649,948,17070,982,pageId,c0->imgForPageId(pageId),"re",0.3);
    vector<Spotting> sb04 = {s04};
    vector<TranscribeBatch*> newBatches0 = c0->updateSpottings(&sb04,NULL,NULL,&newEx0);
    assert(newBatches0.size()==0 && newEx0.size()==0);
    
    Spotting s05(1708,943,1756,984,pageId,c0->imgForPageId(pageId),"st",0.3);
    vector<Spotting> sb05 = {s05};
    vector<TranscribeBatch*> newBatches0 = c0->updateSpottings(&sb05,NULL,NULL,&newEx0);
    assert(newBatches0.size()==1 && newEx0.size()==1);
    


    vector<Spotting*> newEx0r;
    vector<Spotting> empty;
    vector<pair<unsigned long,string> > removeThis0 = {make_pair(s04.id,s04.ngram)};
    vector<unsigned long> toRemove0;
    vector<TranscribeBatch*> newBatches0r = c0->updateSpottings(&empty,&removeThis0,&toRemove0,&newEx0r);
    assert(newBatches0r.size()==0 && toRemove0.size()==1);
    assert(toRemove0[0] == newBatches0[0]->getId());

            //Test updateSpottingResults()
    Knowledge::Corpus c;
    c.addWordSegmentaionAndGT("/home/brian/intel_index/data/gw_20p_wannot", "data/queries.gtp");
    MasterQueue q;

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



    //Test Spotters functionality
    TestMasterQueue tq;//This has the test results in it
    FacadeSpotter spotter(&tq,&c,"");

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
    Spotting* e8= new Spotting(0,0,10,10,pageId,c.imgForPageId(pageId),"at",0);
    unsigned long e8id = e8->id;
    vector<Spotting*> exemplars2 = {e5,e6,e7,e8};
    spotter.addQueries(exemplars2);
    vector<pair<unsigned long,string> > remove1 = {make_pair(e8id,"at")};
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
    
    this_thread::sleep_for (::chrono::seconds(4));
    
    Spotting* e12= new Spotting(0,0,10,10,pageId,c.imgForPageId(pageId),"te",0);
    Spotting* e13= new Spotting(0,0,10,10,pageId,c.imgForPageId(pageId),"te",0);
    Spotting* e14= new Spotting(0,0,10,10,pageId,c.imgForPageId(pageId),"te",0);
    unsigned long e14id = e14->id;
    Spotting* e15= new Spotting(0,0,10,10,pageId,c.imgForPageId(pageId),"th",0);
    Spotting* e16= new Spotting(0,0,10,10,pageId,c.imgForPageId(pageId),"re",0);
    vector<Spotting*> exemplars4 = {e12,e13,e14,e15,e16};
    spotter.addQueries(exemplars4);
    vector<pair<unsigned long,string> > remove3 = {make_pair(e14id,"te")};
    spotter.removeQueries(&remove3);
    
    this_thread::sleep_for (::chrono::seconds(4));
    spotter.stop();
    }
}
/**/
/*
    //Manual testbed
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
/**/



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
