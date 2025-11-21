#include <gtest/gtest.h>
#include "../include/queue.hpp"
#include <vector>
#include <algorithm>
#include <string>
#include <memory>

// Тестовая структура с несколькими полями
struct Employee {
    std::string name;
    int id;
    double salary;
    std::string department;
    
    Employee() : name(""), id(0), salary(0.0), department("") {}
    Employee(std::string n, int i, double s, std::string d) 
        : name(n), id(i), salary(s), department(d) {}
    
    bool operator==(const Employee& other) const {
        return name == other.name && id == other.id && 
               salary == other.salary && department == other.department;
    }
    
    bool operator<(const Employee& other) const {
        return id < other.id;
    }
};

// Вспомогательная функция для вывода
std::ostream& operator<<(std::ostream& os, const Employee& e) {
    os << "Employee{name='" << e.name << "', id=" << e.id 
       << ", salary=" << e.salary << ", department='" << e.department << "'}";
    return os;
}

// ==================== ТЕСТЫ ДЛЯ BLOCKMEMORYRESOURCE ====================

TEST(BlockMemoryResourceTest, BasicAllocationDeallocation) {
    BlockMemoryResource mr;
    
    // Выделение памяти
    void* ptr1 = mr.allocate(100, alignof(std::max_align_t));
    ASSERT_NE(ptr1, nullptr);
    
    void* ptr2 = mr.allocate(200, alignof(std::max_align_t));
    ASSERT_NE(ptr2, nullptr);
    
    // Освобождение
    EXPECT_NO_THROW(mr.deallocate(ptr1, 100, alignof(std::max_align_t)));
    EXPECT_NO_THROW(mr.deallocate(ptr2, 200, alignof(std::max_align_t)));
}

TEST(BlockMemoryResourceTest, ReuseFreedMemory) {
    BlockMemoryResource mr;
    
    // Выделяем и освобождаем блок
    void* ptr = mr.allocate(256, alignof(std::max_align_t));
    mr.deallocate(ptr, 256, alignof(std::max_align_t));
    
    // Должны повторно использовать освобожденный блок
    void* reused_ptr = mr.allocate(256, alignof(std::max_align_t));
    EXPECT_EQ(ptr, reused_ptr);  // Должен вернуть тот же блок
}

TEST(BlockMemoryResourceTest, DeallocateUnknownBlockThrows) {
    BlockMemoryResource mr;
    void* fake_ptr = reinterpret_cast<void*>(0xDEADBEEF);
    
    EXPECT_THROW(mr.deallocate(fake_ptr, 100, alignof(std::max_align_t)), 
                 std::invalid_argument);
}

// ==================== ТЕСТЫ ДЛЯ ОЧЕРЕДИ С INT ====================

TEST(QueueIntTest, DefaultConstructor) {
    Queue<int> q;
    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0);
}

TEST(QueueIntTest, PushIncreasesSize) {
    Queue<int> q;
    q.push(1);
    EXPECT_FALSE(q.empty());
    EXPECT_EQ(q.size(), 1);
    
    q.push(2);
    q.push(3);
    EXPECT_EQ(q.size(), 3);
}

TEST(QueueIntTest, FrontAndBackOperations) {
    Queue<int> q;
    
    q.push(10);
    EXPECT_EQ(q.front(), 10);
    EXPECT_EQ(q.back(), 10);
    
    q.push(20);
    EXPECT_EQ(q.front(), 10);  // Первый элемент не меняется
    EXPECT_EQ(q.back(), 20);   // Последний элемент обновился
    
    q.push(30);
    EXPECT_EQ(q.front(), 10);
    EXPECT_EQ(q.back(), 30);
}

