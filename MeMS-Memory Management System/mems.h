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
#include<stdint.h>
#include<unistd.h>
#include <sys/mman.h>
#include <math.h>

/*
Use this macro where ever you need PAGE_SIZE.
As PAGESIZE can differ system to system we should have flexibility to modify this
macro to make the output of all system same and conduct a fair evaluation.
*/
#define PAGE_SIZE 4096

struct header{
	struct mainChainNode* next;
};
// Define the main chain node structure
struct mainChainNode{
	uint16_t startingPhysicalAddr;
	int pageCount;
	struct subChainNode* subChain;
	struct mainChainNode* prev;
	struct mainChainNode* next;
	size_t unallocatedMem;
};
// Define the sub-chain node structure
struct subChainNode{
	int subAddr;  // MeMS virtual address of the sub node
	// void* subAddress;
    size_t segmentSize;
    int is_hole;    // 1 if HOLE, 0 if PROCESS
	struct subChainNode* prev;
	struct subChainNode* next;
};

//initializing Head
struct header* head;
int starting_v_addr;
int totalPageCount;
/*
Initializes all the required parameters for the MeMS system. The main parameters to be initialized are:
1. the head of the free list i.e. the pointer that points to the head of the free list
2. the starting MeMS virtual address from which the heap in our MeMS virtual address space will start.
3. any other global variable that you want for the MeMS implementation can be initialized here.
Input Parameter: Nothing
Returns: Nothing
*/
void mems_init(){
	struct header* head=(struct header*)mmap(NULL, sizeof(struct header), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (head == MAP_FAILED) {
		perror("mmap");
		exit(1);
	}
	head->next= NULL;
	starting_v_addr=0;
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
            munmap(tempSub, sizeof(struct subChainNode));
        }
        struct mainChainNode* tempMain = currentMain;
        currentMain = currentMain->next;
        munmap(tempMain, sizeof(struct mainChainNode));
    }
    // Set the main_chain_head to NULL to indicate that the MeMS system is finished
    head = NULL;
}


