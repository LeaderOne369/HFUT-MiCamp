#include <iostream>
using namespace std;

template <typename T>
class Add
{
public:
    T add(T a, T b)
    {
        return a + b;
    }
};

int main()
{
    Add<int> Add1;
    cout << "整数: " << Add1.add(3, 4) << endl;

    Add<double> Add2;
    cout << "浮点数: " << Add2.add(3.2, 4.2) << endl;

    return 0;
}