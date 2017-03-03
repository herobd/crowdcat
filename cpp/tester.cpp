#include "tester.h"
#include "CATTSS.h"
#include <iostream>
#include <fstream>
#include "Simulator.h"

void Tester::testSave()
{
    CATTSS* cattss = new CATTSS( "/home/brian/intel_index/data/wordsEnWithNames.txt",
                                "/home/brian/intel_index/data/bentham/BenthamDatasetR0-Images/Images/Pages",
                                "/home/brian/intel_index/data/bentham/ben_cattss_c_corpus.gtp",
                                "model/CATTSS_BENTHAM",
                                "save/0_BENTHAM",
                                30,
                                0,
                                0,
                                100,
                                100,
                                100000,
                                0);
    cattss->savePrefix="save/test";
    cattss->save();
    delete cattss;

    ifstream firstCATTSS("save/0_BENTHAM_CATTSS.sav");
    ifstream secondCATTSS("save/test_CATTSS.sav");

    vector<string> p1,p2;
    while (firstCATTSS.good() && secondCATTSS.good())
    {
        string line1, line2;
        getline(firstCATTSS,line1);
        getline(secondCATTSS,line2);
        if (line1.compare(line2) != 0)
        {
            cout<<line1<<" != "<<line2<<endl;
            assert(line1.compare(line2)==0);
        }

        p1.push_back(line1);
        p2.push_back(line2);
    }
    assert(!(firstCATTSS.good()) && !(secondCATTSS.good()));
    cout<<"testSave() passed"<<endl;
}

void Tester::testSimulator()
{
    //This needs to have perfect accuracy and never skip
    Simulator sim("test","/home/brian/intel_index/data/bentham/manual_segmentations.csv");
    
    //Positive spottings
    //0
    string ngram0="th";
    vector<Location> locs0 ={
                Location(1, 387,295,481,452),
                Location(1, 689,306,802,455),
                Location(1, 270,476,360,611),
                Location(1, 668,1281,780,1433),
                Location(1, 925,1281,1036,1431)
            };
    vector<string> gt0 = {
                "UNKNOWN",
                "UNKNOWN",
                "UNKNOWN",
                "UNKNOWN",
                "UNKNOWN"
            };
    vector<int> labels = sim.spottings(ngram0,locs0,gt0,"");
    for (int i=0; i<labels.size(); i++)
        assert(labels[i]);

    //Negative examples
    //0
    string ngram1="th";
    vector<Location> locs1 ={
                Location(1, 442,284,520,464), 
                Location(1, 398,474,496,621),
                Location(1, 508,1298,696,1443),
                Location(1, 879,1302,938,1431),
                Location(1, 881,1167,978,1279)
            };
    vector<string> gt1 = {
                "UNKNOWN",
                "UNKNOWN",
                "UNKNOWN",
                "UNKNOWN",
                "0"
            };
    labels = sim.spottings(ngram1,locs1,gt1,"");
    for (int i=0; i<labels.size(); i++)
        assert(labels[i]==0);
    
    //Positive examples, from sim
    /*
    string ngram1a="en";
    vector<Location> locs1a ={
                Location(15, 1705, 3079, 1790, 3237), 
                Location(18, 768, 1247, 853, 1405),
            };
    vector<string> gt1a = {
                "UNKNOWN",
                "UNKNOWN"
            };
    labels = sim.spottings(ngram1a,locs1a,gt1a,"");
    for (int i=0; i<labels.size(); i++)
        assert(labels[i]==0);
        */

    //newExemplars, mixed T,T,F,F,F
    vector<string> ngrams2= {"io","id","er", "at","al"};
    vector<Location> locs2 ={
                Location(1, 850,1495,908,1603),
                Location(1, 976,958,1084,1102),
                Location(1, 374,1148,446,1266),
                Location(1, 724,1501,817,1599),
                Location(1, 1040,488,1233,638)
            };
    labels = sim.newExemplars(ngrams2,locs2,"");
    assert(labels[0]==1);
    assert(labels[1]==1);
    assert(labels[2]==0);
    assert(labels[3]==0);
    assert(labels[4]==0);

    //newExemplarsi STRICT, mixed F,F,T
    vector<string> ngrams3= {"en","ou","it"};
    vector<Location> locs3 ={
                Location(1, 822,2839,863,2884),
                Location(1, 689,2687,749,2721),
                Location(1, 1102,2458,1153,2560)
            };
    labels = sim.newExemplars(ngrams3,locs3,"");
    assert(labels[0]==0);
    assert(labels[1]==0);
    assert(labels[2]==1);

    //transcription correct avail
    //that 1
    vector<SpottingPoint> spottings3 = {SpottingPoint(0,-1,"th",0,0,0, 1, 369,298,483,464)};
    vector<string> poss3 = {"this", "that", "they", "thus"};
    string trans = sim.transcription(1, spottings3, poss3, "that", false);
    assert(trans.compare("that")==0);

    //transcription correct not avail
    vector<SpottingPoint> spottings4 = {
                SpottingPoint(0,-1,"th",0,0,0, 1, 369,298,483,464),
            };
    vector<string> poss4 = {"this", "they", "thus"};
    trans = sim.transcription(1, spottings4, poss4, "that", false);
    assert(0==trans.compare("$ERROR$"));
    
    //transcription spotting error
    vector<SpottingPoint> spottings5 = {
                SpottingPoint(11,-1,"th",0,0,0, 1, 369,298,483,464),
                SpottingPoint(13,-1,"ey",0,0,0, 1, 463,373,557,467)
            };
    vector<string> poss5 = {"they", "theyd"};
    trans = sim.transcription(1, spottings5, poss5, "that", false);
    assert(0==trans.compare("$REMOVE:13$"));


    //manual
    vector<string> poss6;
    trans = sim.manual(1,poss6,"that",false);
    assert(0==trans.compare("that"));
}
