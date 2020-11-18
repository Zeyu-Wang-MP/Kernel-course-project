/*
 * vm_helper.cpp
 *
 * Helper Function declarations for vm_helper.cpp
 */

#include "vm_OS.h"
#include "vm_pager.h"

#include <cassert>
#include <utility>
#include <algorithm>
#include <iostream>

using std::vector;
using std::unordered_map;
using std::list;
using std::unordered_set;
using std::string;
using std::pair;

// static member initialization
list<unsigned> OS_Pager::clock = list<unsigned>();
vector<physical_page_t> OS_Pager::physical_memory = vector<physical_page_t>();
vector<physical_page_t> OS_Pager::swap_blocks = vector<physical_page_t>();
unordered_map<string, vector<physical_page_t>> OS_Pager::file_blocks = unordered_map<string, vector<physical_page_t>>();
unordered_map<pid_t, Process*> OS_Pager::processes = unordered_map<pid_t, Process*>();
unordered_set<unsigned> OS_Pager::free_physmem_pages = unordered_set<unsigned>();
unordered_set<unsigned> OS_Pager::free_swap_blocks = unordered_set<unsigned>();
unordered_map<File_block_info, unsigned, File_block_info::Hasher, File_block_info::Equal> \
    OS_Pager::file_blocks_in_physmem = unordered_map<File_block_info, unsigned, File_block_info::Hasher, File_block_info::Equal>();
unordered_set<File_block_info, File_block_info::Hasher, File_block_info::Equal> \
    OS_Pager::file_blocks_seen = unordered_set<File_block_info, File_block_info::Hasher, File_block_info::Equal>();
unsigned OS_Pager::pinning_page_number = 0;
pid_t OS_Pager::curr_process = -1;
unsigned OS_Pager::total_swapbacked_pages = 0;

void OS_Pager::check_consistent()
{
    for(unsigned i = 1; i < physical_memory.size(); ++i)
    {
        if(physical_memory[i].owners.size() >= 2)
        {
            auto prev_elem = *physical_memory[i].owners.begin();
            for (const auto& elem: physical_memory[i].owners) {
                // Consistent OS page entries
                assert(elem->isDirty == prev_elem->isDirty);
                assert(elem->isReferenced == prev_elem->isReferenced);
                assert(elem->isResident == prev_elem->isResident);
                assert(elem->isSwapBack == prev_elem->isSwapBack);
                assert(elem->file_name == prev_elem->file_name);
                
                // Consistent infrastructure page entries
                assert(elem->original_entry_ptr == elem->original_entry_ptr);

                prev_elem = elem;

                // Check infrastructure consistent with OS (below applies only if not pinned page)
                if(i != physical_memory.size() - 1)
                {
                    if(!prev_elem->isReferenced || !prev_elem->isResident)
                    {
                        assert(prev_elem->original_entry_ptr->read_enable == false);
                        assert(prev_elem->original_entry_ptr->write_enable == false);
                    }
                    else if(prev_elem->isDirty)
                    {
                        assert(prev_elem->original_entry_ptr->read_enable == true);
                        assert(prev_elem->original_entry_ptr->write_enable == true);
                    }
                    else if(!prev_elem->isDirty)
                    {
                        assert(prev_elem->original_entry_ptr->read_enable == true);
                        assert(prev_elem->original_entry_ptr->write_enable == false);
                    }
                }
            }
        }   
    }

    for(unsigned i = 0; i < OS_Pager::swap_blocks.size(); ++i)
    {
        if(swap_blocks[i].owners.size() >= 2)
        {
            auto prev_elem = *swap_blocks[i].owners.begin();
            for (const auto& elem: swap_blocks[i].owners) {
                // Consistent OS page entries
                assert(elem->isDirty == prev_elem->isDirty);
                assert(elem->isReferenced == prev_elem->isReferenced);
                assert(elem->isResident == prev_elem->isResident);
                assert(elem->isSwapBack == prev_elem->isSwapBack);
                assert(elem->isSwapBack == true);
                
                // Consistent infrastructure page entries
                assert(elem->original_entry_ptr == elem->original_entry_ptr);

                prev_elem = elem;
            }
        }   
    }
    
    // For each file
    for (const auto& file : OS_Pager::file_blocks)
    {   
        // For each file block in file
        for (const auto& block : file.second)
        {
            if(block.owners.size() >= 2)
            {
                // Check owner information is consistent
                auto prev_elem = *block.owners.begin();
                for (const auto& elem: block.owners)
                {
                    // Consistent OS page entries
                    assert(elem->isDirty == prev_elem->isDirty);
                    assert(elem->isReferenced == prev_elem->isReferenced);
                    assert(elem->isResident == prev_elem->isResident);
                    assert(elem->isSwapBack == prev_elem->isSwapBack);
                    assert(elem->isSwapBack == false);
                    
                    // Consistent infrastructure page entries
                    assert(elem->original_entry_ptr == elem->original_entry_ptr);

                    prev_elem = elem;
                }
            }
        }
    }
    
}

