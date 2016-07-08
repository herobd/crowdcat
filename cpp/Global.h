#ifndef GLOBAL_H
#define GLOABL_H

#include <map>
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>

using namespace std;

#define MIN_N 2
#define MAX_N 2
#define MAX_NGRAM_RANK 300
class Global
{
    private:
        Global();
        static Global* _self;

        map<int, vector<string> > ngramRanks;
    public:
        static Global* knowledge();

        int getNgramRank(string ngram);

};


#endif
