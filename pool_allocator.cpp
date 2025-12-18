#include <iostream>
#include <vector>
#include <cassert>
#include <cstddef>
#include <cstdint> 

class PoolAllocator {
private:
    struct FreeNode {
        FreeNode* next;
    };

    void* start_ptr = nullptr;
    FreeNode* free_head = nullptr;
    
    // FIX: block_size MUST be declared before chunk_size
    size_t block_size;
    size_t chunk_size;

public:
    PoolAllocator(size_t blockSize, size_t blockCount) 
        : block_size(blockSize < sizeof(FreeNode*) ? sizeof(FreeNode*) : blockSize),
          chunk_size(blockCount * block_size) 
    {
        start_ptr = ::operator new(chunk_size); 
        free_head = static_cast<FreeNode*>(start_ptr);
        
        FreeNode* current = free_head;
        for (size_t i = 0; i < blockCount - 1; ++i) {
            uintptr_t next_addr = reinterpret_cast<uintptr_t>(current) + block_size;
            current->next = reinterpret_cast<FreeNode*>(next_addr);
            current = current->next;
        }
        current->next = nullptr;
    }

    ~PoolAllocator() {
        ::operator delete(start_ptr);
    }

    void* allocate() {
        if (!free_head) return nullptr;
        FreeNode* block = free_head;
        free_head = free_head->next;
        return block;
    }

    void deallocate(void* ptr) {
        if (!ptr) return;
        FreeNode* block = static_cast<FreeNode*>(ptr);
        block->next = free_head;
        free_head = block;
    }
};

struct Particle {
    float x, y, z;
    int life;
};

int main() {
    PoolAllocator pool(sizeof(Particle), 5);
    std::cout << "Pool initialized.\n";

    Particle* p1 = static_cast<Particle*>(pool.allocate());
    Particle* p2 = static_cast<Particle*>(pool.allocate());

    if (p1) {
        p1->x = 10.0f;
        std::cout << "Allocated p1 at: " << p1 << " with x=" << p1->x << "\n";
    }
    std::cout << "Allocated p2 at: " << p2 << "\n";

    pool.deallocate(p2);
    std::cout << "Deallocated p2.\n";

    Particle* p4 = static_cast<Particle*>(pool.allocate());
    std::cout << "Allocated p4 at: " << p4 << " (Should match p2)\n";

    return 0;
}