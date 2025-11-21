#include <memory_resource>
#include <list>
#include <stdexcept>

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
            for (auto it = free_blocks.begin(); it != free_blocks.end(); ++it){
                
                if (it->size >= bytes) {
                    void* result = it->ptr;
                    allocated_blocks.push_back(*it);
                    free_blocks.erase(it);
                    return result;
                }
            }
            
            void* ptr = ::operator new(bytes);
            allocated_blocks.emplace_back(ptr, bytes);
            return ptr;
        }
        
        void do_deallocate(void* p, size_t bytes, size_t alignment) override {
            for (auto it = allocated_blocks.begin(); it != allocated_blocks.end(); ++it) {
                if (it->ptr == p) {
                    free_blocks.splice(free_blocks.end(), allocated_blocks, it);
                    return;
                }
            }
            throw std::invalid_argument("Attempt to deallocate unknown block");
        }
        
        bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
            return this == &other;
        }

    public:

        BlockMemoryResource() = default;
        
        ~BlockMemoryResource() override {
            for (const auto& block : allocated_blocks) {
                ::operator delete(block.ptr);
            }
            for (const auto& block : free_blocks){
                ::operator delete(block.ptr);
            }
        }
        
        BlockMemoryResource(const BlockMemoryResource&) = delete;

        BlockMemoryResource& operator=(const BlockMemoryResource&) = delete;
};