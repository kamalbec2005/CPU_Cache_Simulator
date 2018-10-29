#include "first.h"

#ifdef SET_ASSOC
SetAssocDetails stSetAssocDetails;
#endif

inputs stInputs;

long lCacheSize; /* Total Cache sie */
long lAssoc; /*Not clear*/
int iCachePolicy; /* 0 - Normal, 1 - LRU */

/*Start: Related to association */
enum CacheAssoc eCacheAssoc; /* 0 - Direct, 1 - Assoc */
int iAssocSet = 0; /*Associtivity set */
struct AssocEntry *astAssocEntry;
struct AssocEntry *astAssocEntryPF;

int iBlockBitNum = 0;
int iCacheBitNum = 0;
int iBlockIndexBitNum = 0;
int iTagBitNum = 0;
int iPrefetchEnable = FALSE;
/*End: Related to association */


long lBlockSize; /* */
char *pcTraceFile; /* File to parse */

long lTotalBlockCount = 0;
long lCurBlockCount = 0;
long lCurBlockCountPF = 0;

/*Output parameters to display */
unsigned long ulHitCount = 0;
unsigned long ulMissCount = 0;
unsigned long ulReadCount = 0;
unsigned long ulWriteCount = 0;

unsigned long ulHitCountPF = 0;
unsigned long ulMissCountPF = 0;
unsigned long ulReadCountPF = 0;
unsigned long ulWriteCountPF = 0;



