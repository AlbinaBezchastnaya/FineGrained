#include <iostream>
#include <mutex>

struct Node
{
    int value;
    Node* next;
    std::unique_ptr<std::mutex> node_mutex;
    Node(int val) : value(val), next(nullptr), node_mutex(std::make_unique<std::mutex>()) {}   // конструктор структуры с инициализацией значения
    ~Node() = default;                                                                         // деструктор структуры для предотвращения утечек памяти, unique_ptr сам освободит память
};

class FineGrainedQueue
{
    Node* head;
    std::unique_ptr<std::mutex> queue_mutex;
public:
    FineGrainedQueue(Node* node) : head(node), queue_mutex(std::make_unique<std::mutex>()) {}  // конструктор класса с инициализацией структурой 'Node'
    ~FineGrainedQueue() = default;                                                             // деструктор класса для предотвращения утечек памяти, unique_ptr сам освободит память

    void insertIntoMiddle(int value, int pos)
    {
        pos -= 2;                                                       // откорректировать номер позиции в алгоритме с учетом психологических особенностей человеческого восприятия
        std::unique_ptr<Node> newNode = std::make_unique<Node>(value);  // определить вставляемый узел и инициализировать его заданным значением

        {
            std::lock_guard<std::mutex> lock(*queue_mutex);  // залочить мьютекс очереди целиком
            Node* curNode = head;                            // присвоить вспомогательному текущему узлу значение 'head'
            for (int i = 0; i < pos; i++)                    // двигаться по списку до узла с заданным номером или до конца списка
            {
                if (!curNode->next) break;                   // выйти из цикла, если достигнут конец списка
                curNode = curNode->next;                     // перейти к следующему узлу
            }
            std::lock_guard<std::mutex> curNodeLock(*curNode->node_mutex);  // залочить мьютекс текущего элемента
            std::lock_guard<std::mutex> newNodeLock(*newNode->node_mutex);  // залочить мьютекс вставляемого элемента для дальнейших операций вставки

            if (curNode->next)                      // если дальше по списку есть узел/узлы
            {
                newNode->next = curNode->next;      // вставить узел между узлами с инициализацией указателя на последующий узел
                curNode->next = newNode.release();  // передать владение указателем на новый узел текущему узлу
            }
            else                                    // иначе
            {
                curNode->next = newNode.release();  // вставить узел в конец списка без инициализации указателя на следующий элемент
            }
        }
    }

    void printQueue() const           // функция вывода содержимого списка для проверки работы алгоритма программы
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
    Node* node = new Node(0);          // определить узел с инициализацией значения
    FineGrainedQueue queue(node);      // создать ненулевой список
    for (int i = 2; i < 6; i++)
    {
        queue.insertIntoMiddle(i, i);  // вставить узлы в список в различные позиции с различными значениями
    }

    queue.insertIntoMiddle(100, 2);
    queue.insertIntoMiddle(200, 1000);

    queue.printQueue();                // вывести содержимое очереди

    return 0;
}