TEST(QueueIntTest, PopOperations) {
    Queue<int> q;
    q.push(100);
    q.push(200);
    q.push(300);
    
    EXPECT_EQ(q.front(), 100);
    q.pop();
    
    EXPECT_EQ(q.front(), 200);
    EXPECT_EQ(q.size(), 2);
    
    q.pop();
    EXPECT_EQ(q.front(), 300);
    EXPECT_EQ(q.size(), 1);
    
    q.pop();
    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0);
}

TEST(QueueIntTest, PopOnEmptyQueueThrows) {
    Queue<int> q;
    EXPECT_THROW(q.pop(), std::runtime_error);
    
    q.push(1);
    q.pop();
    EXPECT_THROW(q.pop(), std::runtime_error);  // После очистки тоже должно бросать
}

TEST(QueueIntTest, FrontBackOnEmptyQueueThrows) {
    Queue<int> q;
    EXPECT_THROW(q.front(), std::runtime_error);
    EXPECT_THROW(q.back(), std::runtime_error);
}

TEST(QueueIntTest, ClearQueue) {
    Queue<int> q;
    for (int i = 0; i < 10; ++i) {
        q.push(i);
    }
    
    EXPECT_EQ(q.size(), 10);
    q.clear();
    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0);
}

// ==================== ТЕСТЫ НА ПРАВИЛО ПЯТИ ====================

TEST(QueueIntTest, CopyConstructor) {
    Queue<int> q1;
    q1.push(1);
    q1.push(2);
    q1.push(3);
    
    Queue<int> q2(q1);  // Копирующий конструктор
    
    EXPECT_EQ(q1.size(), q2.size());
    
    // Проверяем, что элементы одинаковые
    while (!q1.empty()) {
        EXPECT_EQ(q1.front(), q2.front());
        q1.pop();
        q2.pop();
    }
}

TEST(QueueIntTest, MoveConstructor) {
    Queue<int> q1;
    q1.push(42);
    q1.push(43);
    
    Queue<int> q2(std::move(q1));  // Перемещающий конструктор
    
    EXPECT_TRUE(q1.empty());
    EXPECT_EQ(q1.size(), 0);
    EXPECT_EQ(q2.size(), 2);
    EXPECT_EQ(q2.front(), 42);
}



// ==================== ТЕСТЫ ДЛЯ ОЧЕРЕДИ С STRUCT EMPLOYEE ====================

TEST(QueueEmployeeTest, PushAndAccessComplexObject) {
    Queue<Employee> q;
    
    Employee e1("Alice", 1001, 50000.0, "Engineering");
    Employee e2("Bob", 1002, 45000.0, "Sales");
    
    q.push(e1);
    q.push(e2);
    
    EXPECT_EQ(q.front().name, "Alice");
    EXPECT_EQ(q.front().id, 1001);
    EXPECT_EQ(q.front().department, "Engineering");
    
    EXPECT_EQ(q.back().name, "Bob");
    EXPECT_EQ(q.back().id, 1002);
}

TEST(QueueEmployeeTest, PerfectForwardingWithStruct) {
    Queue<Employee> q;
    
    // Используем perfect forwarding - создаем объект на месте
    q.push(Employee("Charlie", 1003, 60000.0, "Marketing"));
    q.push({"David", 1004, 70000.0, "HR"});  // Uniform initialization
    
    EXPECT_EQ(q.front().name, "Charlie");
    EXPECT_EQ(q.back().name, "David");
}

TEST(QueueEmployeeTest, CopyAndMoveWithStruct) {
    Queue<Employee> q1;
    q1.push(Employee("Eve", 1005, 55000.0, "Finance"));
    
    // Копирование
    Queue<Employee> q2 = q1;
    EXPECT_EQ(q2.front().name, "Eve");
    
    // Перемещение
    Queue<Employee> q3 = std::move(q2);
    EXPECT_EQ(q3.front().name, "Eve");
    EXPECT_TRUE(q2.empty());
}

// ==================== ТЕСТЫ ИТЕРАТОРОВ ====================

