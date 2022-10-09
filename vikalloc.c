// R. Jesse Chaney
// rchaney@pdx.edu

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

/*
 *              NOTES:
 *      First alloc, then split
 *      First free, then coalese
 *
 *      How to free header when coalesce?
 *
 *      Reset - Use brk
 *
 *      fprint(vikalloc_log_stream, (long), (low_water_mark);
 *  
 *
 *      Remove -Werror in Make?
 *
 *      NEW -- MONDAY AT 11:59pm!!
*/


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
    if((size == 0) || (!size)) //unsigned int is always false
    {
        return NULL; // Some professors would skewer me for using two returns in a function block
                     // But I didn't see any specifications (or condemnation) in the lab1 details!
    }
    //If empty
    if(block_list_head == NULL)
    {
        //determine minimum increment/multiple of min_sbrk_size
        int i = 1;
        while (i * min_sbrk_size <= (size + BLOCK_SIZE))
        {
            i++;
        } 
        //Allocate size + header w/ sbrk
        curr = sbrk(i * min_sbrk_size);     // !!!!!SBRK CALL #1!!!!!!
        //  set size
        curr->size = (size);
        //  set capacity
        curr->capacity = ((i * min_sbrk_size) - BLOCK_SIZE);
        // updating ptr's
        curr->next = NULL;
        curr->prev = NULL;
        block_list_head = curr;
        low_water_mark = block_list_head;
        high_water_mark = low_water_mark + (i * min_sbrk_size); // !!!POTENTIAL REWORK!!!
                                                                //     int->string->hex? 0X400=1024
                                                                //     ptr artithmetic
        // set upper bound w/ brk using i?
    }
    //else if NOT empty
    else if(block_list_head != NULL)
    {
        // First fit
        int sentinel = 0;
        curr = block_list_head;


        //  increment through the DLL
        //      if found AND fits
        //          split
        //      else
        //          not enough room?
        //
    
    }
    // Assign pointers
    // Create sentinel
    
    //  While not NULL & sentinel is 0
    //      !!NOTE: FIND OUT HOW TO TRACK FREE!!
    //          CANNOT CHANGE MEM_BLOCK_T
    //      IF Free AND Capacity >= size
    //          Kick rocks, not here
    //          update sentinel
    //      ELSE IF FITS
    //          update PTR
    //          update capacity + size
    //          Link DLL
    //          update sentinel
    //      ELSE
    //          iterate
    //  IF sentinel = 0 && New Mem
    //      Determine min size necessary
    //      Update ptr's
    //      Update size/capacity
    //      Update bounds



    return BLOCK_DATA(curr);
}

void 
vikfree(void *ptr)
{
    if (isVerbose) {
        fprintf(vikalloc_log_stream, ">> %d: %s entry\n"
                , __LINE__, __FUNCTION__);
    }

    // Assign pointers
    //      iterators next/prev
    //      assign CURR to block head
    // Create sentinel

    //  IF CURR == false
    //      return
    //  Check for Coalesce
    //      If prev = open && curr = open
    //      If curr = open && next = open
    //      update ptr's
    //      coalesce


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

    // RESET!
    // Sets     upper address to lower bounds, 
    //          sets block-list-head to NULL
    //          resets lower and high water marks to NULL
    brk(low_water_mark);
    block_list_head = NULL;
    low_water_mark = NULL;
    high_water_mark = NULL;
}

void *
vikcalloc(size_t nmemb, size_t size)
{
    // Get ptr's    
    void *ptr = NULL;
    size_t temp = nmemb * size;
    
    if (isVerbose) {
        fprintf(vikalloc_log_stream, ">> %d: %s entry\n"
                , __LINE__, __FUNCTION__);
    }

    // If nothing/trash is passed
    if(!nmemb || !size)
    {
        return;
    }
    //  vikalloc 
    ptr = vikalloc(temp);
    // memset returned ptr
    memset(ptr, 0, temp); 

    return ptr;
}

void *
vikrealloc(void *ptr, size_t size)
{
    if (isVerbose) {
        fprintf(vikalloc_log_stream, ">> %d: %s entry\n"
                , __LINE__, __FUNCTION__);
    }
    
    // Variable creation
    //      sentinel = 0
    //      ADD PTR
    //      Assign CURR to head
    //      Next?
    //          If next->size > curr->size && curr->size >= size
    //
    //  Test for empty/trash
    //      If !size
    //          Null
    //      If !ptr
    //          return vikalloc ptr
    //
    //      While !curr && sentinel
    //          If CURR = ptr
    //              If curr->size > size
    //                  Update curr
    //                  Update sentinel = 1?
    //              Else
    //                  ***ADD PTR***                  
    //                  memcpy
    //                  free
    //                  update sentinel = 2?
    //          Else
    //              Increement
    //      If sentinel = 1
    //      if sentinel = 2

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
    // Else if str is valid
    else
    {
        size_t strSize = strlen(s) + 1;
        ptr = vikalloc(strSize);
        memcpy(ptr, s, strSize);
    }

    return ptr;
}

#include "vikalloc_dump.c"
