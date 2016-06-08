#ifndef LEXICON_H
#define LEXICON_H

#include <regex>
#include <iostream>

#include <assert.h>

using namespace std;

class Meta
{

};

class Lexicon
{
public:
    static vector<string> search(string query, Meta meta);
    static bool readIn(string fileName);
protected:
    Lexicon()
    {
        words_lineSeperated="";
    }
    static Lexicon* self;
    
    vector<string> words;
    string words_lineSeperated;
};


#endif
