// R. Jesse Chaney
// rchaney@pdx.edu

//  Dylan Herrigstad
//  dyl29@pdx.edu

#include "vikalloc.h"

#define BLOCK_SIZE (sizeof(mem_block_t))
#define BLOCK_DATA(__curr) (((void *) __curr) + (BLOCK_SIZE))
#define DATA_BLOCK(__data) ((mem_block_t *) (__data - BLOCK_SIZE))

#define IS_FREE(__curr) ((__curr -> size) == 0)

#define PTR "0x%07lx"
#define PTR_T PTR "\t"

static mem_block_t *block_list_head = NULL;
static mem_block_t *block_list_tail = NULL;
static void *low_water_mark = NULL;
static void *high_water_mark = NULL;
// only used in next-fit algorithm
static mem_block_t *prev_fit = NULL;

static uint8_t isVerbose = FALSE;
static vikalloc_fit_algorithm_t fit_algorithm = FIRST_FIT;
static FILE *vikalloc_log_stream = NULL;

static void init_streams(void) __attribute__((constructor));

static size_t min_sbrk_size = MIN_SBRK_SIZE;

static void 
init_streams(void)
{
    vikalloc_log_stream = stderr;
}

size_t
vikalloc_set_min(size_t size)
{
    if (0 == size) {
        // just return the current value
        return min_sbrk_size;
    }
    if (size < (BLOCK_SIZE + BLOCK_SIZE)) {
        // In the event that it is set to something silly small.
        size = MAX(BLOCK_SIZE + BLOCK_SIZE, SILLY_SBRK_SIZE);
    }
    min_sbrk_size = size;

    return min_sbrk_size;
}

void 
vikalloc_set_algorithm(vikalloc_fit_algorithm_t algorithm)
{
    fit_algorithm = algorithm;
    if (isVerbose) {
        switch (algorithm) {
        case FIRST_FIT:
            fprintf(vikalloc_log_stream, "** First fit selected\n");
            break;
        case BEST_FIT:
            fprintf(vikalloc_log_stream, "** Best fit selected\n");
            break;
        case WORST_FIT:
            fprintf(vikalloc_log_stream, "** Worst fit selected\n");
            break;
        case NEXT_FIT:
            fprintf(vikalloc_log_stream, "** Next fit selected\n");
            break;
        default:
            fprintf(vikalloc_log_stream, "** Algorithm not recognized %d\n"
                    , algorithm);
            fit_algorithm = FIRST_FIT;
            break;
        }
    }
}

void
vikalloc_set_verbose(uint8_t verbosity)
{
    isVerbose = verbosity;
    if (isVerbose) {
        fprintf(vikalloc_log_stream, "Verbose enabled\n");
    }
}

void
vikalloc_set_log(FILE *stream)
{
    vikalloc_log_stream = stream;
}


