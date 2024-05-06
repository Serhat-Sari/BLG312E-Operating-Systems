// Serhat Sarı 150200068
#define _GNU_SOURCE

#include "hw.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/mman.h>

node_ptr head = NULL;
node_ptr last;
heap fixed_heap = {0};

int InitMyMalloc(int HeapSize){
    head = mmap(NULL, HeapSize, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (head == MAP_FAILED || HeapSize <= 0) { // If heapsize is negative or mmap failed, we return -1.
        perror("mmap failed");
        return -1;
    }
    head->size = HeapSize - sizeof(head);  // set the size
    head->free = true; // it is free to use.
    head->next = NULL;
    head->prev = NULL;
    head->startAddr = 0;
    fixed_heap.available_space = head->size;
    fixed_heap.start = head;

    return 0;
}

void split_node(unsigned int size, node_ptr chunk){
    // New block is the free chunk, chunk is the block of memory that will be used.
    if(chunk->size < size + sizeof(node_ptr) + sizeof(node_ptr)){
        return;
    }
    node_ptr new_block;
    new_block = ((char*)chunk) + size; // how does this work? i mean it does work so idc
    new_block->size = chunk->size - size - sizeof(node_ptr);
    new_block->next = chunk->next;
    new_block->prev = chunk; // prev pointer of free chunk is the chunk that will be used.
    new_block->free = true; // new block is free to use.
    chunk->next = new_block;
    chunk->size = size;
    new_block->startAddr = chunk->startAddr + chunk->size; // Calculate the starting address of this free chunk.
    if(new_block->next){ // Set the previous pointer of the next chunk to new block.
        new_block->next->prev = new_block;
    }
}

void* MyMalloc(int size, int strategy){
    node_ptr chunk;
    if(fixed_heap.available_space < size + sizeof(node_ptr)){ // If we dont have enough memory in the heap.
        printf("Memory allocation failed!\n");
        printf("Need more memory!\n");
        return NULL;
    }
    switch(strategy){ // Select the strategy according to selection of the user.
        case 0:
            chunk = best_fit(size);
            last = chunk;
            printf("Mem. Allocation Strategy is: Best Fit.\n");
            break;
        case 1:
            chunk = worst_fit(size);
            last = chunk;
            printf("Mem. Allocation Strategy is: Worst Fit.\n");
            break;
        case 2:
            chunk = first_fit(size);
            last = chunk;
            printf("Mem. Allocation Strategy is: First Fit.\n");
            break;
        case 3:
            chunk = next_fit(size,last);
            printf("Mem. Allocation Strategy is: Next Fit.\n");
            last = chunk;
            break;
    }

    if(chunk){ // It means we found a proper memory block to use with enough size.
        if(chunk->size > size){ // If chunk's size is bigger than asked size we need to split it.
            split_node(size,chunk);
        }
        chunk->free = false;
        fixed_heap.available_space -= chunk->size + sizeof(node_ptr);
    }
    else{ // It means finding method returned NULL pointer,
        // It means we passed the if check at line 50 but there exists no chunk with enough memory in it.
        printf("Memory allocation failed!\n");
        printf("There exists enough memory but in need of coalescing!\n");
        return NULL;
    }
    printf("Memory allocation succesful.\n");
    return ((char*)chunk);
}

node_ptr first_fit(unsigned int size){
    node_ptr temp = head;
    // Search until we reached null or we found the first free block with enough size
    while(temp != NULL && !(temp->free && temp->size >= size + sizeof(node_ptr))){
        temp = temp->next;
    }
    return temp;
}

node_ptr next_fit(unsigned int size, node_ptr last){
    node_ptr temp = last;
    // If last pointer was null before we set it to beginning.
    if(last == NULL)
        last = head;
    // Same logic as first fit but we dont start to search from beginning but from where is the last pointer at.
    while(temp != NULL && !(temp->free && temp->size >= size + sizeof(node_ptr))){
        temp = temp->next;
    }
    return temp;
}

node_ptr best_fit(unsigned int size){
    node_ptr temp = head;
    node_ptr smallest_bigger = NULL;
    // In this while loop we find the first free block with enough size.
    while(temp != NULL){
        if(temp->free && temp->size >= size){
            smallest_bigger = temp;
            break;
        }
        temp = temp->next;
    }
    // In this loop we find the smallest free block that is bigger equal than the asked size.
    while(temp != NULL){
        if(temp->free && temp->size >= size && smallest_bigger->size > temp->size){
            smallest_bigger = temp;
        }
        temp = temp->next;
    }
    return smallest_bigger;
}

node_ptr worst_fit(unsigned int size){
    node_ptr temp = head;
    node_ptr biggest = NULL;
    // In this while loop we find the first free block with enough size.
    while(temp != NULL){
        if(temp->free && temp->size >= size){
            biggest = temp;
            break;
        }
        temp = temp->next;
    }
    // In this while loop we find the biggest block that is free.
    while(temp != NULL){
        if(temp->free && temp->size >= size && biggest->size < temp->size){
            biggest = temp;
        }
        temp = temp->next;
    }
    return biggest;
}

int MyFree(void *ptr){
    if(ptr == NULL) return 0;
    node_ptr chunk = (node_ptr)(ptr); // Find the pointer to the block that we want to release
    chunk->free = true;
    fixed_heap.available_space += chunk->size + sizeof(node_ptr);
    if(chunk->prev && chunk->prev->free){ // If previous block exists and it is also free
        chunk = coalesce(chunk->prev); // We want to coalesce
    }
    if(chunk->next){ // If next block exists we will try to coalesce.
        chunk = coalesce(chunk);
    }
    else{ // This else clause means we are trying to free the block with no next block or free previous block
        if(chunk->prev) chunk->prev->next = NULL; // If previous block exists, set its next to null
        else head = NULL; // Otherwise we are trying to free the only block in the memory. Set the head to NULL.
    }
    return 0;
}

node_ptr coalesce(node_ptr chunk){
    if(chunk->next && chunk->next->free){ // If next element exists and it is free.
        chunk->size += chunk->next->size; // Increase the size of the chunk.
        chunk->next = chunk->next->next; // Set its next to next of the next chunk.
    }
    if(chunk->next){ // If next exists but it is not free
        chunk->next->prev = chunk; // Handle its previous pointer.
    }
    return chunk;
}

void DumpFreeList(){
    node_ptr temp = head;
    printf("-------------------------------\n");
    // Traverse the linked list.
    while(temp != NULL){
        printf("Addr:%d Size:%d ",temp->startAddr, temp->size);
        if(temp->free == 1) printf("Empty\n");
        else printf("Full\n");
        temp = temp->next;
    }
    printf("-------------------------------\n");
    return;
}

int askStrategy(){
    printf("Please enter the memory allocation strategy you want.\n");
    printf("0 = Best Fit | 1 = Worst Fit | 2 = First Fit | 3 = Next Fit\n");
    int strategy;
    scanf("%d",&strategy);
    return strategy;
}

int main(){
    int pageSize = getpagesize();
    printf("Page size of this system is: %d\n", pageSize);
    printf("Please enter an appropriate number to multiple it with page size...\n");
    int multiple = 1;
    scanf("%d",&multiple);
    if(multiple >=1 ){
        if(InitMyMalloc(pageSize * multiple) == 1)
            printf("Failed to initialize heap!\n");
    }
    else{
        if(InitMyMalloc(pageSize) == 1)
            printf("Failed to initialize heap!\n");
    }

    int strategy;
    strategy = askStrategy();
    printf("Trying to allocate 32 byte.\n");
    void *a = MyMalloc(32,strategy);
    DumpFreeList();
    printf("Amount of empty space: %d\n",fixed_heap.available_space);

    strategy = askStrategy();
    printf("Trying to allocate 64 byte.\n");
    void *b = MyMalloc(64,strategy);
    DumpFreeList();
    printf("Amount of empty space: %d\n",fixed_heap.available_space);

    strategy = askStrategy();
    printf("Trying to allocate 128 byte.\n");
    void *c = MyMalloc(128,strategy);
    DumpFreeList();
    printf("Amount of empty space: %d\n",fixed_heap.available_space);

    strategy = askStrategy();
    printf("Trying to allocate 256 byte.\n");
    void *d = MyMalloc(256,strategy);
    DumpFreeList();
    printf("Amount of empty space: %d\n",fixed_heap.available_space);

    printf("Trying to release 64 byte.\n");
    MyFree(b);
    printf("Amount of empty space: %d\n",fixed_heap.available_space);
    DumpFreeList();

    printf("Trying to release 128 byte.\n");
    MyFree(c);
    printf("Amount of empty space: %d\n",fixed_heap.available_space);
    DumpFreeList();
    
    strategy = askStrategy();
    printf("Trying to allocate 180 byte.\n");
    void *e = MyMalloc(180,strategy);
    DumpFreeList();
    return 0;
}