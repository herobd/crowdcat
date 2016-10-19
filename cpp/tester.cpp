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
