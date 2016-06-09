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
    vector<string> search(string query, Meta meta);
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