void *
vikalloc(size_t size)
{
    mem_block_t *curr = NULL;
    mem_block_t *prev = NULL;

    if (isVerbose) {
        fprintf(vikalloc_log_stream, ">> %d: %s entry: size = %lu\n"
                , __LINE__, __FUNCTION__, size);
    }

    //If nothing/trash is passed via parameter      
    if(size == 0 || !size) //unsigned int is always false
    {
        return NULL; // Some professors would skewer me for using two returns in a function block
                     // But I didn't see any specifications (or condemnation) in the lab1 details!
    }
    //If empty
    if(block_list_head == NULL)     //low_water_mark == NULL)
    {
        //determine minimum increment/multiple of min_sbrk_size
        int i = 1;
        //mem_block_t = a;
        while ((i * min_sbrk_size) <= (size + BLOCK_SIZE))
        {
            i++;
        } 
        //Allocate size + header w/ sbrk
        curr = sbrk(i * min_sbrk_size);     // !!!!!SBRK CALL #1!!!!!!
        //  set size
        curr->size = size;
        //  set capacity
        curr->capacity = ((i * min_sbrk_size) - (BLOCK_SIZE));
        // updating ptr's
        curr->next = NULL;
        curr->prev = NULL;
        block_list_head = curr; // BASED ON INSTRUCTION AND MEM_BLOCK_T, I THINK THIS IS INTENDED AS A POINTER TO THE FIRST MEMORY BLOCK.
        //block_list_tail = curr; // First and only node
        low_water_mark = block_list_head; // Setting the new LWM
        high_water_mark = (low_water_mark + (i * min_sbrk_size)); //Setting the HWM        
        brk(high_water_mark);               // !!!!!BRK CALL #1!!!!!!!   Setting upper bounds of the bloc
        //block_list_head = curr;       
        //low_water_mark = block_list_head;
        //high_water_mark = low_water_mark + (i * min_sbrk_size); // !!!POTENTIAL REWORK!!!
        //     ptr artithmetic/ +/comp or - only
        // set upper bound w/ brk using i?
    }

    //else if NOT empty
    else if(block_list_head != NULL)
    {
        // First fit
        //      Sentinel + reassign ptr
        int sentinel = 0;
        curr = block_list_head; //low_water_mark;
        // Look for free block and split
        while(curr != NULL && sentinel == 0)
        {
            // FIRST TO CHECK IF IT'S FREE
            // THEN CHECK IF IT FITS
            if(IS_FREE(curr) && (curr->capacity >= size))
            {
                curr->size = size;  //Update size
                                    //Removes free status
                }
                sentinel = 1; // Break while loop
            
            }
            // IT'S NOT FREE
            // BUT IT FITS
            else if((curr->capacity - curr->size) >= (size + BLOCK_SIZE))
            {
                prev = curr;
                curr = BLOCK_DATA(prev) + prev->size;
                curr->size = size;
                curr->capacity = prev->capacity - prev->size - BLOCK_SIZE; // BLOCK SIZE NOT OF PREV, BUT OF CURR
                prev->capacity = prev->size;
                curr->next = prev->next;
                curr->prev = prev;
                prev->next = curr;
                    
                if(curr->next) //NULL test & reassignment
                {
                    curr->next->prev = curr;
                } 

                sentinel = 1; // Break while loop
            }
            else
            {
                curr = curr->next; // Increment until success or sentinel break
            }
        }
        // End of the line, we need new memory 
        if(sentinel == 0)
        {
            int index = 1;

            // Determine size
            while((index * min_sbrk_size) <= (size + BLOCK_SIZE))
            {
                index++;
            }
            prev = block_list_head; // Set to front

            // Cycle to end
            while(prev->next != NULL)
            {
                prev = prev->next;
            }
            
            // Allocate memory
            curr = sbrk(min_sbrk_size * index);     // SBRK CALL #2 - ONLY FOR MORE MEMORY!
            
            // Error checking
            if(errno == ENOMEM)
            {
                //errno = ENOMEM;
                return NULL;
            }
            // Update cap
            curr->size = size;
            curr->capacity = ((index * min_sbrk_size) - (BLOCK_SIZE));
            curr->prev = prev;
            prev->next = curr;
            curr->next = NULL;
            high_water_mark = curr;
            high_water_mark = high_water_mark + (index * min_sbrk_size);
            brk(high_water_mark);   // BRK CALL #2 - ONLY TO SET HIGH BOUNDS!
        }
    }

    return BLOCK_DATA(curr);
}

void 
vikfree(void *ptr)
{
    mem_block_t *curr = NULL;
    mem_block_t *next = NULL;
    mem_block_t *prev = NULL;
    int sentinel;

    sentinel = 0;
    curr = block_list_head; // Assigning to iterate

    while(curr != NULL && sentinel == 0)
    {
        if(BLOCK_DATA(curr) == ptr)
        {
            if(IS_FREE(curr))
            {
                // IF BLOCK IS FREE
                if (isVerbose) {        // THE DIRECTION SHOWN TO US IN CLASS!!!
                fprintf(vikalloc_log_stream, "Block is already free: ptr = "PTR"\n",(long) ( ptr-low_water_mark));
                }
                //      UPDATE SENTINEL
                sentinel = 1;
            }
            else
            {
                // FREE BLOCK
                //      NULL DATA
                //      CHANGE SENTINEL
                curr->size = 0;
                sentinel = 1;
            }

        }
        else
        {
            curr = curr->next;
        }
    }
    //If empty
    if(!curr) //Empty list or if Ptr was already free
    {
        return;
    }
    
    // IF CURRENT IS NULL AND PREV IS NULL
    if((curr->prev) && (IS_FREE(curr)) && (IS_FREE(curr->prev)))
    {
        prev = curr->prev;
        prev->capacity = curr->capacity + prev->capacity + BLOCK_SIZE;
        
        if(curr->next)//adjust for top
        {
            curr->next->prev = curr->prev;
        }
        //then bottom
        if(curr->prev)
        {
            curr->prev->next = curr->next;
        }
        //READJUST PTRS
        curr->next = NULL;
        curr->prev = NULL;
        curr = prev;
    }

    // IF CURR IS NULL AND NEXT IS NULL
    if(curr->next && (IS_FREE(curr)) && (IS_FREE(curr->next)))
    {
        next = curr->next;
        curr->capacity = curr->capacity + next->capacity + BLOCK_SIZE;
        
        if(next->next)
        {
            next->next->prev = next->prev;
        }
        if(next->prev) // ALLOW BOTTOM ABSORBTION
        {
            next->prev->next = next->next;
        }
        // REASSIGN PTRS
        next->next = NULL;
        next->prev = NULL;
        next = curr;
    }

    return;
}

