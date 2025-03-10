#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cctype>

using namespace std;

bool isValid(const string &s)
{
    for (char c : s)
    {
        if (!isalpha(c) && !isspace(c))
        {
            return false;
        }
    }
    return true;
}

string reverseWords(const string &s)
{
    vector<string> words;
    vector<int> leadingSpaces;  // 记录每个单词前的空格数量
    vector<int> trailingSpaces; // 记录每个单词后的空格数量
    string word;
    int spaceCount = 0;

    for (size_t i = 0; i < s.length(); ++i)
    {
        if (isspace(s[i]))
        {
            // 记录空格数量
            spaceCount++;
        }
        else
        {
            if (spaceCount > 0)
            {
                trailingSpaces.push_back(spaceCount);
                spaceCount = 0;
            }
            word.push_back(s[i]);
        }

        if ((i == s.length() - 1 || isspace(s[i + 1])) && !word.empty())
        {
            words.push_back(word);
            word.clear();
            leadingSpaces.push_back(spaceCount);
            spaceCount = 0;
        }
    }

    reverse(words.begin(), words.end());
    reverse(leadingSpaces.begin(), leadingSpaces.end());
    reverse(trailingSpaces.begin(), trailingSpaces.end());

    ostringstream oss;
    for (size_t i = 0; i < words.size(); ++i)
    {
        if (i != 0)
        {
            oss << string(leadingSpaces[i - 1], ' ');
        }
        oss << words[i];
        if (i != words.size() - 1)
        {
            oss << string(trailingSpaces[i], ' ');
        }
    }

    return oss.str();
}

int main()
{
    string s;
    cout << "请输入一句话: ";
    getline(cin, s);

    if (!isValid(s))
    {
        cout << "输入无效，请输入一句由英文字母和空格组成的话。" << endl;
        return 1;
    }

    cout << reverseWords(s) << endl;
    return 0;
}