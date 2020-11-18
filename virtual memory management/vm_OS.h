/*
 * vm_OS.h
 *
 * Helper functions and variables
 */

#ifndef _VM_HELPER_H_
#define _VM_HELPER_H_

#include "vm_arena.h"
#include "vm_pager.h"

#include <vector>
#include <unordered_map>
#include <list>
#include <unordered_set>
#include <string>


struct OS_page_entry 
{
    OS_page_entry(page_table_entry_t* const entry_ptr, bool _isSwapBack, 
        int block_index, std::string _file_name):
        isDirty(true), isReferenced(true), isResident(true), isSwapBack(_isSwapBack),
        swap_or_file_block_index(block_index), file_name(_file_name), original_entry_ptr(entry_ptr){}
    // don't need dtor

    bool isDirty, isReferenced, isResident, isSwapBack;

    // for swap-backed page, if this page is not in swap space, make it -1
    // for file-backed page, make it file block index
    int swap_or_file_block_index;
    
    // for swap-backed page, make it empty
    // for file-backed page, make it the file name
    const std::string file_name;

    // Infrastructure page table entry
    page_table_entry_t* original_entry_ptr;

    // just use the default copy/move ctor/assignment operator
};


struct Process 
{
    Process():invalid_arena_start(VM_ARENA_BASEADDR){
        for(unsigned i = 0; i < VM_ARENA_SIZE/VM_PAGESIZE; ++i){
            original_pagetable.ptes[i].ppage = 0;
            original_pagetable.ptes[i].read_enable = 0;
            original_pagetable.ptes[i].write_enable = 0;
        }
        // avoid the vector reallocate and make the owners pointer invalid
        OS_process_pagetable.reserve(VM_ARENA_SIZE/VM_PAGESIZE);
    }
    ~Process();

    // OS page table with extra bits.
    // only push_back when we make new page valid, so we don't need isValid bit
    std::vector<OS_page_entry> OS_process_pagetable;

    // real page table. Use for PTBR
    page_table_t original_pagetable;

    // Keep track of valid arena space (note: always increases)
    void* invalid_arena_start;  

    Process(const Process&) = delete;
    Process(Process&&) = delete;
    Process& operator=(const Process&) = delete;
    Process& operator=(Process&&) = delete;
};

// indicate the status of each page in physical memory/swap space
struct physical_page_t{
    std::unordered_set<OS_page_entry*> owners;
    
    physical_page_t(){}
    // don't need to write dtor
    
    // only allow move assignment
    physical_page_t(const physical_page_t&) = delete;
    physical_page_t& operator=(const physical_page_t&) = delete;
    
    physical_page_t(physical_page_t&& rhs) noexcept:owners(std::move(rhs.owners)){}
    physical_page_t& operator=(physical_page_t&& rhs) noexcept{
        owners = std::move(rhs.owners);
        return *this;
    }
};

struct File_block_info{
    std::string filename;
    unsigned block_index;
    struct Hasher{
        size_t operator()(const File_block_info& block) const{
            std::hash<std::string> str_hasher;
            std::hash<unsigned> int_hasher;
            std::hash<size_t> size_hasher;
            return size_hasher(str_hasher(block.filename) + int_hasher(block.block_index));
        }
    };
    struct Equal{
        bool operator()(const File_block_info& lhs, const File_block_info& rhs)const{
            return (lhs.filename == rhs.filename) && (lhs.block_index == rhs.block_index);
        }
    };
};



// Container for variables and functions for pager
class OS_Pager 
{
    public:

    // clock queue - integer represents the pages index in physical memory
    static std::list<unsigned int> clock;
    
    // OS data structure of physical memory, use physical_page_t to indicate the use of each
    // page in physical memory
    // the size of it should be physical page number
    static std::vector<physical_page_t> physical_memory; 

    // OS data structure indicating the use of all swap blocks
    static std::vector<physical_page_t> swap_blocks;

    // OS data structure indicating the use of each file opened
    static std::unordered_map<std::string, std::vector<physical_page_t> > file_blocks;

    // all current live processes
    static std::unordered_map<pid_t, Process*> processes;

    // use this set to manage free pages in physical memory and swap space
    static std::unordered_set<unsigned> free_physmem_pages, free_swap_blocks;

    // use this set to track if a file block is in the physical memory
    static std::unordered_map<File_block_info, unsigned, \
        File_block_info::Hasher, File_block_info::Equal> file_blocks_in_physmem;

    static std::unordered_set<File_block_info, File_block_info::Hasher, File_block_info::Equal> file_blocks_seen;
    
    static unsigned pinning_page_number;
    
    static pid_t curr_process;
    
    // for eager swap reservation, track how much pages in phsical memory,  
    // update when we create new swap-backed pages and kill a process
    static unsigned total_swapbacked_pages;
    
    // --------------- Helper functions --------------- //

    // Given a page number, convert to physical address in vm_physmem
    static char* physmem_page_to_address(unsigned int);

    // Given *filename that resides in application virtual space, form a string version
    static std::string read_c_string(const char *filename);
    
    // Given ptr to virtual address, return virtual page number. 
    static unsigned virtual_address_to_page(const void*);

    // Returns corrsponding virtual page AND offset as <page num, offset> 
    static std::pair<unsigned, unsigned> vAddress_to_page_offset(const void*);
    
    // Given index into OS_Pager::physical_memory, return pointer to 
    // a OS virtual page owner of that physical page.
    static OS_page_entry* get_virtual_owner(unsigned int phys_mem_index);

    // run clock algorithm and evict one page
    static void evict_pages();

    // Checks physical_memory, swap_blocks, file_blocks
    // returns true if owners have identical bits.
    static void check_consistent();
};


#endif /* _VM_HELPER_H_ */


/*
482 P3 Notes

Good piazza posts
vm_destroy(): @1513 
- should do liveness of physical page analysis (i.e. how many references it)

State Diagram/Thoughts
if reference == 0 || valid == 0 || resident 0 
then BOTH r and w == 0
	- must trap if not referenced to make referenced 1
	- must trap if not valid to return error
	- must trap if not resident to make resident (i.e. read from disk)

if ALL reference, valid, resident == 1 
then we have to look at dirty bit.
	if dirty bit == 0, then r: 1, w: 0 
		- if not modified, we allow app to read. If app writes, we must trap to make dirty bit: 1
	if dirty bit == 1, then r: 1, w: 1
		- if modified, then we can read or write to it without problems

Pinned pages


notes:
- page can only be evicted if resident: 1 (then turn into resident: 0)
- 

Questions
- is there a way for virtual pages to be made invalid (i.e. arena always fills bottom to up)


*/