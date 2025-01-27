#include <iostream>
#include <mutex>

struct Node
{
    int value;
    Node* next;
    std::unique_ptr<std::mutex> node_mutex;
    Node(int val) : value(val), next(nullptr), node_mutex(std::make_unique<std::mutex>()) {}   // ����������� ��������� � �������������� ��������
    ~Node() = default;                                                                         // ���������� ��������� ��� �������������� ������ ������, unique_ptr ��� ��������� ������
};

class FineGrainedQueue
{
    Node* head;
    std::unique_ptr<std::mutex> queue_mutex;
public:
    FineGrainedQueue(Node* node) : head(node), queue_mutex(std::make_unique<std::mutex>()) {}  // ����������� ������ � �������������� ���������� 'Node'
    ~FineGrainedQueue() = default;                                                             // ���������� ������ ��� �������������� ������ ������, unique_ptr ��� ��������� ������

    void insertIntoMiddle(int value, int pos)
    {
        pos -= 2;                                                       // ���������������� ����� ������� � ��������� � ������ ��������������� ������������ ������������� ����������
        std::unique_ptr<Node> newNode = std::make_unique<Node>(value);  // ���������� ����������� ���� � ���������������� ��� �������� ���������

        {
            std::lock_guard<std::mutex> lock(*queue_mutex);  // �������� ������� ������� �������
            Node* curNode = head;                            // ��������� ���������������� �������� ���� �������� 'head'
            for (int i = 0; i < pos; i++)                    // ��������� �� ������ �� ���� � �������� ������� ��� �� ����� ������
            {
                if (!curNode->next) break;                   // ����� �� �����, ���� ��������� ����� ������
                curNode = curNode->next;                     // ������� � ���������� ����
            }
            std::lock_guard<std::mutex> curNodeLock(*curNode->node_mutex);  // �������� ������� �������� ��������
            std::lock_guard<std::mutex> newNodeLock(*newNode->node_mutex);  // �������� ������� ������������ �������� ��� ���������� �������� �������

            if (curNode->next)                      // ���� ������ �� ������ ���� ����/����
            {
                newNode->next = curNode->next;      // �������� ���� ����� ������ � �������������� ��������� �� ����������� ����
                curNode->next = newNode.release();  // �������� �������� ���������� �� ����� ���� �������� ����
            }
            else                                    // �����
            {
                curNode->next = newNode.release();  // �������� ���� � ����� ������ ��� ������������� ��������� �� ��������� �������
            }
        }
    }

    void printQueue() const           // ������� ������ ����������� ������ ��� �������� ������ ��������� ���������
    {
        Node* curNode = head;
        while (curNode)
        {
            std::lock_guard<std::mutex> lock(*curNode->node_mutex);
            std::cout << curNode->value << " ";
            curNode = curNode->next;
        }
        std::cout << std::endl;
    }
};

int main()
{
    Node* node = new Node(0);          // ���������� ���� � �������������� ��������
    FineGrainedQueue queue(node);      // ������� ��������� ������
    for (int i = 2; i < 6; i++)
    {
        queue.insertIntoMiddle(i, i);  // �������� ���� � ������ � ��������� ������� � ���������� ����������
    }

    queue.insertIntoMiddle(100, 2);
    queue.insertIntoMiddle(200, 1000);

    queue.printQueue();                // ������� ���������� �������

    return 0;
}