char* OS_Pager::physmem_page_to_address(unsigned int pageNumber)
{
    return &((char*)vm_physmem)[pageNumber* VM_PAGESIZE];
}

unsigned OS_Pager::virtual_address_to_page(const void* virtual_address)
{
    return ((unsigned)((uintptr_t)virtual_address - (uintptr_t)VM_ARENA_BASEADDR)) / VM_PAGESIZE;
}

// <page num, offset>
std::pair<unsigned, unsigned> OS_Pager::vAddress_to_page_offset(const void* virtual_address)
{
    unsigned int pageOffset;
    unsigned int pageNumber;

    pageOffset = ((uintptr_t)virtual_address - (uintptr_t)VM_ARENA_BASEADDR) % VM_PAGESIZE;
    pageNumber = OS_Pager::virtual_address_to_page(virtual_address);

    std::pair<unsigned, unsigned> returnPair(pageNumber, pageOffset);
    return returnPair;
}


string OS_Pager::read_c_string(const char *filename)
{
    if((uintptr_t)filename < (uintptr_t)VM_ARENA_BASEADDR || \
        (uintptr_t)filename >= ((uintptr_t)VM_ARENA_BASEADDR + VM_ARENA_SIZE)){
        throw std::runtime_error("c-string malformed/out of bounds");
    }
    Process* curr_process = OS_Pager::processes[OS_Pager::curr_process];
    std::string converted_filename;
    bool nullfound = false;

    // Get virtual page number and virtual page offset of *filename.
    unsigned int file_vpage_num = OS_Pager::vAddress_to_page_offset(filename).first;
    unsigned int file_vpage_offset = OS_Pager::vAddress_to_page_offset(filename).second;

    while(!nullfound)
    {
        char * readAddress = (char*)(file_vpage_num * VM_PAGESIZE + (uintptr_t)VM_ARENA_BASEADDR \
            + (uintptr_t)file_vpage_offset);
        // Check if working in valid arena space.
        if ((uintptr_t)readAddress >= (uintptr_t)curr_process->invalid_arena_start)
        {
            throw std::runtime_error("c-string malformed/out of bounds");
        }

        // Check infrastructure. 
        // If not read_enabled (i.e. not in phys_mem), bring swapbacked to physical memory.
        bool read_enabled = curr_process->original_pagetable.ptes[file_vpage_num].read_enable;
        if(!read_enabled)
        {
            // Convert page to virtual page address for vm_fault to bring into physmeme
            if(vm_fault(readAddress, false) == -1){
                throw std::runtime_error("c-string malformed/out of bounds");
            }
        }

        // Get start and end index of filename in physical memory
        unsigned int ppage = curr_process->original_pagetable.ptes[file_vpage_num].ppage;
        unsigned int physmem_index = ppage * VM_PAGESIZE + file_vpage_offset; // start
        unsigned int physmeme_pageend = ppage * VM_PAGESIZE + VM_PAGESIZE; // end

        // Start reading string in physical memory until end of page or '\0' found
        for(; physmem_index < physmeme_pageend; ++physmem_index)
        {
            //if null terminating character encountered, stop reading
            if(((char *)vm_physmem)[physmem_index] == '\0')
            {
                nullfound = true;
                break;
            }
            
            // Read one character into return string
            converted_filename.push_back(((char *)vm_physmem)[physmem_index]);
        }
        
        // Prepare loop to start reading next virtual page (if '\0' not found yet)
        file_vpage_num++; 
        file_vpage_offset = 0;

        if(!nullfound)
        {
            //std::cout << "DEBUG: Reading next page" << std::endl;
        }
    }

    return converted_filename;    

}