TEST(QueueIteratorTest, BasicIteration) {
    Queue<int> q;
    for (int i = 1; i <= 5; ++i) {
        q.push(i * 10);
    }
    
    std::vector<int> collected;
    for (auto it = q.begin(); it != q.end(); ++it) {
        collected.push_back(*it);
    }
    
    ASSERT_EQ(collected.size(), 5);
    EXPECT_EQ(collected[0], 10);
    EXPECT_EQ(collected[1], 20);
    EXPECT_EQ(collected[4], 50);
}

TEST(QueueIteratorTest, RangeBasedForLoop) {
    Queue<int> q;
    q.push(100);
    q.push(200);
    q.push(300);
    
    std::vector<int> collected;
    for (int value : q) {  // Использует begin() и end()
        collected.push_back(value);
    }
    
    ASSERT_EQ(collected.size(), 3);
    EXPECT_EQ(collected[0], 100);
    EXPECT_EQ(collected[1], 200);
    EXPECT_EQ(collected[2], 300);
}

TEST(QueueIteratorTest, IteratorOperators) {
    Queue<int> q;
    q.push(1);
    q.push(2);
    
    auto it1 = q.begin();
    auto it2 = q.begin();
    
    // Проверка равенства
    EXPECT_EQ(it1, it2);
    
    // Разыменование
    EXPECT_EQ(*it1, 1);
    
    // Префиксный инкремент
    ++it1;
    EXPECT_EQ(*it1, 2);
    
    // Постфиксный инкремент
    EXPECT_EQ(*(it2++), 1);
    EXPECT_EQ(*it2, 2);
    
    // Стрелочка (если бы был объект с полями)
    Queue<std::string> str_q;
    str_q.push("test");
    auto str_it = str_q.begin();
    EXPECT_EQ(str_it->size(), 4);  // str_it->size() вместо (*str_it).size()
}

TEST(QueueIteratorTest, STLAlgorithmCompatibility) {
    Queue<int> q;
    for (int i = 1; i <= 10; ++i) {
        q.push(i);
    }
    
    // Использование с алгоритмами STL
    int sum = 0;
    std::for_each(q.begin(), q.end(), [&sum](int x) { 
        sum += x; 
    });
    EXPECT_EQ(sum, 55);  // 1+2+...+10 = 55
    
    // Поиск элемента
    auto found = std::find(q.begin(), q.end(), 7);
    EXPECT_NE(found, q.end());
    EXPECT_EQ(*found, 7);
    
    // Подсчет элементов
    size_t count = std::distance(q.begin(), q.end());
    EXPECT_EQ(count, 10);
}

// ==================== ТЕСТЫ С ПОЛИМОРФНЫМИ АЛЛОКАТОРАМИ ====================

TEST(QueueAllocatorTest, DefaultAllocator) {
    // Использование аллокатора по умолчанию
    Queue<int> q1;
    q1.push(42);
    EXPECT_EQ(q1.front(), 42);
    
    // Проверяем, что аллокатор сохраняется
    auto alloc = q1.get_allocator();
    EXPECT_NE(alloc.resource(), nullptr);
}

TEST(QueueAllocatorTest, CustomBlockMemoryResource) {
    // Создаем кастомный аллокатор
    BlockMemoryResource custom_mr;
    
    // Создаем очередь с кастомным аллокатором
    Queue<int> q(&custom_mr);
    
    // Проверяем, что аллокатор установлен правильно
    EXPECT_EQ(q.get_allocator().resource(), &custom_mr);
    
    // Заполняем очередь
    for (int i = 0; i < 50; ++i) {
        q.push(i);
    }
    
    EXPECT_EQ(q.size(), 50);
    
    // Освобождаем память
    q.clear();
    EXPECT_TRUE(q.empty());
}