int main(int argc, char **argv)
{

	FILE *fp; 
	int iRet;
	char line[1024], *acTemp=NULL;
	long size = 0;

	lCacheSize = atol(argv[1]);
	//iCachePolicy = atol(argv[3]);
	lBlockSize = atol(argv[4]);
	pcTraceFile = argv[5];

	/* Input validation */
	if ( 
             (argc != 6) /* Number of arguments */
           ||(0 >= lCacheSize) /* Cache size should be positive */
	   ||(0 >= lBlockSize) /* Block size should be positive */
	   ||(pcTraceFile == NULL) /*File name should not be NULL */
	   )
	{
		//printf("Exit: Reason: Invalid arguments\n");
		exit(-1); 
	}
	else
	{
		lTotalBlockCount = lCacheSize/lBlockSize;
	}

	if (strstr(argv[3], "lru"))
		iCachePolicy = LRU;
	else if (strstr(argv[3], "fifo"))
		iCachePolicy = FIFO;
	else
	{
		exit(-1);
	}
	
	/*Validate cache size: Value is should power of 2 */
	if (-1 == (iCacheBitNum = checkNumPowerOfTwo(lCacheSize)))
	{	
		printf("Cache size is not power of 2\n");
		exit(-1);
	}
	
	if (-1 == (iBlockBitNum = checkNumPowerOfTwo(lBlockSize)))
	{	
		printf("Block size is not power of 2\n");
		exit(-1);
	}

	iBlockIndexBitNum = iCacheBitNum-iBlockBitNum;
	iTagBitNum = MAX_ADDRESS_SIZE - (iBlockBitNum+iBlockIndexBitNum);

	//printf("iCacheBitNum = %d\n", iCacheBitNum);
	//printf("iBlockBitNum = %d\n", iBlockBitNum);
	//printf("Tag bit Num = %d, Block Index bit num = %d, Block bits = %d\n", iTagBitNum, iBlockIndexBitNum, iBlockBitNum);

	/* Check the block size and cache size */
	if ((lCacheSize%lBlockSize != 0) ||(lCacheSize < lBlockSize))
	{
		printf("Cache sise is not devisible by block size: Cache size = %lu, block size = %lu\n", lCacheSize, lBlockSize);
		exit(-1);
	}

	/* Check the argument argv[2] */
	if (strstr(argv[2], "assoc"))
	{
		
		//eCacheAssoc = ASSOC;
		if ((acTemp = strchr(argv[2], ':')))
		{
			stSetAssocDetails.ulSetValue = atoi(++acTemp); /* Ex: 2,4,8*/
		}
		else
		{
                        stSetAssocDetails.ulSetValue = lTotalBlockCount; /* Ex: 2,4,8*/
                }

	        if (stSetAssocDetails.ulSetValue == 0)
                	exit(-3);

                stSetAssocDetails.ulTotalSets = lTotalBlockCount/stSetAssocDetails.ulSetValue; /*Ex: 32/2 = 16sets */
                stSetAssocDetails.pstSetEntry =  setassoc_alloc_set_tbl();
                stSetAssocDetails.pstSetEntryPF =  setassoc_alloc_set_tbl();
                stSetAssocDetails.ulSetBitNum = checkNumPowerOfTwo(stSetAssocDetails.ulSetValue);
		if (stSetAssocDetails.ulSetValue > lTotalBlockCount)
		{
			exit(-3);
		}
                //printf("Initiated: SET ASSOC------------------------\n"); 
                eCacheAssoc = ASSOC_SET;
	
	}
	else if (strstr(argv[2], "direct"))
	{
#if 0
			stSetAssocDetails.ulSetValue = 1;;

                        stSetAssocDetails.ulTotalSets = lTotalBlockCount/stSetAssocDetails.ulSetValue; /*Ex: 32/2 = 16sets */
                        stSetAssocDetails.pstSetEntry =  setassoc_alloc_set_tbl();
                        stSetAssocDetails.pstSetEntryPF =  setassoc_alloc_set_tbl();
                        stSetAssocDetails.ulSetBitNum = checkNumPowerOfTwo(stSetAssocDetails.ulSetValue);
                        //printf("Initiated: SET ASSOC------------------------\n"); 
                        eCacheAssoc = ASSOC_SET;
#endif
		eCacheAssoc = DIRECT;

                size = lTotalBlockCount*sizeof(struct AssocEntry);
                //printf("Cache Associvitivity: %s, assoc value  = %d, size of array = %lu\n", argv[2], iAssocSet, size);
                /* Alloc entries for assoc */
                astAssocEntry = (struct AssocEntry *) malloc(size);
	        astAssocEntryPF = (struct AssocEntry *) malloc(size);
                memset(astAssocEntry, 0, size);
                memset(astAssocEntryPF, 0, size);
		//printf("Cache Associvitivity: Direct\n");
	}
	else
	{
		printf("Assoc argument is worng: Exiting !!!\n");
		exit(-1);
	} 
	
	//printf("Cache size = %lu, block size = %lu, Total bnumber of blocks = %lu\n", lCacheSize, lBlockSize, lTotalBlockCount);

	/*1. Open trace file */
	fp = fopen(pcTraceFile, "r");
	if (fp == NULL)
	{
		perror("open file");
		//printf("Exit: Reason: Invalid file name\n");
		exit(-2);
	}

	/*2. Get the details from read line */
	while(1)
	{
		/*Read  line from file */
		if (fgets(line, 1024, fp))
		{
			//printf("\nRead line = %s", line);
		}
		else
		{
			//printf("End of the file reached\n");
			break;
		}
		//printf("Read line from File = %s\n", line);
		iRet = parse_line(line, &stInputs);
		if (0 > iRet)
		{
			//printf("parse_line: Issue\n");
			exit(-3);
		}
		else if (iRet == 1)
		{
			break;
		}

		if (WRITE == stInputs.mode)
		{
			ulWriteCount++;
			ulWriteCountPF++;
		}	
		/*3. Check hit or miss
		if iRet is 1: It is hit
		else it is miss */
		iPrefetchEnable = FALSE;
		if (check_cache((stInputs.addr)-(stInputs.addr)%lBlockSize, 0))
		{
			//NoPF printf("Cache HIT ##########i Addr = 0x%x\n", (unsigned int)stInputs.addr);
			ulHitCount++;
		}
		else
		{
			//NoPF printf("Cache MISS ##########i Addr = 0x%x\n", (unsigned int)stInputs.addr);
			cache_block((stInputs.addr)-(stInputs.addr)%lBlockSize);
			ulMissCount++;
			ulReadCount++;

		}
	
		/*With Prefetch */
		iPrefetchEnable = TRUE;
		if (check_cache((stInputs.addr)-(stInputs.addr)%lBlockSize, 1))
		{
			//NoPF printf("Cache HIT ##########i Addr = 0x%x\n", (unsigned int)stInputs.addr);
			ulHitCountPF++;
		}
		else
		{
			//NoPF printf("Cache MISS ##########i Addr = 0x%x\n", (unsigned int)stInputs.addr);
			cache_block((stInputs.addr)-(stInputs.addr)%lBlockSize);
			ulMissCountPF++;
			ulReadCountPF++;
			ulReadCountPF++;

		}
		
	}


	printf("no-prefetch\n");
	printf("Memory reads: %lu\n", ulReadCount);
	printf("Memory writes: %lu\n", ulWriteCount);
	printf("Cache hits: %lu\n", ulHitCount);
	printf("Cache misses: %lu\n", ulMissCount++);

	printf("with-prefetch\n");
	printf("Memory reads: %lu\n", ulReadCountPF);
	printf("Memory writes: %lu\n", ulWriteCountPF);
	printf("Cache hits: %lu\n", ulHitCountPF);
	printf("Cache misses: %lu\n", ulMissCountPF);
	

	return 0;
}/*End of main*/


