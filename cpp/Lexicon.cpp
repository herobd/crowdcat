#include "Lexicon.h"
//#include <ctime>

Lexicon* Lexicon::self=NULL;

vector<string> Lexicon::search(string query, SearchMeta meta) const
{
    vector<string> ret1;//, ret2;

    auto iter = fields.find(meta.field);
    if (iter == fields.end())
        return ret1;
    
    
    //clock_t start = clock();
    regex q(query);
    const vector<string>& words = iter->second;
    for (const string& word : words)
    {
    
        if (regex_match (word, q ))
        {
            ret1.push_back(word);
            if (meta.max>0 && ret1.size()>meta.max)
                break;
        }
    }
    /*clock_t time1 = clock()-start;
    
    start = clock();
    string s = words_lineSeperated;
    //regex e("^"+query+"$");
    regex e("(?:\\n)("+query+")(?:\\n)");
    smatch m;
    while (std::regex_search (s,m,e)) {
        //for (auto x:m) 
        //    ret2.push_back(x.str());
        ret2.push_back(m[1].str());
        
        s = m.suffix().str();
    }
    clock_t time2 = clock()-start;
    
    cout <<"search times, vector: "<<time1<<"  string: "<<time2<<"  for query "<<query<<endl;
    assert(ret1.size() == ret2.size());
    for (int i=0; i<ret1.size(); i++)
    {
        assert(ret1[i].compare(ret2[i])==0);
    }*/
#ifdef TEST_MODE
    cout << query <<": ";
    if (meta.max>0 && ret1.size()>meta.max)
        cout<<" MAXED";
    else
        for (string s : ret1)
            cout<<s<<", ";
    cout<<endl;
#endif
    return ret1;
}

bool Lexicon::readIn(string fileName, string field)
{
    
    ifstream in(fileName);
    string word;
    regex notWordChar ("[^\\w]");
    vector<string>& words = fields[field];
    
    while(getline(in,word))
    {
        word = regex_replace (word,notWordChar,"");
        
        //words_lineSeperated+=word+"\n";
        words.push_back(word);
    }
    in.close();
    return true;
}
