// Serhat Sarı 150200068
#include <stdbool.h>

typedef struct header *node_ptr;
typedef struct heapinfo heap;

struct heapinfo{
    unsigned int available_space;
    node_ptr start;
};

struct header{
    unsigned int size;
    unsigned int startAddr;
    bool free;
    node_ptr next;
    node_ptr prev;
};

int InitMyMalloc(int);
void* MyMalloc(int,int);
int MyFree(void *);
void DumpFreeList();
void split_node(unsigned int, node_ptr);
node_ptr first_fit(unsigned int);
node_ptr best_fit(unsigned int);
node_ptr worst_fit(unsigned int);
node_ptr next_fit(unsigned int, node_ptr);
node_ptr coalesce(node_ptr);