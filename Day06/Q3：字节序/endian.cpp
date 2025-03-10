#include <iostream>
#include <bit>
#include <arpa/inet.h>
using namespace std;

// 方法 1：使用整数和指针
void check1()
{
    int num = 1;
    char *byte = reinterpret_cast<char *>(&num);
    cout << "方法一: " << ((*byte == 1) ? "小端" : "大端") << endl;
}
// 方法 2：使用 union 结构
void check2()
{
    union
    {
        uint32_t i;
        uint8_t c[4];
    } test = {0x01020304};

    cout << "方法二: " << ((test.c[0] == 1) ? "大端" : "小端") << endl;
}
// 方法 3：使用 htonl() 函数
void check3()
{
    int num = 0x12345678;
    cout << "方法三: " << ((htonl(num) == num) ? "大端" : "小端") << endl;
}
// 方法 4：使用 C++20 的 std::endian
void check4()
{
#if __cplusplus >= 202002L
    if constexpr (std::endian::native == std::endian::little)
    {
        cout << "方法四: 小端" << endl;
    }
    if constexpr (std::endian::native == std::endian::big)
    {
        cout << "方法四: 大端" << endl;
    }
#else
    cout << "请检查是否使用C++20进行编译。" << endl;
#endif
}

int main()
{
    check1();
    check2();
    check3();
    check4();
    return 0;
}