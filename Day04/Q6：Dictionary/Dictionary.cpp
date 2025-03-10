#include "Dictionary.h"

void Dictionary::add(const string &english, const string &chinese)
{
    dict[english] = chinese;
    dict[chinese] = english;
}

string Dictionary::translate(const string &word)
{
    if (dict.find(word) != dict.end())
    {
        return dict[word];
    }
    else
    {
        return "未收录这个词汇。";
    }
}