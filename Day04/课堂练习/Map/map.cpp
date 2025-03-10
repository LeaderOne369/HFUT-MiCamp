#include <iostream>
#include <unordered_map>
#include <string>

using namespace std;

int main()
{
    unordered_map<int, string> m;

    m[1] = "abc";
    m[2] = "bcd";
    m[3] = "cde";

    for (auto it = m.begin(); it != m.end(); ++it)
    {
        cout << it->first << " ";
    }

    return 0;
}
