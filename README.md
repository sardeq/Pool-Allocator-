# The Core Concept: The Free List
- The most efficient way to manage a memory pool is using an embedded free list.
- The Chunk: You allocate one large block of raw memory from the OS (using malloc or mmap).
- The Nodes: You divide this chunk into small, fixed-size units.
- The Trick: When a block is not in use, you use the memory inside that block to store a pointer to the next free block. 
- This costs zero overhead because you reuse the empty memory itself.This makes allocation and deallocation extremely fast (O(1) complexity) because you are simply pushing and popping from a linked list.

# The "Union" Trick
- To store data when allocated, but a pointer when free, we can conceptually treat the block as a *union*.
![alt text](image.png)