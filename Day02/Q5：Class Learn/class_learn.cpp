#include <iostream>
#include <string>
#include <vector>

using namespace std;

class Book
{
protected:
    string title;
    string author;
    int year;
    bool isBorrowed;

public:
    Book(string t, string a, int y)
        : title(t), author(a), year(y), isBorrowed(false) {}

    virtual ~Book() {}

    virtual void showInfo() const
    {
        cout << "书名: " << title << endl;
        cout << "作者: " << author << endl;
        cout << "出版年份: " << year << endl;
        cout << "是否借阅: " << (isBorrowed ? "是" : "否") << endl;
    }

    void borrow()
    {
        if (isBorrowed)
        {
            cout << "这本书已经被借出，无法再次借阅！" << endl;
        }
        else
        {
            isBorrowed = true;
            cout << "成功借阅《" << title << "》！" << endl;
        }
    }

    void returnBook()
    {
        if (isBorrowed)
        {
            isBorrowed = false;
            cout << "成功归还《" << title << "》！" << endl;
        }
        else
        {
            cout << "这本书并没有借出！" << endl;
        }
    }

    string getTitle() const
    {
        return title;
    }

    bool getIsBorrowed() const
    {
        return isBorrowed;
    }

    virtual void displayGenre() const = 0;
};

class Novel : public Book
{
public:
    Novel(string t, string a, int y)
        : Book(t, a, y) {}

    void showInfo() const override
    {
        Book::showInfo();
        cout << "类型: 小说" << endl;
    }

    void displayGenre() const override
    {
        cout << "这是一本小说。" << endl;
    }
};

class ScienceBook : public Book
{
public:
    ScienceBook(string t, string a, int y)
        : Book(t, a, y) {}

    void showInfo() const override
    {
        Book::showInfo();
        cout << "类型: 科技书籍" << endl;
    }

    void displayGenre() const override
    {
        cout << "这是一本科技书籍。" << endl;
    }
};

class ArtBook : public Book
{
public:
    ArtBook(string t, string a, int y)
        : Book(t, a, y) {}

    void showInfo() const override
    {
        Book::showInfo();
        cout << "类型: 艺术类书籍" << endl;
    }

    void displayGenre() const override
    {
        cout << "这是一本艺术类书籍。" << endl;
    }
};

int main()
{
    vector<Book *> library;

    library.push_back(new Novel("红楼梦", "曹雪芹", 1791));
    library.push_back(new ScienceBook("量子物理简明教程", "理查德·费曼", 1964));
    library.push_back(new ArtBook("艺术的故事", "恩斯特·贡布里希", 1950));

    for (const auto &book : library)
    {
        book->showInfo();
        book->displayGenre();
        cout << endl;
    }

    library[0]->borrow();
    library[0]->returnBook();

    for (auto &book : library)
    {
        delete book;
    }

    return 0;
}