OS_page_entry* OS_Pager::get_virtual_owner(unsigned int phys_mem_index)
{
    assert(phys_mem_index < OS_Pager::physical_memory.size());
    assert(!OS_Pager::physical_memory[phys_mem_index].owners.empty());
    return *(OS_Pager::physical_memory[phys_mem_index].owners.begin());
}


void OS_Pager::evict_pages(){
    assert(OS_Pager::free_physmem_pages.empty());
    assert(!OS_Pager::clock.empty());
    
    // find the page we need to evict
    while(get_virtual_owner(OS_Pager::clock.front())->isReferenced){
        // update all owners reference bit
        unordered_set<OS_page_entry*>& owner_ref = OS_Pager::physical_memory[OS_Pager::clock.front()].owners;
        
        
        for(auto it = owner_ref.begin(); it != owner_ref.end(); ++it)
        {
            OS_page_entry& curr_entry = *(*it);
            // if update reference, must update read_enable and write_enable
            curr_entry.isReferenced = false;
            curr_entry.original_entry_ptr->read_enable = 0;
            curr_entry.original_entry_ptr->write_enable = 0;
        }

        OS_Pager::clock.push_back(OS_Pager::clock.front());
        OS_Pager::clock.pop_front();
    }
    unsigned evict_page_number = OS_Pager::clock.front();
    // update clock
    OS_Pager::clock.pop_front();
    
    // if this page is already in the swap space/file (disk) and is clean, only transfer ownership
    {
        unordered_set<OS_page_entry*>& owner_ref = OS_Pager::physical_memory[evict_page_number].owners;
        OS_page_entry* first_owner_ptr = *owner_ref.begin();
        if(!(first_owner_ptr->isDirty) && first_owner_ptr->swap_or_file_block_index != -1){
            // update page table
            for(auto it = owner_ref.begin(); it != owner_ref.end(); ++it){
                (*it)->isResident = false;
                // already set read_enable/write_enable
            }
            // update physical_memory data structure(move owner information)
                // for swap-backed pages
            if(first_owner_ptr->isSwapBack){
                OS_Pager::swap_blocks[first_owner_ptr->swap_or_file_block_index] = std::move(
                    OS_Pager::physical_memory[evict_page_number]);
            }
                // for file-backed pages
            else{
                OS_Pager::file_blocks[first_owner_ptr->file_name][first_owner_ptr->swap_or_file_block_index] = std::move(
                    OS_Pager::physical_memory[evict_page_number]);
                File_block_info candidate = {first_owner_ptr->file_name, \
                    (unsigned)first_owner_ptr->swap_or_file_block_index};
                assert(OS_Pager::file_blocks_in_physmem.find(candidate) != OS_Pager::file_blocks_in_physmem.end());
                OS_Pager::file_blocks_in_physmem.erase(candidate);
            }
            free_physmem_pages.insert(evict_page_number);
            return;
        }
    }

    // if this is a swap-backed page and is dirty
    if(get_virtual_owner(evict_page_number)->isSwapBack){
        // update free_physmem_pages and free_swap_block
        unsigned free_swap_block_number;

        // if it is already in the swap space
        if(get_virtual_owner(evict_page_number)->swap_or_file_block_index != -1){
            free_swap_block_number = get_virtual_owner(evict_page_number)->swap_or_file_block_index;
        }
        else{
            free_swap_block_number = *OS_Pager::free_swap_blocks.begin();
            OS_Pager::free_swap_blocks.erase(free_swap_block_number);
        }
        OS_Pager::free_physmem_pages.insert(evict_page_number);

        // update physical_memory and swap_blocks data structure
        OS_Pager::swap_blocks[free_swap_block_number] = std::move(
            OS_Pager::physical_memory[evict_page_number]);
        
        // update page table data structure
        // write data from physical memory to swap space
        // since this is a swap-backed page, there is only one owner
        auto it = OS_Pager::swap_blocks[free_swap_block_number].owners.begin();
        OS_page_entry& curr_entry = *(*it);
        file_write(nullptr, free_swap_block_number, OS_Pager::physmem_page_to_address(evict_page_number));
        
        curr_entry.isDirty = false;
        curr_entry.isResident = false;
        curr_entry.swap_or_file_block_index = free_swap_block_number;
        // already update the reference and read/write enable bits above
        
        return;
    }
    
    // if this is a file-backed page and is dirty
    else{
        unordered_set<OS_page_entry*>& owner_ref = OS_Pager::physical_memory[evict_page_number].owners;
        OS_page_entry* first_owner_ptr = *owner_ref.begin();
        // update evicting pages  
        for (auto it = owner_ref.begin(); it != owner_ref.end(); ++it) {
            (*it)->isResident = false;
            (*it)->isDirty = false;
        }
        // update file block info
        OS_Pager::file_blocks[first_owner_ptr->file_name][first_owner_ptr->swap_or_file_block_index] = std::move(
                    OS_Pager::physical_memory[evict_page_number]);
        File_block_info candidate = {first_owner_ptr->file_name, \
                    (unsigned)first_owner_ptr->swap_or_file_block_index};
        assert(OS_Pager::file_blocks_in_physmem.find(candidate) != OS_Pager::file_blocks_in_physmem.end());
        //erase candidate since it is no longer in the phys mem and free that space. 
        OS_Pager::file_blocks_in_physmem.erase(candidate);
       
        // write data to the file block
        file_write((first_owner_ptr->file_name).c_str(), first_owner_ptr->swap_or_file_block_index, OS_Pager::physmem_page_to_address(evict_page_number));
        
        OS_Pager::free_physmem_pages.insert(evict_page_number);
        return;
    }
    
}

