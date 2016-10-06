#ifndef LEXICON_H
#define LEXICON_H

#include <regex>
#include <iostream>
#include <fstream>
#include <map>

#include <assert.h>

using namespace std;

//Max does not represent the maximum number of items to return, but a cut off, meaning if more than these are produced the results will be discarded. Thus the Lexicon can stop searching then.
struct SearchMeta
{
    string field;
    int max;
    SearchMeta() : field(""), max(-1) {}
    SearchMeta(string field) : field(field), max(-1) {}
    SearchMeta(int max) : field(""), max(max) {}
    SearchMeta(string field, int max) : field(field), max(max) {}
    SearchMeta(const SearchMeta& other) : field(other.field), max(other.max) {}
    SearchMeta(ifstream& in) 
    {
        in>>max;
        in.get();//burn newline
        getline(in,field);
    }
    void save(ofstream& out)
    {
        out<<max<<"\n"<<field<<"\n";
    }
};

class Lexicon
{
public:
    //This performs the given regex query on a lexicon and returns the matches.
    //meta specifies parameters of the search
    vector<string> search(string query, SearchMeta meta) const;
    

    //This reads in a file (new word on each line), and stores it under the given field name. The empty string is the default lexicon
    bool readIn(string fileName, string field="");
    static Lexicon* instance()
    {
        if (self==NULL) {
            self=new Lexicon();
        }
        return self;
    }
    void load(string loadPrefix);
    void save(string savePrefix);

private:
    Lexicon()
    {
        //words_lineSeperated="";
    }
    static Lexicon* self;
    
    map<string, vector<string> > fields;
    //string words_lineSeperated;
};


#endif
