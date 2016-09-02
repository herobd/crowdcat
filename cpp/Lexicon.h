#ifndef LEXICON_H
#define LEXICON_H

#include <regex>
#include <iostream>
#include <fstream>

#include <assert.h>

using namespace std;

class Meta
{

};

class Lexicon
{
public:
    //Max does not represent the maximum number of items to return, but a cut off, meaning if more than these are produced the results will be discarded. Thus the Lexicon can stop searching then.
    vector<string> search(string query, Meta meta, int max=-1);
    bool readIn(string fileName);
    static Lexicon* instance()
    {
        if (self==NULL) {
            self=new Lexicon();
        }
        return self;
    }
private:
    Lexicon()
    {
        words_lineSeperated="";
    }
    static Lexicon* self;
    
    vector<string> words;
    string words_lineSeperated;
};


#endif