int check_cache(unsigned long inPtr, int isPrefetch)
{
	if (eCacheAssoc == DIRECT)
	{
		return (check_cache_direct(inPtr, isPrefetch));
	}
	else if ((eCacheAssoc == ASSOC) &&(iAssocSet == 0))
	{		
		return (check_cache_assoc(inPtr, isPrefetch));
	}
	else if ((eCacheAssoc == ASSOC_SET))
	{		
		return (check_cache_assoc_set(inPtr, isPrefetch));
	}
	else
	{
		printf("Associvity is wrong\n");
		return 0;
	}
}

unsigned long get_block_index(unsigned long addr)
{
	unsigned long temp = addr;
	temp = temp<<(sizeof(long)*8-iCacheBitNum);
	temp = temp>> (sizeof(long)*8-iBlockIndexBitNum);
	//printf("Block Inedx num = %lx of Addr = %lx\n", temp, addr);
	return temp;
}


int check_cache_direct(unsigned long inPtr, int isPrefetch)
{
	unsigned long lBlockInd = get_block_index(inPtr);
	unsigned long lTagId = (inPtr)>>iCacheBitNum;
	struct AssocEntry stAssocEntry;
	if (TRUE == iPrefetchEnable)
	{
		stAssocEntry.valid = astAssocEntryPF[lBlockInd].valid;
		stAssocEntry.addr = astAssocEntryPF[lBlockInd].addr;
	}
	else
	{
		stAssocEntry.valid = astAssocEntry[lBlockInd].valid;
		stAssocEntry.addr = astAssocEntry[lBlockInd].addr;
	}

	if (iPrefetchEnable)
	{
		//printf("checking prefetched: BlockIndex = %lx, lTagId = %lx, Ptr = %lx \n", lBlockInd, lTagId, inPtr);
		//printf("checking prefetched: stAssocEntry.addr = %lx, stAssocEntry.valid = %d\n", stAssocEntry.addr,stAssocEntry.valid);
	}
	if (stAssocEntry.valid == VALID)
	{
		if (stAssocEntry.addr == lTagId)
		{	
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}
}


#ifdef SET_ASSOC
int check_cache_assoc_set(unsigned long inPtr, int isPrefetch)
{
	return setassoc_lookup(inPtr, isPrefetch);
}
#else 
int check_cache_assoc_set(unsigned long inPtr, int isPrefetch)
{
	return 0;
}
#endif
/* Need to optimise more to handle invalid strings */
int parse_line(char *line, inputs *pstInputs)
{
	char *ptr;

	/* Check for end of file */
	if (strstr(line, "#eof"))
	{
		//printf("parse_line: Reached end of the file\n");
		return 1;
	}

	/* Parse program counter */
	ptr = (void *)strtok(line, ":");
        if (ptr == NULL)
        {
                //printf("Error: parse_line: Program counter field is NULL\n");
                return -1;
        }
	else
	{
		pstInputs->addr = strtoul(ptr, NULL, 16);
	}

	/* Parse mode */
	ptr = strtok(NULL, " ");
        if (ptr == NULL)
        {
                //printf("Error: parse_line: read or write field is NULL\n");
                return -1;
	}
	else
	{
		if (strstr(ptr,"R"))
			pstInputs->mode = READ;
		else if(strstr(ptr, "W"))
			pstInputs->mode = WRITE;
		else
		{
                	//printf("Error: parse_line: read or write field is not W nor R\n");
                	return -1;
		}
			
	}	

	/* Parse address */
	ptr = strtok(NULL, "\n");
        if (ptr == NULL )
	{
		printf("Error: parse_line: Program cpunter =  %lu, mode = %d\n", pstInputs->addr, pstInputs->mode);
		return -1;
	}
	//printf("Parsed address from line = %s\n",ptr);
	pstInputs->addr = (MAX_ADDRESS)&(strtoul(ptr, NULL, 16));

	//printf("Converted to ul Addr =  %lx, mode = %d\n", pstInputs->addr, pstInputs->mode);
	return 0;
}


struct list *head = NULL; /*Header for list without prefetch */
struct list *headPF = NULL; /*Header for list with prefetch */

/* Remove tail */
void remove_tail(int isPrefetch)
{
	//printf("remove_tail: Going to delete tail\n");
	struct list *prev;
	struct list *temp;
	if (1 == isPrefetch)
	{
		temp = headPF;
	}
	else
	{
		temp = head;
	}

	if (temp->nxt == NULL)
	{
		free(temp);
		if (isPrefetch)
			lCurBlockCountPF--;
		else
			lCurBlockCount--;

		return;
	}
	
	prev = temp->nxt;

	while(temp->nxt)
	{
		prev = temp;
		temp = temp->nxt;
	}

	prev->nxt = NULL;
	free(temp);

	if (isPrefetch)
		lCurBlockCountPF--;
	else
		lCurBlockCount--;

	return;
}

void cache_block(unsigned long inPtr)
{
	if (eCacheAssoc == DIRECT)
	{
		cache_block_direct(inPtr);
	}
	else if ((eCacheAssoc == ASSOC) &&(iAssocSet == 0))
        {
		if (iPrefetchEnable == FALSE)
	        	cache_block_assoc(inPtr);
	        else
			cache_block_assoc_pf(inPtr);

	}
	else if ((eCacheAssoc == ASSOC_SET))
	{
		cache_block_assoc_set(inPtr);
	}
	else
	{
	        printf("Associvity is wrong\n");
	}
}

#ifndef SET_ASSOC
void cache_block_assoc_set(unsigned long inPtr)
{
	return;
}
#else
void cache_block_assoc_set(unsigned long inPtr)
{
	setassoc_cache_block(inPtr);	
}
#endif

void cache_block_direct(unsigned long inPtr)
{
	unsigned long lBlockInd = get_block_index(inPtr);
	unsigned long lTagId = (inPtr)>>iCacheBitNum;
	
	
	if ((iPrefetchEnable == FALSE))
	{
		astAssocEntry[lBlockInd].addr = lTagId;
		astAssocEntry[lBlockInd].valid = VALID;
		//printf("Caching ++++ : BlockIndex = %lx, lTagId = %lx, Ptr = %lx \n", lBlockInd, lTagId, inPtr);
	}
	else
	{
		astAssocEntryPF[lBlockInd].addr = lTagId;
		astAssocEntryPF[lBlockInd].valid = VALID;
		//printf("Prefetch: Caching ++++ : BlockIndex = %lx, lTagId = %lx, Ptr = %lx \n", lBlockInd, lTagId, inPtr);
		//printf("Prefetch: Caching ++++ : BlockIndex = %lu\n", lBlockInd);
		
		//if (lBlockInd < (lTotalBlockCount-1) && !((astAssocEntryPF[lBlockInd+1].addr == lTagId) && (VALID == astAssocEntryPF[lBlockInd+1].valid)))// && (VALID != astAssocEntryPF[lBlockInd+1].valid))
		if (!(lBlockInd < (lTotalBlockCount-1)) && !((astAssocEntryPF[0].addr == lTagId) && (VALID == astAssocEntryPF[0].valid)))
		{
			//printf("Coming back to zero: Block id = %lx \n",lBlockInd);
			astAssocEntryPF[0].addr = lTagId;
			astAssocEntryPF[0].valid = VALID;
		}
		else if ((lBlockInd < (lTotalBlockCount-1)) && !((astAssocEntryPF[lBlockInd+1].addr == lTagId) && (VALID == astAssocEntryPF[lBlockInd+1].valid)))// && (VALID != astAssocEntryPF[lBlockInd+1].valid))
		{
			astAssocEntryPF[lBlockInd+1].addr = lTagId;
			astAssocEntryPF[lBlockInd+1].valid = VALID;
			//printf("Prefetched: astAssocEntryPF[lBlockInd+1].addr = %lx, astAssocEntryPF[lBlockInd+1].valid = %d \n", astAssocEntryPF[lBlockInd+1].addr,astAssocEntryPF[lBlockInd+1].valid);
			////printf("Prefetch: Caching ++++ : BlockIndex = %lx, lTagId = %lx, Ptr = %lx \n", lBlockInd+1, lTagId+1, inPtr);
		}
		else
		{
                	ulReadCountPF--;
			//printf("End of List: Prefetch: Caching ++++ : BlockIndex = %lx, lTagId = %lx, Ptr = %lx \n", lBlockInd+1, lTagId, inPtr);
			if ((lBlockInd < (lTotalBlockCount-1)))
			{
			//	printf("Prefetched block exists: astAssocEntryPF[lBlockInd+1].addr = %lx, valid = %d \n", astAssocEntryPF[lBlockInd+1].addr, astAssocEntryPF[lBlockInd+1].valid);
			}
			else
			{
			//	printf("Prefetched block exists: astAssocEntryPF[0].addr = %lx, valid = %d \n", astAssocEntryPF[0].addr, astAssocEntryPF[0].valid);
			}
		}	
	}
}

void cache_block_assoc(unsigned long inPtr)
{
	struct list *new = NULL;
	//printf("cache_block: inPtr = %lu\n", inPtr);
	if (head == NULL)
	{
		//printf("cache_block: Creating header\n");
		head = (struct list *)malloc(sizeof(struct list));
		head->nxt = NULL;
		head->ptr = inPtr;
		lCurBlockCount++;
		return;
	}
	
	/* Delete the tail in case of over flow */
	if (lTotalBlockCount <= lCurBlockCount)
	{
		remove_tail(0);
                if (lCurBlockCount == 0)
                        head = NULL;

	}	
	/* Insert new at header */
	new = (struct list *)malloc(sizeof(struct list));
	new->ptr = inPtr;
	new->nxt = head;
	head = new;
	lCurBlockCount++;
	return;
}


int check_cache_assoc(unsigned long inPtr, int isPrefetch)
{
	struct list *temp;
	struct list *prev = NULL;
	
	if (isPrefetch == 1)
		temp = headPF;
	else
		temp = head;

	if (NULL == temp)
		return 0;

	if(temp->ptr == inPtr)
	{
		return 1;
	}
	prev = temp;
	temp = temp->nxt;
	
	while(temp)
	{
		if (temp->ptr == inPtr)
		//if (((inPtr - temp->ptr) >= 0) && ((inPtr - temp->ptr) < lBlockSize))
		{
			if (iCachePolicy == FIFO)
				return 1;
			/*Bring the pointer to head*/
			prev->nxt = temp->nxt;
			if (isPrefetch)
			{
				temp->nxt = headPF;
				headPF = temp;
			}	
			else
			{
				temp->nxt = head;
				head = temp;
			}
			return 1;
		}
		prev = temp;
		temp = temp->nxt;
	}
	return 0;
}

void cache_block_assoc_pf(unsigned long inPtr)
{
        struct list *new = NULL;
        //struct list *PrefetchNew = NULL;
        if (headPF == NULL)
        {
                //printf("cache_block_pf: Creating header\n");
                headPF = (struct list *)malloc(sizeof(struct list));
                headPF->ptr = inPtr;

                headPF->nxt = (struct list *)malloc(sizeof(struct list));
		headPF->nxt->nxt = NULL;
		headPF->nxt->ptr = inPtr+lBlockSize;
                lCurBlockCountPF++;
                lCurBlockCountPF++;
                return;
        }

        /* Delete the tail in case of over flow */
        if (lTotalBlockCount <= lCurBlockCountPF)
        {
                remove_tail(1);
//                remove_tail(1);
		if (lCurBlockCountPF == 0)
			headPF = NULL;
        }
        /* Insert new at header */
        new = (struct list *)malloc(sizeof(struct list));
	/*Interchanging */
        new->ptr = inPtr;
        lCurBlockCountPF++;

	/*Check for the prefecting block */
	//if (!check_entry_in_list(inPtr + lBlockSize, 1))
	//{		
              	remove_tail(1);
		new->nxt = (struct list *)malloc(sizeof(struct list));
        	new->nxt->nxt = headPF;
		new->nxt->ptr = inPtr + lBlockSize;
        	lCurBlockCountPF++;
		headPF = new;
  	//}
	
	//if (!check_entry_in_list(inPtr + lBlockSize, 1))
	//{		
#if 0
                remove_tail(1);
		new->nxt = headPF;
		PrefetchNew = (struct list *)malloc(sizeof(struct list));
        	PrefetchNew->nxt = new;
		PrefetchNew->ptr = inPtr + lBlockSize;
        	lCurBlockCountPF++;
		headPF = PrefetchNew;
  	//}

	else
	{
	//	printf("Prefetched is already there\n");
		new->nxt = headPF;
		headPF = new;
	}
#endif
        return;
}


int check_entry_in_list(unsigned long inPtr, int isPrefetch)
{
	struct list *temp;
	if (1 == isPrefetch)
		temp = headPF;
	else
		temp = head;
	
	while(temp)
	{	
		if (temp->ptr == inPtr)
			return 1;
		temp = temp->nxt;
	}
	return 0;
}


int checkNumPowerOfTwo(long lNum)
{
	int i;
	for (i=0; i<(sizeof(lNum)*8); i++)
	{
		if (((lNum>>i) &1) == 1)
		{
			if ((lNum>>(i+1)) != 0)
				return -1;
			else
				return i;
		}
	}
	return -1;
}

#ifdef SET_ASSOC

struct _stSetEntry * setassoc_alloc_set_tbl()
{
        return (struct _stSetEntry * )malloc(stSetAssocDetails.ulTotalSets * sizeof(SetEntry));
}


int setassoc_search_in_list(SetEntry *pstSetEntry, unsigned long ulTagId)
{
	unsigned long ulTempTagId = 0;
	SetList *temp = pstSetEntry->pstTail;
	if (NULL == temp)
		return 0;
	
	while (temp != pstSetEntry->pstHead)
	{
		if (temp->ulTagId == ulTagId)
		{
			//printf(" Hit :):):):)\n");
			if (FIFO == iCachePolicy)
				return 1;
			else
			{
				/*Need to make it hot: LRU*/
				setassoc_make_entry_hot(temp, pstSetEntry);
				return 1;
			}				
		}
		//Moving to previous
		temp = temp->prev;
	}	
	if (temp->ulTagId == ulTagId)
	{
		//printf("Head and tail are same\n");
		//printf(" Hit :):):):)\n");
		if (FIFO == iCachePolicy)
			return 1;
		else
		{
			/* LRU: If HEAD and TAIL for are same: Not need to do any thing */
			if (pstSetEntry->pstHead != pstSetEntry->pstTail)
			{
				/* Case 1: NULL --- HEAD --- TAIL -- NULL */
				/* Case 2: HEAD --- TAIL -- HEAD*/
				//if ((NULL == pstSetEntry->pstHead->prev) && (NULL == pstSetEntry->pstTail->nxt)
				if(pstSetEntry->pstTail->prev == pstSetEntry->pstHead)
				{
					ulTempTagId = pstSetEntry->pstHead->ulTagId;
					pstSetEntry->pstHead->ulTagId = pstSetEntry->pstTail->ulTagId;
					pstSetEntry->pstTail->ulTagId = ulTempTagId;
				}				
				else
				{
					setassoc_make_entry_hot(temp, pstSetEntry);	
				}				
			}
			return 1;
		}
	}
	else
	{
		return 0;
	}
	
}

/*
Node - Place where data found (Hit case)
1) Remove node from its place
2) Insert after tail 
3) Make Node as tail
 
Before function call:     ----Entry1----Node---Entry3----Entry4----Entry5(TAIL)
After function call :     ----Entry1----Entry3---Entry4---Entry5---Node(TAIL)
*/
void setassoc_make_entry_hot(SetList *pstNode, SetEntry *pstSetEntry)
{
	unsigned long temp;
	/*Case 1: Node is TAIL: Then Node is alreay hot */
	if (pstSetEntry->pstTail == pstNode)
	{
		/* No need to make hot */
		return;
	}
	
	/*Case 2: HEAD----Node----TAIL: Need to move Tail to Node */
	if (pstSetEntry->pstTail->prev == pstNode)	
	{
		temp = pstSetEntry->pstTail->ulTagId;
		pstSetEntry->pstTail->ulTagId = pstNode->ulTagId;
		pstNode->ulTagId = temp;
		return;
	}

	SetList *pstTail = pstSetEntry->pstTail;
	/* Handle special Case:  NULL--- Node ---TAIL--  */
	if (NULL != pstNode->prev)
		pstNode->prev->nxt = pstNode->nxt;
	
	pstNode->nxt->prev = pstNode->prev;

	/* Insert node after tail */
	/* Handle special Case:  NULL--- Node ---TAIL--  */
	if (NULL != pstTail->nxt)
		pstTail->nxt->prev = pstNode;

	pstNode->nxt = pstTail->nxt;
	pstTail->nxt = pstNode;
	pstNode->prev = pstTail;	
	pstSetEntry->pstTail = pstNode;
	return;
}

unsigned long get_set_index(unsigned long addr, int ulSetBitNum)
{
	unsigned long mask = ~(0);
	mask = mask << (sizeof(long)*8-(iCacheBitNum) + ulSetBitNum);
	//printf("mask = %lx\n", mask);
	mask = mask >> (sizeof(long)*8-(iCacheBitNum) + ulSetBitNum);
	//printf("mask = %lx\n", mask);

        unsigned long temp = addr;
	temp = temp & mask;

      	return temp>>iBlockBitNum;
}

int setassoc_lookup(unsigned long inPtr, int isPrefetch)
{
	SetEntry *pstSetEntry;
	unsigned long ulSetIndex = get_set_index(inPtr,stSetAssocDetails.ulSetBitNum);
	/*Shift addr by (cachebits-setbits )*/
	unsigned long ulTagId = inPtr>>(iCacheBitNum - stSetAssocDetails.ulSetBitNum);
	//printf("LookUp:------------------ Tag idex = %lx, Set index = %lx inPtr = %lx \n", ulTagId, ulSetIndex, inPtr);
	if (!isPrefetch)
		pstSetEntry = &(stSetAssocDetails.pstSetEntry[ulSetIndex]);	
	else
		pstSetEntry = &(stSetAssocDetails.pstSetEntryPF[ulSetIndex]);	

	if (NULL == pstSetEntry->pstTail)
	{
		return 0;
	}	
	return setassoc_search_in_list(pstSetEntry, ulTagId);
}

void setassoc_cache_block(unsigned long inPtr)
{
	SetEntry *pstSetEntry;
	unsigned long IndexPF=0;
	/* Get tag index */
	unsigned long ulSetIndex = get_set_index(inPtr,stSetAssocDetails.ulSetBitNum);
	/*Shift addr by (cachebits-setbits )*/
	unsigned long ulTagIndex = inPtr>>(iCacheBitNum - stSetAssocDetails.ulSetBitNum);

        if (FALSE == iPrefetchEnable)
	{
                pstSetEntry = &(stSetAssocDetails.pstSetEntry[ulSetIndex]);
        	setassoc_cache_list(pstSetEntry, ulTagIndex);
        }
	else
	{
                pstSetEntry = &(stSetAssocDetails.pstSetEntryPF[ulSetIndex]);
        	setassoc_cache_list(pstSetEntry, ulTagIndex);

		//IndexPF = 1;
		if (0 != stSetAssocDetails.ulSetValue)
		{
			IndexPF = (ulSetIndex+1)%stSetAssocDetails.ulTotalSets;  
			if (IndexPF == 0) /*It means full associativity */
				ulTagIndex = ulTagIndex + 1;
              	
			pstSetEntry = &(stSetAssocDetails.pstSetEntryPF[IndexPF]);
			if (1 == setassoc_search_in_list(pstSetEntry, ulTagIndex))
			{			
				//printf("Found entry which is about be prefetched\n");
				ulReadCountPF--;
      			}
			else
			{
				setassoc_cache_list(pstSetEntry, ulTagIndex);
			}
		}
		else
		{
			ulReadCountPF--;
		}
	}
}

void setassoc_cache_list(SetEntry *pstSetEntry, unsigned long ulTagId)
{
	SetList *new;
	/*if last == NULL, create head*/
	//printf("Caching +++++++++++++++++ = %lx\n", ulTagId);
	if (NULL == pstSetEntry->pstHead)
	{
		pstSetEntry->uiNoOfEntry++;
		pstSetEntry->pstHead = (SetList *)malloc(sizeof(SetList));
		pstSetEntry->pstHead->ulTagId = ulTagId;
		pstSetEntry->pstHead->nxt = NULL;
		pstSetEntry->pstHead->prev = NULL;
		pstSetEntry->pstTail = pstSetEntry->pstHead;		
		//printf("Creating header 111111111111111111111111111111111111111111111111 \n");
		return;
	}

	/*If there is no overlap, make new entry
	uiNoOfEntry should be less than stSetAssocDetails.ulSetValue */
	if (pstSetEntry->uiNoOfEntry  < stSetAssocDetails.ulSetValue)
	{
		pstSetEntry->uiNoOfEntry++;

        	new = (SetList *)malloc(sizeof(SetList));
        	new->ulTagId = ulTagId;		
        	new->prev = pstSetEntry->pstTail;
        	new->nxt = pstSetEntry->pstTail->nxt;

		pstSetEntry->pstTail->nxt = new;

        	pstSetEntry->pstTail = new;
		//printf("Not Over lap 2222222222222222222222222222222222222222222222222222222Overloaped\n");
        	
		/* In case of equal... make circular list */
		if (pstSetEntry->uiNoOfEntry  == stSetAssocDetails.ulSetValue)
		{	
			pstSetEntry->pstTail->nxt = pstSetEntry->pstHead;
			pstSetEntry->pstHead->prev = pstSetEntry->pstTail;
			//printf("Justpt Over lap 333333333333333333333333333333333333333333333333333Overloaped\n");
			pstSetEntry->iJustOverLap = 1;		
		}
	}
	else /*Need to create new entry: Update head: FIFO case*/
	{
		//Update head
		pstSetEntry->pstHead->ulTagId = ulTagId;
		//Move head to next
		pstSetEntry->pstHead = pstSetEntry->pstHead->nxt;
		
		//Update tail to next
		pstSetEntry->pstTail = pstSetEntry->pstTail->nxt;
		//printf("Cached in Overlaped 444444444444444444444444444444444444444444444444444444444444\n");
		pstSetEntry->iCachedOverLap = 1;		
	}

	return;
}

#endif
