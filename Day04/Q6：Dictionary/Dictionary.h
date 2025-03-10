#ifndef DICTIONARY_H
#define DICTIONARY_H
#include <iostream>
#include <map>
#include <string>
using namespace std;

class Dictionary
{
public:
    void add(const string &english, const string &chinese);
    string translate(const string &word);

private:
    map<string, string> dict;
};

#endif