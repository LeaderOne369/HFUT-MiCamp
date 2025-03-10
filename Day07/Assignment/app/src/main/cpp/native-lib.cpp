#include <jni.h>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <cctype>

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_assignment_MainActivity_stringFromJNI(
    JNIEnv *env,
    jobject /* this */)
{
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_assignment_MainActivity_reverseStringJNI(
    JNIEnv *env,
    jobject /* this */,
    jstring input_string)
{

    const char *str = env->GetStringUTFChars(input_string, nullptr);
    std::string s(str);

    env->ReleaseStringUTFChars(input_string, str);

    for (char c : s)
    {
        if (!isalpha(c) && !isspace(c))
        {
            return env->NewStringUTF("输入无效，请只输入英文字母和空格");
        }
    }

    std::vector<std::string> words;
    std::vector<int> leadingSpaces;
    std::vector<int> trailingSpaces;
    std::string word;
    int spaceCount = 0;

    for (size_t i = 0; i < s.length(); ++i)
    {
        if (isspace(s[i]))
        {
            spaceCount++;
        }
        else
        {
            if (spaceCount > 0 && word.empty())
            {
                leadingSpaces.push_back(spaceCount);
                spaceCount = 0;
            }
            word.push_back(s[i]);
        }

        if ((i == s.length() - 1 || isspace(s[i])) && !word.empty())
        {
            words.push_back(word);
            word.clear();
            if (i < s.length() - 1)
            {
                trailingSpaces.push_back(spaceCount);
            }
            spaceCount = 0;
        }
    }

    std::reverse(words.begin(), words.end());

    std::ostringstream oss;
    for (size_t i = 0; i < words.size(); ++i)
    {
        if (i > 0 && i - 1 < leadingSpaces.size())
        {
            oss << std::string(leadingSpaces[i - 1], ' ');
        }
        oss << words[i];
        if (i < trailingSpaces.size())
        {
            oss << std::string(trailingSpaces[i], ' ');
        }
    }

    std::string result = "反转字符串: " + oss.str();
    return env->NewStringUTF(result.c_str());
}