///////////////

void 
vikalloc_reset(void)
{
    if (isVerbose) {
        fprintf(vikalloc_log_stream, ">> %d: %s entry\n"
                , __LINE__, __FUNCTION__);
    }

    if (low_water_mark != NULL) {
        if (isVerbose) {
            fprintf(vikalloc_log_stream, "*** Resetting all vikalloc space ***\n");
        }
    }

    //
    //  Resets limits! At the list of seeming stupid, it wasn't requested to free memory, so I just reset all of the ptr's 
    //
    brk(low_water_mark);        // SETS BRK TO THE BEGINNING
    block_list_head = NULL; 
    block_list_tail = NULL;
    low_water_mark = NULL;
    high_water_mark = NULL;
}


void *
vikcalloc(size_t nmemb, size_t size)
{
    // Get ptr's    
    void *ptr = NULL;
    size_t tempSize = nmemb * size;
    
    if (isVerbose) {
        fprintf(vikalloc_log_stream, ">> %d: %s entry\n"
                , __LINE__, __FUNCTION__);
    }

    // If nothing/trash is passed
    if(!nmemb || !size)
    {
        return NULL; //If nmemb or size is 0, then calloc() returns either NULL
    }
    
    /*
        NOTE: By virture of the values being unsigned, it seems they're protected from possible integer overflows, 
        as size_t can't go negative- so i've left that protection out from the manual entry.
    */

    //  vikalloc 
    ptr = vikalloc(tempSize);
    // memset returned ptr
    memset(ptr, 0, tempSize); 

    return ptr;
}

void *
vikrealloc(void *ptr, size_t size)
{
    int sentinel = 0;
    void *tempPtr = NULL;
    mem_block_t *curr = block_list_head;  //      Assign CURR to head
    //      Next?
    //          If next->size > curr->size && curr->size >= size

    if (isVerbose) {
        fprintf(vikalloc_log_stream, ">> %d: %s entry\n"
                , __LINE__, __FUNCTION__);
    }
    
    //  Test for empty/trash
    if(!ptr) 
    {
        return vikalloc(size); // On the off-chance this is requested a pointer can be returned.
    }
    while(curr != NULL && sentinel == 0)
    {
        if(BLOCK_DATA(curr) == ptr) //BLOCK_SIZE (sizeof(mem_block_t))
        {
            if(size <= curr->capacity)
            {
                curr->size = size;
                sentinel = 1; // Loop break
                // ESTABLISH CURR IN THIS POSITION? SENTINEL REWORK?
            }
            else
            {
                tempPtr = vikalloc(size);   // New block is allocated
                memcpy(tempPtr, ptr, size); // contents are copied
                vikfree(ptr);   // old block is deallocated
                sentinel = 9;   // !!!!!!!!sentunel for ptr return!!!!!!!!
            }
        }
        else
        {
            curr = curr->next; // else, increment
        }
    }

    //      if sentinel was flagged IN LINE 496
    if (sentinel == 9)
    {
        return tempPtr; // THIS WILL EXIT THE LOOP SINCE PTR WAS VIKFREE'D
    }
    
    return ptr;
}

void *
vikstrdup(const char *s)
{
    void *ptr = NULL;

    if (isVerbose) {
        fprintf(vikalloc_log_stream, ">> %d: %s entry\n"
                , __LINE__, __FUNCTION__);
    }

    // If nothing/trash is passed as a parameter
    if(s == NULL)
    {
        return NULL;
    }
    // Else if s is valid
    else
    {
        size_t strSize = (strlen(s) + 1); // copy size + Null char
        ptr = vikalloc(strSize);    // Allocate room for the size of the str
        memcpy(ptr, s, strSize);    // Copy into memory
    }

    return ptr; // Return copied str
}



#include "vikalloc_dump.c"
