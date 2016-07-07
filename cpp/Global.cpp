#include "Global.h"
Global* Global::_self=NULL;
Global::Global()
{
    string filePaths[2] = {"./data/top500_bigrams_in_freq_order.txt" , "./data/top500_trigrams_in_freq_order.txt"};
    for (int i=0; i<1+(MAX_N-MIN_N); i++)
    {
        ifstream in(filePaths[i]);
        string ngram;
        while(read_line(in,ngram))
        {   
            transform(ngram.begin(), ngram.end(), ngram.begin(), ::tolower);
            ngramRanks[i+MIN_N].push_back(ngram);
        }
        in.close();
    }

}

Global* Global::knowledge()
{
    if (_self==NULL)
        _self=new Global();
    return _self;
}

int Global::getNgramRank(string ngram)
{
    transform(ngram.begin(), ngram.end(), ngram.begin(), ::tolower);
    const vector<string>& ranks = ngramRanks[ngram.length()];
    for (int i=0; i<ranks.size(); i++)
    {
        if (ranks[i].compare(ngram)==0)
            return i+1;
    }
    return ranks.size()+1;
}


