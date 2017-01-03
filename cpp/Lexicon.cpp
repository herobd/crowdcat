#include "Lexicon.h"
//#include <ctime>

Lexicon* Lexicon::self=NULL;

#if TRANS_DONT_WAIT
vector<string> Lexicon::search(string query, SearchMeta meta, const set<string>& reject) const
#else
vector<string> Lexicon::search(string query, SearchMeta meta) const
#endif
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
    
#if TRANS_DONT_WAIT
        if (regex_match (word, q ) && reject.find(word)==reject.end())
#else
        if (regex_match (word, q ))
#endif
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

bool Lexicon::inVocab(string word, string field)
{
    return find(fields[field].begin(), fields[field].end(), word)!=fields[field].end();
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
        if (find(words.begin(), words.end(), word) == words.end()) //prevent duplicates
            words.push_back(word);
    }
    in.close();
    return true;
}

void Lexicon::save(ofstream& out)
{
    //ofstream out(savePrefix+"_Lexicon.sav");
    out<<fields.size()<<"\n";
    for (auto p : fields)
    {
        out<<p.first<<"\n";
        out<<p.second.size()<<"\n";
        for (string s : p.second)
        {
            out<<s<<"\n";
        }
    }
    //out.close();
}
void Lexicon::load(ifstream& in)
{
    //ifstream in (loadPrefix+"_Lexicon.sav");
    int fSize;
    in>>fSize;
    in.get();
    for (int i=0; i<fSize; i++)
    {
        string fieldName;
        getline(in,fieldName);
        int wSize;
        in>>wSize;
        in.get();
        //fields[fieldName].resize(wSize);
        fields[fieldName];
        for (int j=0; j<wSize; j++)
        {
            string word;
            getline(in,word);
            //prevent dup, so that my change will work with current save file
            if (find(fields.at(fieldName).begin(), fields.at(fieldName).end(), word) == fields.at(fieldName).end())
                fields.at(fieldName).push_back(word);
        }
    }
    //in.close();
}

