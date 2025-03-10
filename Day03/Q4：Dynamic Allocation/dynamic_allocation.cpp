#include <iostream>

using namespace std;

// 创建单向链表节点结构体
struct ListNode
{
    int val;
    ListNode *next;
    ListNode(int x) : val(x), next(nullptr)
    {
        cout << "创建了值为 " << x << " 的节点。" << endl;
    }
    ~ListNode()
    {
        cout << "释放了值为 " << val << " 的节点。" << endl;
    }
};

int main()
{
    // 创建包含5个节点的链表
    ListNode *head = new ListNode(1);
    head->next = new ListNode(2);
    head->next->next = new ListNode(3);
    head->next->next->next = new ListNode(4);
    head->next->next->next->next = new ListNode(5);

    // 遍历链表并打印节点值
    ListNode *current = head;
    cout << endl;
    while (current != nullptr)
    {
        cout << "节点值: " << current->val << " " << endl;
        current = current->next;
    }
    cout << endl;

    // 释放内存
    current = head;
    while (current != nullptr)
    {
        ListNode *temp = current;
        current = current->next;
        delete temp;
    }

    return 0;
}