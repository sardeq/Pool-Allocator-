#include <iostream>
#include <cstddef>
#include <cstdint>
#include <utility>

template <typename T>
class ObjectPool {
private:
    struct FreeNode {
        FreeNode* next;
    };

    void* start_ptr = nullptr;
    FreeNode* free_head = nullptr;
    size_t chunk_size;
    size_t block_size;

public:
    ObjectPool(size_t blockCount) {
        block_size = sizeof(T) > sizeof(FreeNode*) ? sizeof(T) : sizeof(FreeNode*);
        chunk_size = blockCount * block_size;

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

    ~ObjectPool() {
        ::operator delete(start_ptr);
    }

    template <typename... Args>
    T* create(Args&&... args) {
        if (!free_head) return nullptr;

        FreeNode* block = free_head;
        free_head = free_head->next;

        return new(block) T(std::forward<Args>(args)...);
    }

    void destroy(T* ptr) {
        if (!ptr) return;

        ptr->~T();

        FreeNode* block = reinterpret_cast<FreeNode*>(ptr);
        block->next = free_head;
        free_head = block;
    }
};

struct Particle {
    float x, y, z;
    int id;

    Particle(float _x, float _y, float _z, int _id)
        : x(_x), y(_y), z(_z), id(_id) {
        std::cout << "  > Particle " << id << " Constructed\n";
    }

    ~Particle() {
        std::cout << "  > Particle " << id << " Destroyed\n";
    }
};

int main() {
    ObjectPool<Particle> pool(3);
    std::cout << "Pool initialized.\n";

    std::cout << "Creating p1...\n";
    Particle* p1 = pool.create(10.0f, 20.0f, 30.0f, 1);

    std::cout << "Creating p2...\n";
    Particle* p2 = pool.create(5.5f, 5.5f, 5.5f, 2);

    std::cout << "Destroying p1...\n";
    pool.destroy(p1);

    std::cout << "Creating p3...\n";
    Particle* p3 = pool.create(100.0f, 100.0f, 100.0f, 3);

    std::cout << "Address of p1 (old): " << p1 << "\n";
    std::cout << "Address of p3 (new): " << p3 << "\n";

    return 0;
}
