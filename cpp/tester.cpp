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

    ifstream& firstCATTSS("save/0_"
}