TEST(QueueAllocatorTest, AllocatorPropagationOnCopy) {
    BlockMemoryResource mr1;
    
    Queue<int> q1(&mr1);
    q1.push(1);
    q1.push(2);
    
    // Копирование должно сохранить аллокатор
    Queue<int> q2 = q1;
    EXPECT_EQ(q2.get_allocator().resource(), &mr1);
    EXPECT_EQ(q2.size(), 2);
}

TEST(QueueAllocatorTest, AllocatorPropagationOnMove) {
    BlockMemoryResource mr1;
    
    Queue<int> q1(&mr1);
    q1.push(3);
    q1.push(4);
    
    // Перемещение должно переместить аллокатор
    Queue<int> q2 = std::move(q1);
    EXPECT_EQ(q2.get_allocator().resource(), &mr1);
    EXPECT_EQ(q2.size(), 2);
    EXPECT_TRUE(q1.empty());
}

// ==================== ТЕСТЫ НА ПРОИЗВОДИТЕЛЬНОСТЬ ====================

TEST(QueuePerformanceTest, LargeNumberOfElements) {
    Queue<int> q;
    const int N = 10000;
    
    // Добавление большого количества элементов
    for (int i = 0; i < N; ++i) {
        q.push(i);
    }
    
    EXPECT_EQ(q.size(), N);
    
    // Извлечение всех элементов
    for (int i = 0; i < N; ++i) {
        EXPECT_EQ(q.front(), i);
        q.pop();
    }
    
    EXPECT_TRUE(q.empty());
}

TEST(QueueMemoryTest, NoMemoryLeaksWithCustomAllocator) {
    // Тест должен проходить без утечек памяти
    BlockMemoryResource mr;
    {
        Queue<int> q(&mr);
        for (int i = 0; i < 1000; ++i) {
            q.push(i);
        }
        // Память очищается в деструкторе q
    }
    // Память очищается в деструкторе mr
}

// ==================== ИНТЕГРАЦИОННЫЕ ТЕСТЫ ====================

TEST(QueueIntegrationTest, ComplexScenario) {
    // Создаем кастомный аллокатор
    BlockMemoryResource custom_mr;
    
    // Этап 1: Создание и заполнение
    Queue<std::string> q(&custom_mr);
    q.push("first");
    q.push("second");
    q.push("third");
    
    EXPECT_EQ(q.size(), 3);
    EXPECT_EQ(q.front(), "first");
    EXPECT_EQ(q.back(), "third");
    
    // Этап 2: Копирование с сохранением аллокатор
    Queue<std::string> q_copy = q;
    EXPECT_EQ(q_copy.size(), 3);
    EXPECT_EQ(q_copy.get_allocator().resource(), &custom_mr);
    
    // Этап 3: Изменение копии
    q_copy.push("fourth");
    EXPECT_EQ(q_copy.back(), "fourth");
    
    // Этап 4: Оригинал не должен измениться
    EXPECT_EQ(q.size(), 3);
    EXPECT_EQ(q.back(), "third");
    
    // Этап 5: Перемещение
    Queue<std::string> q_moved = std::move(q);
    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q_moved.size(), 3);
    
    // Этап 6: Очистка
    q_moved.clear();
    EXPECT_TRUE(q_moved.empty());
}

TEST(QueueIntegrationTest, MixedOperations) {
    Queue<int> q;
    
    // Чередуем push и pop
    q.push(1);
    q.push(2);
    EXPECT_EQ(q.front(), 1);
    
    q.pop();
    EXPECT_EQ(q.front(), 2);
    
    q.push(3);
    q.push(4);
    EXPECT_EQ(q.size(), 3);
    
    q.pop();
    q.pop();
    EXPECT_EQ(q.front(), 4);
    EXPECT_EQ(q.size(), 1);
    
    q.clear();
    EXPECT_TRUE(q.empty());
}

// ==================== ТЕСТЫ НА ГРАНИЧНЫЕ СЛУЧАИ ====================

