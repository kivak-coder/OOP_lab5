#include <memory_resource>
#include <list>
#include <memory>
#include <iterator>
#include <iostream>

class BlockMemoryResource : public std::pmr::memory_resource {
private:
    struct Block {
        void* ptr;
        size_t size;
        
        Block(void* p, size_t s) : ptr(p), size(s) {}
    };
    
    std::list<Block> allocated_blocks;
    std::list<Block> free_blocks;
    
    void* do_allocate(size_t bytes, size_t alignment) override {
        auto it = free_blocks.begin();
        while (it != free_blocks.end()) {
            if (it->size >= bytes) {
                void* result = it->ptr;
                size_t remaining_size = it->size - bytes;
                
                free_blocks.erase(it);
                allocated_blocks.emplace_back(result, bytes);
                return result;
            }
            ++it;
        }
        
        void* ptr = ::operator new(bytes);
        allocated_blocks.emplace_back(ptr, bytes);
        return ptr;
    }
    
    void do_deallocate(void* p, size_t bytes, size_t alignment) override {
        auto it = allocated_blocks.begin();
        while (it != allocated_blocks.end()) {
            if (it->ptr == p) {
                free_blocks.splice(free_blocks.end(), allocated_blocks, it);
                return;
            }
            ++it;
        }
        throw std::invalid_argument("Attempt to deallocate unknown block");
    }
    
    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
        return this == &other;
    }
    
public:
    ~BlockMemoryResource() override {
        for (const auto& block : allocated_blocks) {
            ::operator delete(block.ptr);
        }
        for (const auto& block : free_blocks) {
            ::operator delete(block.ptr);
        }
    }
};

template<typename T>
struct QueueNode {
    T data;
    QueueNode* next;
    
    template<typename... Args>
    QueueNode(Args&&... args) 
        : data(std::forward<Args>(args)...), next(nullptr) {}
};

template<typename T>
class QueueIterator {
private:
    QueueNode<T>* current;
    
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T&;
    
    QueueIterator(QueueNode<T>* node = nullptr) : current(node) {}
    
    reference operator*() const { return current->data; }
    pointer operator->() const { return &current->data; }
    
    QueueIterator& operator++() {
        current = current->next;
        return *this;
    }
    
    QueueIterator operator++(int) {
        QueueIterator temp = *this;
        ++(*this);
        return temp;
    }
    
    bool operator==(const QueueIterator& other) const {
        return current == other.current;
    }
    
    bool operator!=(const QueueIterator& other) const {
        return !(*this == other);
    }
};

template<typename T>
class Queue {
private:
    using allocator_type = std::pmr::polymorphic_allocator<QueueNode<T>>;
    
    QueueNode<T>* head;
    QueueNode<T>* tail;
    allocator_type allocator;
    size_t size_;
    
public:
    using iterator = QueueIterator<T>;
    using const_iterator = QueueIterator<const T>;
    
    explicit Queue(std::pmr::memory_resource* mr = std::pmr::get_default_resource())
        : head(nullptr), tail(nullptr), size_(0), allocator(mr) {}
    
    Queue(const Queue& other) : head(nullptr), tail(nullptr), size_(0), 
                               allocator(other.allocator) {
        for (const auto& item : other) {
            push(item);
        }
    }
    
    Queue(Queue&& other) noexcept 
        : head(other.head), tail(other.tail), size_(other.size_), 
          allocator(std::move(other.allocator)) {
        other.head = nullptr;
        other.tail = nullptr;
        other.size_ = 0;
    }
    
    Queue& operator=(const Queue& other) {
        if (this != &other) {
            clear();
            for (const auto& item : other) {
                push(item);
            }
        }
        return *this;
    }
    
    Queue& operator=(Queue&& other) noexcept {
        if (this != &other) {
            clear();
            head = other.head;
            tail = other.tail;
            size_ = other.size_;
            allocator = std::move(other.allocator);
            
            other.head = nullptr;
            other.tail = nullptr;
            other.size_ = 0;
        }
        return *this;
    }
    
    ~Queue() {
        clear();
    }
    
    template<typename U>
    void push(U&& value) {
        auto* new_node = allocator.allocate(1);
        try {
            allocator.construct(new_node, std::forward<U>(value));
        } catch (...) {
            allocator.deallocate(new_node, 1);
            throw;
        }
        
        new_node->next = nullptr;
        
        if (tail) {
            tail->next = new_node;
        } else {
            head = new_node;
        }
        tail = new_node;
        ++size_;
    }
    
    void pop() {
        if (empty()) {
            throw std::runtime_error("Queue is empty");
        }
        
        QueueNode<T>* temp = head;
        head = head->next;
        
        if (!head) {
            tail = nullptr;
        }
        
        allocator.destroy(temp);
        allocator.deallocate(temp, 1);
        --size_;
    }
    
    T& front() {
        if (empty()) {
            throw std::runtime_error("Queue is empty");
        }
        return head->data;
    }
    
    const T& front() const {
        if (empty()) {
            throw std::runtime_error("Queue is empty");
        }
        return head->data;
    }
    
    T& back() {
        if (empty()) {
            throw std::runtime_error("Queue is empty");
        }
        return tail->data;
    }
    
    const T& back() const {
        if (empty()) {
            throw std::runtime_error("Queue is empty");
        }
        return tail->data;
    }
    
    bool empty() const { return head == nullptr; }
    size_t size() const { return size_; }
    
    void clear() {
        while (!empty()) {
            pop();
        }
    }
    
    iterator begin() { return iterator(head); }
    iterator end() { return iterator(nullptr); }
    const_iterator begin() const { return const_iterator(head); }
    const_iterator end() const { return const_iterator(nullptr); }
    const_iterator cbegin() const { return const_iterator(head); }
    const_iterator cend() const { return const_iterator(nullptr); }
    
    allocator_type get_allocator() const { return allocator; }
};


