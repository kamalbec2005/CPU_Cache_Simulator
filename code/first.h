#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#define LRU 1
#define READ 0
#define WRITE 1

#define TRUE 1
#define FALSE 0

#define FIFO 0
#define LRU 1 
 
#define VALID 1
#define NOTVALID 0

#define MAX_ADDRESS_SIZE 48
#define MAX_ADDRESS 0xFFFFFFFFFFFF

#define SET_ASSOC 1

typedef long u_l;

typedef struct input_s
{
	unsigned long addr;
	int mode;
	int res;
}inputs;

struct list {
        unsigned long ptr;
        struct list *nxt;
};

enum CacheAssoc
{
	DIRECT, ASSOC, ASSOC_SET
};

struct AssocEntry
{
	int valid;
	unsigned long addr;
	int res;
};	 


#ifdef SET_ASSOC
typedef struct _stSetList /* Header of each set */
{
        unsigned int uiValid;
        unsigned long ulTagId;
        struct _stSetList *nxt;        
        struct _stSetList *prev;        
}SetList;

typedef struct _stSetEntry /*array of sets*/
{
	unsigned int uiSetAssoc;
	unsigned int uiNoOfEntry;
	SetList *pstHead;	
	SetList *pstTail;	
	unsigned int iJustOverLap;
	unsigned int iCachedOverLap;
}SetEntry;

typedef struct _stSetAssocDetails /*Details about set assoc*/
{
	unsigned long ulSetValue;
	int ulSetBitNum;
	int res;
	unsigned long ulTotalSets;
	struct _stSetEntry *pstSetEntry;
	struct _stSetEntry *pstSetEntryPF;
}SetAssocDetails;

struct _stSetEntry * setassoc_alloc_set_tbl();
int setassoc_lookup(unsigned long inPtr, int isPrefetch);
int setassoc_search_in_list(SetEntry *pstSetEntry, unsigned long ulTagId);


unsigned long get_set_index(unsigned long addr, int ulSetBitNum);
void setassoc_cache_list(SetEntry *pstSetEntry, unsigned long ulTagId);
void setassoc_cache_block(unsigned long inPtr);
void setassoc_make_entry_hot(SetList *pstNode, SetEntry *pstSetEntry);
#endif

void cache_block(unsigned long inPtr);
void cache_block_direct(unsigned long inPtr);
void cache_block_assoc(unsigned long inPtr);
void cache_block_assoc_set(unsigned long inPtr);
void cache_block_assoc_pf(unsigned long inPtr);
int check_cache(unsigned long inPtr, int isPrefetch);
int check_cache_assoc(unsigned long inPtr, int isPrefetch);
int check_cache_assoc_set(unsigned long inPtr, int isPrefetch);
int check_cache_direct(unsigned long inPtr, int isPrefetch);
void display_list(void);
int parse_line(char *line, inputs *pstInputs);
void remove_tail(int isPrefetch);
int checkNumPowerOfTwo(long);
int check_entry_in_list(unsigned long inPtr, int isPrefetch);

