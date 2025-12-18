#include <iostream>
#include <vector>
#include <cassert>
#include <cstddef>

class PoolAllocator {
private:
    struct FreeNode {
        FreeNode* next;
    };

    void* start_ptr = nullptr;
    FreeNode* free_head = nullptr;
    size_t chunk_size;
    size_t block_size;

public:
    // Initialize the pool
    PoolAllocator(size_t blockSize, size_t blockCount) 
        : block_size(blockSize < sizeof(FreeNode*) ? sizeof(FreeNode*) : blockSize),
          chunk_size(blockCount * block_size) 
    {
        // 1. Allocate the big chunk of raw memory
        start_ptr = ::operator new(chunk_size); 
        
        // 2. Initialize the free list
        free_head = static_cast<FreeNode*>(start_ptr);
        
        FreeNode* current = free_head;
        // Iterate through the chunk and link blocks together
        for (size_t i = 0; i < blockCount - 1; ++i) {
            uintptr_t next_addr = reinterpret_cast<uintptr_t>(current) + block_size;
            current->next = reinterpret_cast<FreeNode*>(next_addr);
            current = current->next;
        }
        current->next = nullptr; // Last block points to nothing
    }

    ~PoolAllocator() {
        ::operator delete(start_ptr);
    }

    // Allocate: Pop from head
    void* allocate() {
        if (!free_head) {
            return nullptr; // Pool is empty
        }
        
        FreeNode* block = free_head;
        free_head = free_head->next;
        return block;
    }

    // Deallocate: Push to head
    void deallocate(void* ptr) {
        if (!ptr) return;

        // In a real allocator, you might want to check if ptr is inside start_ptr range
        FreeNode* block = static_cast<FreeNode*>(ptr);
        block->next = free_head;
        free_head = block;
    }
};

// Example Object
struct Particle {
    float x, y, z;
    int life;
};

int main() {
    // Create a pool for 5 Particles
    PoolAllocator pool(sizeof(Particle), 5);

    std::cout << "Pool initialized.\n";

    // Allocate 3 particles
    Particle* p1 = static_cast<Particle*>(pool.allocate());
    Particle* p2 = static_cast<Particle*>(pool.allocate());
    Particle* p3 = static_cast<Particle*>(pool.allocate());

    // Use them
    p1->x = 10.0f;
    std::cout << "Allocated p1 at: " << p1 << " with x=" << p1->x << "\n";
    std::cout << "Allocated p2 at: " << p2 << "\n";

    // Return p2 to the pool
    pool.deallocate(p2);
    std::cout << "Deallocated p2.\n";

    // Re-allocate should get p2's old spot (LIFO behavior)
    Particle* p4 = static_cast<Particle*>(pool.allocate());
    std::cout << "Allocated p4 at: " << p4 << " (Should match p2)\n";

    return 0;
}