struct subChainNode* addSubChainNode(struct subChainNode* prevNode, size_t size){
	struct subChainNode* subChainNode=(struct subChainNode*)mmap(NULL, sizeof(struct subChainNode), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (subChainNode == MAP_FAILED) {
		perror("mmap");
		exit(1);
	}
	subChainNode->segmentSize=size;
	subChainNode->is_hole=1;
	if (prevNode) {
		struct subChainNode* prevSubNode = (struct subChainNode*)prevNode;
		prevSubNode->next = subChainNode;
	}
	subChainNode->prev=prevNode;
	subChainNode->next=NULL;
	return subChainNode;
}

struct mainChainNode* addMainChainNode(void* prevMainNode, size_t size){
	void* memsHeapStart = mmap(NULL, PAGE_SIZE*ceil(size/PAGE_SIZE), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (memsHeapStart == MAP_FAILED) {
		perror("failed to initialize memsHeapStart");
		exit(1);
	}
	struct mainChainNode* newMainNode=(struct mainChainNode*)mmap(NULL, sizeof(struct mainChainNode), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (newMainNode == MAP_FAILED) {
		perror("mmap");
		exit(1);
	}
	totalPageCount+=ceil(size/PAGE_SIZE);
	newMainNode->startingPhysicalAddr = (uint64_t)memsHeapStart;
	newMainNode->pageCount=ceil(size/PAGE_SIZE);
	newMainNode->subChain = NULL;
	newMainNode->prev = NULL;
	newMainNode->next = NULL;

	struct subChainNode* newSubNode=addSubChainNode(NULL,size);
	newMainNode->subChain=newSubNode;
	newSubNode->subAddr=starting_v_addr+PAGE_SIZE*totalPageCount;
	newSubNode->segmentSize=size;
	newSubNode->is_hole=0;
	if (size<PAGE_SIZE*ceil(size/PAGE_SIZE)){
		size_t remainingSize=size-PAGE_SIZE*ceil(size/PAGE_SIZE);
		struct subChainNode* newSubNode2=addSubChainNode(newSubNode,remainingSize);
		newSubNode2->subAddr=newSubNode->subAddr+newSubNode->segmentSize;
		newMainNode->unallocatedMem=remainingSize;
	} else newMainNode->unallocatedMem=0;
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
	if (head->next){
		// Find a sufficiently large segment in the free list
		struct mainChainNode* currentMain = head->next;
		while (currentMain) {
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
							newSubNode->prev=currentSub;
							nextNode->prev=newSubNode;
							newSubNode->next=nextNode;
							newSubNode->subAddr=currentSub->subAddr+currentSub->segmentSize;
							return currentSub->subAddr;
						}
					}
					currentSub=currentSub->next;
				}
			}
			currentMain = currentMain->next;
		}
		//No free memory available, has to create another node
		struct mainChainNode* newMainNode=addMainChainNode(currentMain,size);
		newMainNode->prev=currentMain;
		currentMain->next=newMainNode;
		return newMainNode->subChain->subAddr;
	}
	else{ //When not even single main chain node is initialized
		struct mainChainNode* newMainNode=addMainChainNode(head,size);
		newMainNode->prev=NULL;
		head->next=newMainNode;
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
		printf("MeMS virtual Address of Main Node %d is %d",mainNodeNumber, mainNode->subChain->subAddr);
		printf("Starting Physical Address of Main Node %d is %lu",mainNodeNumber, mainNode->startingPhysicalAddr);
		printf("Number of pages in Main Node %d is %d",mainNodeNumber, mainNode->pageCount);
		struct subChainNode* subNode=mainNode->subChain;
		int subNodeNumber=1;
		while (subNode){
			printf("\tMeMS virtual Address of Sub Node %d is %d",subNodeNumber, subNode->subAddr);
			printf("\tSegment size of Sub Node %d is %zu",subNodeNumber, subNode->segmentSize);
			printf("\tSub Node %d is %s", subNodeNumber, (subNode->is_hole ? "HOLE" : "PROCESS"));
			subNode=subNode->next;
			subNodeNumber++;
		}
		mainNode=mainNode->next;
		mainNodeNumber++;
	}
	printf("\n");
	printf("Unused memory in Free list: %zu", freeMem);
}

/*
Returns the MeMS physical address mapped to ptr ( ptr is MeMS virtual address).
Parameter: MeMS Virtual address (that is created by MeMS)
Returns: MeMS physical address mapped to the passed ptr (MeMS virtual address).
*/
void* mems_get(void* v_ptr) {
	int offset= (int)v_ptr%PAGE_SIZE;
	int page= (int)v_ptr/PAGE_SIZE;
	int pageCount=0;
	struct mainChainNode* mainNode=head->next;
	while(pageCount+mainNode->pageCount<=page){
		pageCount+=mainNode->pageCount;
		mainNode=mainNode->next;
	}
	int difference=(int)v_ptr-pageCount*PAGE_SIZE;
	return mainNode->startingPhysicalAddr+difference;
}

/*
this function free up the memory pointed by our virtual_address and add it to the free list
Parameter: MeMS Virtual address (that is created by MeMS)
Returns: nothing
*/
void mems_free(void* v_ptr) {
	struct mainChainNode* mainNode=head->next;
	while ((int)v_ptr<=mainNode->next->subChain->subAddr) mainNode=mainNode->next;
	struct subChainNode* subNode=mainNode->subChain;
	while ((int)v_ptr!=subNode->subAddr) subNode=subNode->next;
	if ((int)v_ptr!=mainNode->subChain->subAddr) printf("ptr subChainNode not found");
	if (subNode->is_hole==0) subNode->is_hole=1;
	else (printf("ptr was already hole"));
	//checking for adjacent holes
	if (subNode->prev->is_hole){
		subNode->prev->segmentSize=subNode->prev->segmentSize+subNode->segmentSize;
		subNode->prev->next=subNode->next;
		subNode->next->prev=subNode->prev;
		subNode=subNode->prev;
	}
	if (subNode->next->is_hole){
		subNode->segmentSize=subNode->next->segmentSize+subNode->segmentSize;
		subNode->next=subNode->next->next;
		subNode->next->next->prev=subNode;
	}
}
