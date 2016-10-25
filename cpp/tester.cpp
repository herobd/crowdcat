#include "tester.h"
#include "CATTSS.h"
#include <iostream>
#include <fstream>

void Tester::testSave()
{
    CATTSS* cattss = new CATTSS( "lexicon",
                                "pageImageDir",
                                "segFile",
                                "model/CATTSS_BENTHAM",
                                "save/0_BENTHAM",
                                0,
                                0,
                                100,
                                100,
                                100000);
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
    Simulator sim;
    
    //Positive spottings
    //0
    string ngram0="th";
    vector<Location> locs0 ={
                Location(0, 53,295,481,452),
                Location(0, 689,306,802,455),
                Location(0, 270,476,360,611),
                Location(0, 668,1281,780,1433),
                Location(0, 925,1281,1036,1431)
            };
    vector<string> gt0 = {
                "UNKNOWN",
                "UNKNOWN",
                "UNKNOWN",
                "UNKNOWN",
                "UNKNOWN"
            };
    vector<int> labels = sim.spottings(ngram0,locs0,gt0);
    for (int i=0; i<labels.size(); i++)
        assert(labels[i]);

    //Negative examples
    //0
    string ngram1="th";
    vector<Location> locs1 ={
                Location(0, 442,284,520,464), 
                Location(0, 398,474,496,621),
                Location(0, 508,1298,696,1443),
                Location(0, 879,1302,938,1431),
                Location(0, 881,1167,978,1279)
            };
    vector<string> gt1 = {
                "UNKNOWN",
                "UNKNOWN",
                "UNKNOWN",
                "UNKNOWN",
                "0"
            };
    labels = sim.spottings(ngram1,locs1,gt1);
    for (int i=0; i<labels.size(); i++)
        assert(labels[i]==0);
}
