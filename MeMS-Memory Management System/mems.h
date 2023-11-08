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
#define MAP_ANONYMOUS 0x20

typedef struct header{
	struct mainChainNode* next;
} header;
// Define the main chain node structure
typedef struct mainChainNode{
	void* startingPhysicalAddr;
	unsigned int pageCount;
	struct subChainNode* subChain;
	struct mainChainNode* prev;
	struct 	mainChainNode* next;
	size_t unallocatedMem;
}mainChainNode;
// Define the sub-chain node structure
typedef struct subChainNode{
	void* subAddr;  // MeMS virtual address of the sub node
    size_t segmentSize;
    int is_hole;    // 1 if HOLE, 0 if PROCESS
	struct subChainNode* prev;
	struct subChainNode* next;
}subChainNode;

//initializing global variables
struct header* head;
size_t starting_v_addr;
unsigned int totalPageCount;
/*
Initializes all the required parameters for the MeMS system. The main parameters to be initialized are:
1. the head of the free list i.e. the pointer that points to the head of the free list
2. the starting MeMS virtual address from which the heap in our MeMS virtual address space will start.
3. any other global variable that you want for the MeMS implementation can be initialized here.
Input Parameter: Nothing
Returns: Nothing
*/
void mems_init(){
	head=(struct header*)mmap(NULL, sizeof(struct header), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (head == MAP_FAILED) {
		perror("mmap failed");
		exit(1);
	}
	head->next= NULL;
	starting_v_addr=1000;
	totalPageCount=0;
}

/*
This function will be called at the end of the MeMS system and its main job is to unmap the
allocated memory using the munmap system call.
Input Parameter: Nothing
Returns: Nothing
*/
void mems_finish() {
    // Free the main chain and sub-chains
    struct mainChainNode* currentMain = head->next;

    while (currentMain) {
        struct subChainNode* currentSub = currentMain->subChain;
        while (currentSub) {
            struct subChainNode* tempSub = currentSub;
            currentSub = currentSub->next;
            tempSub->next = NULL;
            tempSub->prev = NULL;
            munmap(tempSub, sizeof(struct subChainNode));
        }

        // Deallocate memory at the starting physical address of the mainChainNode
        munmap(currentMain->startingPhysicalAddr, currentMain->pageCount * PAGE_SIZE);

        struct mainChainNode* tempMain = currentMain;
        currentMain = currentMain->next;
        munmap(tempMain, sizeof(struct mainChainNode));
    }

    // Set the main_chain_head to NULL to indicate that the MeMS system is finished
    head->next = NULL;
}

//Helper function
struct subChainNode* addSubChainNode(struct subChainNode* prevNode, size_t size){
	struct subChainNode* newSubNode=(struct subChainNode*)mmap(NULL, sizeof(struct subChainNode), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (newSubNode == MAP_FAILED) {
		perror("mmap error in addnewSUBNODE");
		exit(1);
	}
	newSubNode->segmentSize=size;
	newSubNode->is_hole=1;
	newSubNode->prev=prevNode;
	newSubNode->next=NULL;
	return newSubNode;
}
//Helper function
struct mainChainNode* addMainChainNode(void* prevMainNode, size_t size){
	size_t mainChainSize=((size+PAGE_SIZE-1)/PAGE_SIZE)*PAGE_SIZE;
	void* memsHeapStart = mmap(NULL,size*sizeof(size_t), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (memsHeapStart == MAP_FAILED) {
		perror("Error while allocating memory using mmap\n");
		exit(1);
	}
	struct mainChainNode* newMainNode=(struct mainChainNode*)mmap(NULL, sizeof(struct mainChainNode), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (newMainNode == MAP_FAILED) {
		perror("mmap");
		exit(1);
	}

	newMainNode->pageCount=(size+PAGE_SIZE-1)/PAGE_SIZE;
	newMainNode->startingPhysicalAddr = memsHeapStart;
	newMainNode->subChain = NULL;
	newMainNode->prev = NULL;
	newMainNode->next = NULL;

	struct subChainNode* newSubNode=addSubChainNode(NULL,size);
	newMainNode->subChain=newSubNode;
	newSubNode->subAddr=(void*)(starting_v_addr+PAGE_SIZE*totalPageCount);
	newSubNode->segmentSize=size;
	newSubNode->is_hole=0;
	if (size<mainChainSize){
		size_t remainingSize=mainChainSize-size;
		struct subChainNode* newSubNode2=addSubChainNode(newSubNode,remainingSize);
		newSubNode->next=newSubNode2;
		newSubNode2->subAddr=(void*)((size_t)(newSubNode->subAddr)+newSubNode->segmentSize);
		newMainNode->unallocatedMem=remainingSize;
	} else newMainNode->unallocatedMem=0;
	totalPageCount+=(size+PAGE_SIZE-1)/PAGE_SIZE;
	return newMainNode;
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
	if (size==0) return NULL;
	if (head->next==NULL){ //When not even single main chain node is initialized
		struct mainChainNode* newMainNode=addMainChainNode(head,size);
		newMainNode->prev=NULL;
		head->next=newMainNode;
		return newMainNode->subChain->subAddr;
	}else{
		// Find a sufficiently large segment in the free list
		for (mainChainNode *currentMain = head->next; currentMain != NULL;currentMain = currentMain->next){
			if (currentMain-> unallocatedMem >= size) {//No need to allocate new memory
				currentMain->unallocatedMem-=size;
				struct subChainNode* currentSub = currentMain->subChain;
				while (currentSub){
					if (currentSub->is_hole) {
						if (currentSub->segmentSize==size){
							currentSub->is_hole=0;
							return currentSub->subAddr;
						}else if (currentSub->segmentSize>size){
							size_t remainingSize=currentSub->segmentSize-size;
							currentSub->segmentSize=size;
							currentSub->is_hole=0;
							struct subChainNode* nextNode=currentSub->next;
							struct subChainNode* newSubNode=addSubChainNode(currentSub,remainingSize);
							currentSub->next=newSubNode;
							if (nextNode!=NULL) nextNode->prev=newSubNode;
							newSubNode->next=nextNode;
							newSubNode->subAddr=(void*)(((size_t)currentSub->subAddr)+currentSub->segmentSize);
							return currentSub->subAddr;
						}
					}
					currentSub=currentSub->next;
				}
			}
		}
		//No free memory available, has to create another node
		mainChainNode* currentMain=head->next;
		while (currentMain->next!=NULL) currentMain=currentMain->next;
		struct mainChainNode* newMainNode=addMainChainNode(currentMain,size);
		newMainNode->prev=currentMain;
		currentMain->next=newMainNode;
		return newMainNode->subChain->subAddr;
	}
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
	printf("Total pages utilized by mems_malloc: %d",totalPageCount);
	printf("\n");
	struct mainChainNode* mainNode=head->next;
	size_t freeMem=0;
	int mainNodeNumber=1;
	while (mainNode){
		freeMem+=mainNode->unallocatedMem;
		printf("MeMS virtual Address of Main Node %d is %lu\n",mainNodeNumber,(size_t) mainNode->subChain->subAddr);
		printf("Starting Physical Address of Main Node %d is %lu\n",mainNodeNumber,(size_t) mainNode->startingPhysicalAddr);
		printf("Number of pages in Main Node %d is %d\n",mainNodeNumber, mainNode->pageCount);
		struct subChainNode* subNode=mainNode->subChain;
		int subNodeNumber=1;
		while (subNode){
			printf("\tMeMS virtual Address of Sub Node %d is %lu\n",subNodeNumber,(size_t) subNode->subAddr);
			printf("\tSegment size of Sub Node %d is %zu\n",subNodeNumber, subNode->segmentSize);
			printf("\tSub Node %d is %s\n", subNodeNumber, (subNode->is_hole ? "HOLE" : "PROCESS"));
			subNode=subNode->next;
			subNodeNumber++;
		}
		mainNode=mainNode->next;
		mainNodeNumber++;
	}
	printf("Unused memory in Free list: %zu\n\n", freeMem);
}

/*
Returns the MeMS physical address mapped to ptr ( ptr is MeMS virtual address).
Parameter: MeMS Virtual address (that is created by MeMS)
Returns: MeMS physical address mapped to the passed ptr (MeMS virtual address).
*/
void* mems_get(void* v_ptr) {
    struct mainChainNode* mainNode = head->next;
    while (mainNode) {
        if (mainNode->subChain && (size_t)mainNode->subChain->subAddr <= (size_t)v_ptr) {
            struct subChainNode* subNode = mainNode->subChain;
            while (subNode) {
                if ((size_t)subNode->subAddr <= (size_t)v_ptr &&
                    ((size_t)v_ptr < ((size_t)subNode->subAddr + subNode->segmentSize))) {
                    // Calculate the offset within the segment
                    size_t offset = (size_t)v_ptr - (size_t)subNode->subAddr;
                    // Calculate the physical address
                    return (void*)((size_t)mainNode->startingPhysicalAddr + offset);
                }
                subNode = subNode->next;
            }
        }
        mainNode = mainNode->next;
    }
    return NULL; // Return NULL for an invalid v_ptr
}


/*
this function free up the memory pointed by our virtual_address and add it to the free list
Parameter: MeMS Virtual address (that is created by MeMS)
Returns: nothing
*/
void mems_free(void* v_ptr) {
    struct mainChainNode* mainNode = head->next;

    // Check if mainNode is NULL (no main chain node)
    if (mainNode == NULL) {
        printf("Error: No main chain nodes available.\n");
        return;
    }

    // Traverse the main chain nodes to find the right one
    while (mainNode->next != NULL && (size_t)v_ptr > (size_t)mainNode->next->subChain->subAddr) {
        mainNode = mainNode->next;
    }

    // Check if the v_ptr is outside the bounds of the main chain nodes
    if ((size_t)v_ptr < (size_t)mainNode->subChain->subAddr) {
        printf("Error: v_ptr not found in the main chain nodes.\n");
        return;
    }

    struct subChainNode* subNode = mainNode->subChain;

    // Traverse the sub-chain nodes to find the right one
    while ((size_t)v_ptr != (size_t)subNode->subAddr) {
        subNode = subNode->next;
    }

    if (subNode->is_hole == 0) {
        subNode->is_hole = 1;
        mainNode->unallocatedMem += subNode->segmentSize;  // Decrease unallocatedMem
    } else {
        printf("Error: v_ptr was already a hole.\n");
        return;
    }

    // Check for adjacent holes and merge them if necessary
    if (subNode->prev != NULL && subNode->prev->is_hole) {
        subNode->prev->segmentSize += subNode->segmentSize;
        subNode->prev->next = subNode->next;
        if (subNode->next != NULL) {
            subNode->next->prev = subNode->prev;
        }
        struct subChainNode* tempSubNode = subNode;
        subNode = subNode->prev;
        // Unmap the memory associated with the merged node
        munmap(tempSubNode, sizeof(struct subChainNode));
    }

    if (subNode->next != NULL && subNode->next->is_hole) {
        subNode->segmentSize += subNode->next->segmentSize;
        subNode->next = subNode->next->next;
        if (subNode->next != NULL) {
            subNode->next->prev = subNode;
        }
        struct subChainNode* tempSubNode = subNode->next;
        // Unmap the memory associated with the merged node
        munmap(tempSubNode, sizeof(struct subChainNode));
    }
}