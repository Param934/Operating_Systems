/*
All the main functions with respect to the MeMS are inplemented here
read the function discription for more details

NOTE: DO NOT CHANGE THE NAME OR SIGNATURE OF FUNCTIONS ALREADY PROVIDED
you are only allowed to implement the functions
you can also make additional helper functions a you wish

REFER DOCUMENTATION FOR MORE DETAILS ON FUNSTIONS AND THEIR FUNCTIONALITY
*/

// add other headers as required
#include<stdio.h>
#include<stdlib.h>
#include <sys/mman.h>

/*
Use this macro where ever you need PAGE_SIZE.
As PAGESIZE can differ system to system we should have flexibility to modify this 
macro to make the output of all system same and conduct a fair evaluation. 
*/
#define PAGE_SIZE 4096

// Define the main chain node structure
struct mainChainNode{
	void* mainAddr; // MeMS virtual address of main node
	struct subChainNode* subChain;
	struct mainChainNode* prev;
	struct mainChainNode* next;
}

// Define the sub-chain node structure
struct subChainNode{
	void* subAddr;  // MeMS virtual address of the sub node
    size_t segmentSize;
    int is_hole;    // 1 if HOLE, 0 if PROCESS
	struct subChainNode* prev;
	struct subChainNode* next;
}

/*
Initializes all the required parameters for the MeMS system. The main parameters to be initialized are:
1. the head of the free list i.e. the pointer that points to the head of the free list
2. the starting MeMS virtual address from which the heap in our MeMS virtual address space will start.
3. any other global variable that you want for the MeMS implementation can be initialized here.
Input Parameter: Nothing
Returns: Nothing
*/
void mems_init(){
	struct mainChainNode* mainChainHead = NULL;
	void* memsHeapStart = NULL;	// Starting MeMS virtual address
	void* addr = NULL;
	int protection = PROT_READ | PROT_WRITE; //readable and writable
	int map_flags = MAP_PRIVATE | MAP_ANONYMOUS;
	int fd= -1;	// Passing an invalid file descriptor
	off_t offset=0;
	
	//Initializing the main chain
	mainChainHead= (struct mainChainNode*)mmap(NULL, sizeof(struct mainChainNode), protection, map_flags, fd, offset);
    if (mainChainHead== MAP_FAILED) {
        perror("failed to initialize mainChainHead");
        exit(1);
    }
	memsHeapStart = mmap(addr, PAGE_SIZE, protection, map_flags, fd, offset);
    if (memsHeapStart == MAP_FAILED) {
        perror("failed to initialize memsHeapStart");
        exit(1);
    }
    mainChainHead->mainAddr = memsHeapStart;
    mainChainHead->subChain = NULL;
    mainChainHead->prev = NULL;
    mainChainHead->next = NULL;
}

/*
This function will be called at the end of the MeMS system and its main job is to unmap the
allocated memory using the munmap system call.
Input Parameter: Nothing
Returns: Nothing
*/
void mems_finish(){
    if (memsHeapStart) {
        if (munmap(memsHeapStart, PAGE_SIZE)) == -1{
			perror("error in unmapping memory of memsHeapStart");
			exit(1);
		}
    }

    // Iterate through the main chain and unmap both main and sub-chain nodes
    struct mainChainNode* currentMain = mainChainhead;
    while (currentMain) {
        struct subChainNode* currentSub = currentMain->subChain;
        while (currentSub) {
            struct subChainNode* temp_sub = currentSub;
            currentSub = currentSub->next;
            munmap(temp_sub, sizeof(struct subChainNode));
        }
        struct mainChainNode* temp_main = currentMain;
        currentMain = currentMain->next;
        munmap(temp_main, sizeof(struct mainChainNode));
    }
}


/*
Allocates memory of the specified size by reusing a segment from the free list if 
a sufficiently large segment is available. 

Else, uses the mmap system call to allocate more memory on the heap and updates 
the free list accordingly.

Note that while mapping using mmap do not forget to reuse the unused space from mapping
by adding it to the free list.
Parameter: The size of the memory the user program wants
Returns: MeMS Virtual address (that is created by MeMS)
*/ 
void* mems_malloc(size_t size){

}


/*
this function print the stats of the MeMS system like
1. How many pages are utilised by using the mems_malloc
2. how much memory is unused i.e. the memory that is in freelist and is not used.
3. It also prints details about each node in the main chain and each segment (PROCESS or HOLE) in the sub-chain.
Parameter: Nothing
Returns: Nothing but should print the necessary information on STDOUT
*/
void mems_print_stats() {
    int total_pages_utilized = 0;
    size_t unused_memory = 0;

    // Iterate through the main chain and sub-chains
    struct MainChainNode* current_main = mainChainHead;
    while (current_main) {
        printf("Main Chain Node - MeMS Virtual Address: %p\n", current_main->main_addr);

        // Sub-chain statistics
        struct SubChainNode* current_sub = current_main->sub_chain;
        while (current_sub) {
            printf("  Sub Chain Node - MeMS Virtual Address: %p, Size: %zu, Type: %s\n", current_sub->sub_addr, current_sub->size, current_sub->is_hole ? "HOLE" : "PROCESS");
            
            if (current_sub->is_hole) unused_memory += current_sub->size;            
            current_sub = current_sub->next;
        }

        total_pages_utilized++;
        current_main = current_main->next;
    }

    printf("Total Pages Utilized: %d\n", total_pages_utilized);
    printf("Unused Memory: %zu bytes\n", unused_memory);
}


/*
Returns the MeMS physical address mapped to ptr ( ptr is MeMS virtual address).
Parameter: MeMS Virtual address (that is created by MeMS)
Returns: MeMS physical address mapped to the passed ptr (MeMS virtual address).
*/
void *mems_get(void*v_ptr){
    
}


/*
this function free up the memory pointed by our virtual_address and add it to the free list
Parameter: MeMS Virtual address (that is created by MeMS) 
Returns: nothing
*/
void mems_free(void *v_ptr){
    
}