TEST(QueueEdgeCasesTest, SingleElementQueue) {
    Queue<int> q;
    q.push(42);
    
    EXPECT_FALSE(q.empty());
    EXPECT_EQ(q.size(), 1);
    EXPECT_EQ(q.front(), 42);
    EXPECT_EQ(q.back(), 42);  // front и back одинаковы для одного элемента
    
    q.pop();
    EXPECT_TRUE(q.empty());
}

TEST(QueueEdgeCasesTest, MoveFromEmptyQueue) {
    Queue<int> q1;
    Queue<int> q2 = std::move(q1);
    
    EXPECT_TRUE(q1.empty());
    EXPECT_TRUE(q2.empty());
}

TEST(QueueEdgeCasesTest, CopyEmptyQueue) {
    Queue<int> q1;
    Queue<int> q2 = q1;
    
    EXPECT_TRUE(q1.empty());
    EXPECT_TRUE(q2.empty());
}

TEST(QueueEdgeCasesTest, ClearEmptyQueue) {
    Queue<int> q;
    EXPECT_NO_THROW(q.clear());
    EXPECT_TRUE(q.empty());
}

// ==================== ТЕСТЫ С РАЗНЫМИ ТИПАМИ ДАННЫХ ====================

TEST(QueueDifferentTypesTest, StringQueue) {
    Queue<std::string> q;
    
    q.push("Hello");
    q.push("World");
    q.push("!");
    
    EXPECT_EQ(q.front(), "Hello");
    EXPECT_EQ(q.back(), "!");
    
    q.pop();
    EXPECT_EQ(q.front(), "World");
}

TEST(QueueDifferentTypesTest, PointerQueue) {
    Queue<std::unique_ptr<int>> q;
    
    q.push(std::make_unique<int>(10));
    q.push(std::make_unique<int>(20));
    
    EXPECT_EQ(*q.front(), 10);
    EXPECT_EQ(*q.back(), 20);
    
    // Указатели корректно уничтожаются при pop
    q.pop();
    EXPECT_EQ(*q.front(), 20);
}

TEST(QueueDifferentTypesTest, PairQueue) {
    Queue<std::pair<int, std::string>> q;
    
    q.push({1, "one"});
    q.push({2, "two"});
    
    EXPECT_EQ(q.front().first, 1);
    EXPECT_EQ(q.front().second, "one");
    EXPECT_EQ(q.back().first, 2);
    EXPECT_EQ(q.back().second, "two");
}

// ==================== ТЕСТЫ С ИЗМЕНЕНИЕМ ДАННЫХ ====================

TEST(QueueModificationTest, ModifyElements) {
    Queue<int> q;
    q.push(1);
    q.push(2);
    q.push(3);
    
    // Модификация через front() и back()
    q.front() = 100;
    q.back() = 300;
    
    EXPECT_EQ(q.front(), 100);
    EXPECT_EQ(q.back(), 300);
    
    // Модификация через итераторы
    for (auto it = q.begin(); it != q.end(); ++it) {
        *it *= 2;  // Умножаем все элементы на 2
    }
    
    // Проверяем измененные значения
    EXPECT_EQ(q.front(), 200);
    q.pop();
    EXPECT_EQ(q.front(), 4);  // 2 * 2 = 4
    q.pop();
    EXPECT_EQ(q.front(), 600);  // 300 * 2 = 600
}

TEST(QueueModificationTest, ModifyComplexObjects) {
    Queue<Employee> q;
    q.push(Employee("Alice", 1, 50000.0, "IT"));
    q.push(Employee("Bob", 2, 60000.0, "HR"));
    
    // Модификация через front() и back()
    q.front().name = "Alice Modified";
    q.front().salary = 55000.0;
    
    q.back().department = "Finance";
    q.back().id = 3;
    
    EXPECT_EQ(q.front().name, "Alice Modified");
    EXPECT_EQ(q.front().salary, 55000.0);
    EXPECT_EQ(q.back().department, "Finance");
    EXPECT_EQ(q.back().id, 3);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}