Process::~Process(){
    for(OS_page_entry& each : OS_process_pagetable){
        // if this is a swap-backed page
        if(each.isSwapBack){
            // if it used swap space
            if(each.swap_or_file_block_index != -1){
                OS_Pager::swap_blocks[each.swap_or_file_block_index] = physical_page_t();
                OS_Pager::free_swap_blocks.insert(each.swap_or_file_block_index);
            }
            // if it used physical memory
            if(each.isResident){
                unsigned physical_page_number = each.original_entry_ptr->ppage;
                // if it is in the pinning page
                if(physical_page_number == OS_Pager::pinning_page_number){
                    OS_Pager::physical_memory[OS_Pager::pinning_page_number].owners.erase(&each);
                }
                // if it is in the other region of physical mem
                else{
                    OS_Pager::physical_memory[physical_page_number] = physical_page_t();
                    OS_Pager::free_physmem_pages.insert(physical_page_number);
                    
                    // clear the clock
                    list<unsigned>::iterator it = std::find(
                        OS_Pager::clock.begin(), OS_Pager::clock.end(), physical_page_number);
                    OS_Pager::clock.erase(it);
                }
            }
            --OS_Pager::total_swapbacked_pages;
        }
        // handle file-backed page
        else {
            File_block_info candidate = {each.file_name, \
            (unsigned)each.swap_or_file_block_index};
            // if page entry is in physical memory
            if (OS_Pager::file_blocks_in_physmem.find(candidate) != OS_Pager::file_blocks_in_physmem.end()) {
                unsigned physical_page_number = each.original_entry_ptr->ppage;
                OS_Pager::physical_memory[physical_page_number].owners.erase(&each);
            }
            else {
                // else we need to remove the page entry from file block
                (OS_Pager::file_blocks[each.file_name][each.swap_or_file_block_index]).owners.erase(&each);
            }
        }
    }
}


