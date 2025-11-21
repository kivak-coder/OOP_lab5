#include <iostream>
#include "include/queue.hpp"

void demonstrate_int_queue() {
    std::cout << "=== Demonstration with int ===" << std::endl;
    
    BlockMemoryResource mr;
    Queue<int> queue(&mr);
    
    std::cout << "Pushing elements: 10, 20, 30, 40, 50" << std::endl;
    for (int i = 1; i <= 5; ++i) {
        queue.push(i * 10);
    }
    
    std::cout << "Queue size: " << queue.size() << std::endl;
    std::cout << "Front: " << queue.front() << std::endl;
    std::cout << "Back: " << queue.back() << std::endl;
    
    std::cout << "Queue contents: ";
    for (const auto& item : queue) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
    
    std::cout << "\nPopping two elements..." << std::endl;
    queue.pop();
    queue.pop();
    
    std::cout << "After popping - Size: " << queue.size() << std::endl;
    std::cout << "Front: " << queue.front() << std::endl;
    
    std::cout << "\nPushing new elements: 60, 70" << std::endl;
    queue.push(60);
    queue.push(70);
    
    std::cout << "Final queue: ";
    for (const auto& item : queue) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}


void demonstrate_copy_and_move() {
    std::cout << "\n=== Demonstration of copy and move ===" << std::endl;
    
    BlockMemoryResource mr;
    
    Queue<int> original(&mr);
    original.push(100);
    original.push(200);
    original.push(300);
    
    std::cout << "Original: ";
    for (const auto& item : original) std::cout << item << " ";
    std::cout << std::endl;
    
    Queue<int> copy = original;
    std::cout << "Copy: ";
    for (const auto& item : copy) std::cout << item << " ";
    std::cout << std::endl;
    
    Queue<int> moved = std::move(original);
    std::cout << "After move:" << std::endl;
    std::cout << "Moved: ";
    for (const auto& item : moved) std::cout << item << " ";
    std::cout << std::endl;
    std::cout << "Original size: " << original.size() << std::endl;
    std::cout << "Original empty: " << std::boolalpha << original.empty() << std::endl;
}

int main() {
    try {
        demonstrate_int_queue();
        demonstrate_copy_and_move();
        
        std::cout << "\n=== All demonstrations completed successfully ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}