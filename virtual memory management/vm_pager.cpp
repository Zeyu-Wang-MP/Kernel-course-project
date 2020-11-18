/*
 * vm_pager.cpp
 * Function declarations for vm_pager.cpp
 */

#include "vm_pager.h"
#include "vm_OS.h"
#include <cstring>
#include <cassert>
#include <iostream>
#include <unordered_map>
#include <string>

using std::string;
using std::pair;
using std::unordered_set;


void vm_init(unsigned int memory_pages, unsigned int swap_blocks)
{
    OS_Pager::physical_memory.resize(memory_pages);
    OS_Pager::swap_blocks.resize(swap_blocks);

    // all physical pages(except the last pinning page) and swap blocks are free to use
    for(unsigned i = 0; i < memory_pages-1; ++i){
        OS_Pager::free_physmem_pages.insert(i);
    }
    for(unsigned i = 0; i < swap_blocks; ++i){
        OS_Pager::free_swap_blocks.insert(i);
    }
    // Set the last page in physical memory as the pinned page
    memset(OS_Pager::physmem_page_to_address(memory_pages-1), 0, VM_PAGESIZE);
    OS_Pager::pinning_page_number = memory_pages-1;
}

int vm_create(pid_t parent_pid, pid_t child_pid) 
{
    OS_Pager::processes[child_pid] = new Process();
    return 0;
}

void vm_switch(pid_t pid)
{
    OS_Pager::curr_process = pid;
    page_table_base_register = &OS_Pager::processes[pid]->original_pagetable;
}

void* vm_map(const char *filename, unsigned int block)
{   // for swap-backed pages
    if(filename == nullptr){
        // first case: we can add this page, don't need to evict other page in physical mem
        // since we always share this new page with the pinning page(copy-on-write)
        if(OS_Pager::total_swapbacked_pages < OS_Pager::swap_blocks.size()){
            Process& curr_process = *OS_Pager::processes[OS_Pager::curr_process];
            void* res = curr_process.invalid_arena_start;
            assert(curr_process.OS_process_pagetable.size() <= VM_ARENA_SIZE / VM_PAGESIZE);
            // check if we already run out of virtual addr space
            if((uintptr_t)res == (uintptr_t)VM_ARENA_BASEADDR + VM_ARENA_SIZE) return nullptr;


            unsigned virtual_page_number = OS_Pager::virtual_address_to_page(res);
            ++OS_Pager::total_swapbacked_pages;
            
            // set original page table
            curr_process.original_pagetable.ptes[virtual_page_number].ppage = OS_Pager::pinning_page_number;
            curr_process.original_pagetable.ptes[virtual_page_number].read_enable = 1;
            curr_process.original_pagetable.ptes[virtual_page_number].write_enable = 0;

            // set OS data structure page table
            curr_process.OS_process_pagetable.push_back(
                OS_page_entry(&curr_process.original_pagetable.ptes[virtual_page_number], true, -1, ""));
            curr_process.invalid_arena_start = (void*)((uintptr_t)curr_process.invalid_arena_start + VM_PAGESIZE);
            
            // set OS physical memory data structure(add owner)
            OS_Pager::physical_memory[OS_Pager::pinning_page_number].owners.insert(
                &curr_process.OS_process_pagetable.back());
            
            //OS_Pager::check_consistent();
            return res;
        }
        // second case: we can't add this page since eager swap reservation
        else return nullptr;
    }

    // for file-backed pages
    else{
        // Read filename c-string
        std::string _filename;
        try
        {
            _filename = OS_Pager::read_c_string(filename);
        }
        catch(const std::runtime_error& e)
        {
            //std::cerr << e.what() << '\n';
            return nullptr;
        }

        // Declare variables
        Process& curr_process = *OS_Pager::processes[OS_Pager::curr_process];
        void* new_valid_arena_start = curr_process.invalid_arena_start;

        // Check if there is still enough virtual address space 
        if((uintptr_t)new_valid_arena_start >= (uintptr_t)VM_ARENA_BASEADDR + VM_ARENA_SIZE) return nullptr;
        // Update new valid arena 
        curr_process.invalid_arena_start = (void*)((uintptr_t)curr_process.invalid_arena_start + VM_PAGESIZE);
        
        // Check if already in physical memory
        bool inphysmem = false;
        unsigned phys_index;
        OS_page_entry* phys_owner = nullptr;
        File_block_info candidate = {_filename, block};
        if(OS_Pager::file_blocks_in_physmem.find(candidate) != OS_Pager::file_blocks_in_physmem.end()){
            inphysmem = true;
            phys_index = OS_Pager::file_blocks_in_physmem[candidate];
            phys_owner = OS_Pager::get_virtual_owner(phys_index);
        }
        

        unsigned virtual_page_number = OS_Pager::virtual_address_to_page(new_valid_arena_start);
        if(!inphysmem)
        {
            // Add entry to infrastructure page table (original_pagetable)
            curr_process.original_pagetable.ptes[virtual_page_number].ppage = 0; // Default to 0. Assigned in vm_fault()
            curr_process.original_pagetable.ptes[virtual_page_number].read_enable = 0;
            curr_process.original_pagetable.ptes[virtual_page_number].write_enable = 0;

            // Add entry to OS virtual page table
            curr_process.OS_process_pagetable.push_back(
                    OS_page_entry(&curr_process.original_pagetable.ptes[virtual_page_number], false, block, _filename));
            curr_process.OS_process_pagetable.back().isDirty = false;
            curr_process.OS_process_pagetable.back().isResident = false; // Make resident in vm_fault()
            curr_process.OS_process_pagetable.back().isReferenced = false;


            if(OS_Pager::file_blocks[_filename].size() <= block) 
            {
                OS_Pager::file_blocks[_filename].resize(block + 1);    
            }

            // if this is the first time we meet this {file, block}
            File_block_info curr_block = {_filename, block};
            if(OS_Pager::file_blocks_seen.find(curr_block) == OS_Pager::file_blocks_seen.end()){
                OS_Pager::file_blocks_seen.insert(curr_block);
                
                OS_page_entry* self_owner = new OS_page_entry(new page_table_entry_t, false, block, _filename);
                self_owner->original_entry_ptr->ppage = 0;
                self_owner->original_entry_ptr->read_enable = 0;
                self_owner->original_entry_ptr->write_enable = 0;
                self_owner->isDirty = false;
                self_owner->isReferenced = false;
                self_owner->isResident = false;
                OS_Pager::file_blocks[_filename][block].owners.insert(self_owner);
            }

            // Add an owner to the corresponding file block (start off with page in disk, and only evict in vm_fault())
            OS_Pager::file_blocks[_filename][block].owners.insert(&curr_process.OS_process_pagetable.back());
        }
        else
        {
            // Since it is already in physical memory, bits should match its fellow virtual owners.
            // and then update other bits it needed

            // TODO: Consider this map a reference. Change below code.
            // Add entry to infrastructure page table (original_pagetable).
            curr_process.original_pagetable.ptes[virtual_page_number].ppage = phys_index; // already in physmem
            curr_process.original_pagetable.ptes[virtual_page_number].read_enable = phys_owner->original_entry_ptr->read_enable;
            curr_process.original_pagetable.ptes[virtual_page_number].write_enable = phys_owner->original_entry_ptr->write_enable;

            // Add entry to OS virtual page table
            curr_process.OS_process_pagetable.push_back(
                    OS_page_entry(&curr_process.original_pagetable.ptes[virtual_page_number], false, block, _filename));
            curr_process.OS_process_pagetable.back().isDirty = phys_owner->isDirty;
            curr_process.OS_process_pagetable.back().isResident = phys_owner->isResident;
            curr_process.OS_process_pagetable.back().isReferenced = phys_owner->isReferenced;
            
            // Add an owner to the corresponding phys block (already in physmem, so add owner)
            OS_Pager::physical_memory[phys_index].owners.insert(&curr_process.OS_process_pagetable.back());
        }

        //OS_Pager::check_consistent();
        return new_valid_arena_start;
    }

}

int vm_fault(const void* addr, bool write_flag)
{
    //OS_Pager::check_consistent();
    Process& curr_process = *OS_Pager::processes[OS_Pager::curr_process];
    unsigned virtual_page_number = OS_Pager::virtual_address_to_page(addr);
    
    // handle user access an invalid page
    if(virtual_page_number >= curr_process.OS_process_pagetable.size()) return -1;

    OS_page_entry& curr_entry = curr_process.OS_process_pagetable[virtual_page_number];
    
    // handle user (read/write an unreferenced page) || (write a clean page) in physical memory
    if(curr_entry.isResident){
        // find the physical page number 
        unsigned physical_page_number = curr_entry.original_entry_ptr->ppage;
        unordered_set<OS_page_entry*>& owner_ref = OS_Pager::physical_memory[physical_page_number].owners;
        
        // if the user read/write an unreferenced page in physical memory
        if(!curr_entry.isReferenced){
            for(auto it = owner_ref.begin(); it != owner_ref.end(); ++it){
                if(write_flag){
                    (*it)->isDirty = true;
                }
                
                (*it)->isReferenced = true;
                (*it)->original_entry_ptr->read_enable = 1;
                (*it)->original_entry_ptr->write_enable = (*it)->isDirty ? 1 : 0;
            }
            return 0;
        }

        // if the user write a clean page in physical memory
        if(curr_entry.isReferenced && !curr_entry.isDirty && write_flag){
            for(auto it = owner_ref.begin(); it != owner_ref.end(); ++it){
                (*it)->isDirty = true;
                (*it)->original_entry_ptr->write_enable = 1;
            }
            return 0;
        }
    }

    // handle user (write a new created swap-backed page) || (read/write a non-resident swap-backed page)
    if(curr_entry.isSwapBack){
        // if the user tries to write the created page, need to copy it from the pinning 
        // page to a new page(may need to evict page)
        if(curr_entry.isDirty && curr_entry.isReferenced && curr_entry.isResident
            && curr_entry.original_entry_ptr->read_enable == 1){
            // if we need to evict pages
            if(OS_Pager::free_physmem_pages.empty()){
                OS_Pager::evict_pages();
            }
            assert(!OS_Pager::free_physmem_pages.empty());
            // now we have free physical mem page
            unsigned free_page_number = *OS_Pager::free_physmem_pages.begin();
            OS_Pager::free_physmem_pages.erase(free_page_number);

            // copy the content to the new free page in physical mem
            memcpy(OS_Pager::physmem_page_to_address(free_page_number), 
                OS_Pager::physmem_page_to_address(OS_Pager::pinning_page_number), VM_PAGESIZE);
            
            // update clock
            OS_Pager::clock.push_back(free_page_number);
            
            // update physical memory data structure
              // update owner of pinning page
            OS_Pager::physical_memory[OS_Pager::pinning_page_number].owners.erase(&curr_entry);
              // update owner of free page
            assert(OS_Pager::physical_memory[free_page_number].owners.empty());
            OS_Pager::physical_memory[free_page_number].owners.insert(&curr_entry);
            
            // update original page table
            curr_entry.original_entry_ptr->ppage = free_page_number;
            curr_entry.original_entry_ptr->write_enable = 1;

            // don't need to update OS page table data structure
            return 0;
        }

        // if the user tries to read/write a non-resident page
        if(!curr_entry.isResident){
            if(OS_Pager::free_physmem_pages.empty()){
                OS_Pager::evict_pages();
            }
            assert(!OS_Pager::free_physmem_pages.empty());
            unsigned free_page_number = *OS_Pager::free_physmem_pages.begin();
            OS_Pager::free_physmem_pages.erase(free_page_number);

            // copy content from swap space to memory
            int readstatus = file_read(nullptr, curr_entry.swap_or_file_block_index, 
                OS_Pager::physmem_page_to_address(free_page_number));
            if(readstatus == -1) return -1;
            
            // update clock
            OS_Pager::clock.push_back(free_page_number);
            
            // update physical memory data structure
            assert(OS_Pager::physical_memory[free_page_number].owners.empty());
            OS_Pager::physical_memory[free_page_number] = std::move(
                OS_Pager::swap_blocks[curr_entry.swap_or_file_block_index]);
            
            //update page table
            curr_entry.isResident = true;
            curr_entry.isReferenced = true;
            if(write_flag) curr_entry.isDirty = true;
            curr_entry.original_entry_ptr->ppage = free_page_number;
            curr_entry.original_entry_ptr->read_enable = 1;
            curr_entry.original_entry_ptr->write_enable = curr_entry.isDirty ? 1 : 0;
            //OS_Pager::check_consistent();
            return 0;
        }
        
    }

    // TODO: handle user read/write a non-resident file-backed page
    else{
        // Get physical page number in phys_mem, make resident.
        unsigned phys_mem_index; 
        
        // If there are no free pages in physical memory, evict to get a free page.
        unsigned int free_physmem_index;
        if(OS_Pager::free_physmem_pages.empty())
        {
            OS_Pager::evict_pages();
        }
        
        // Allocate a free page in physical memory. 
        free_physmem_index = *OS_Pager::free_physmem_pages.begin();
        // Read into vm_physmem
        void* vm_physmem_addr = OS_Pager::physmem_page_to_address(free_physmem_index);
        if (file_read(curr_entry.file_name.c_str(), curr_entry.swap_or_file_block_index, vm_physmem_addr) == -1) return -1;
        
        
        OS_Pager::free_physmem_pages.erase(free_physmem_index);

        //update clock
        OS_Pager::clock.push_back(free_physmem_index);

        // Move ownership from file (disk) to physical memory
        OS_Pager::physical_memory[free_physmem_index] = std::move(OS_Pager::file_blocks[curr_entry.file_name][curr_entry.swap_or_file_block_index]);

        // since we add a file block to physical memory
        File_block_info new_block = {curr_entry.file_name, (unsigned)curr_entry.swap_or_file_block_index};
        assert(OS_Pager::file_blocks_in_physmem.find(new_block) == OS_Pager::file_blocks_in_physmem.end());
        OS_Pager::file_blocks_in_physmem[new_block] = free_physmem_index;

        // Update bits for all virtual owners
        {
            unordered_set<OS_page_entry*>& owners = OS_Pager::physical_memory[free_physmem_index].owners;
            for (auto iter = owners.begin(); iter != owners.end(); ++iter)
            {
                (*iter)->isResident = true;
                (*iter)->original_entry_ptr->ppage = free_physmem_index;
            }
        }
        phys_mem_index = free_physmem_index;
        
        //TODO: on user read
        if(!write_flag)
        {
            // Update bits for all owners
            unordered_set<OS_page_entry*>& owners = OS_Pager::physical_memory[phys_mem_index].owners;
            for (auto iter = owners.begin(); iter != owners.end(); ++iter)
            {
                (*iter)->isReferenced = true;
                (*iter)->original_entry_ptr->read_enable = 1;
            } 
        }

        //TODO: on user write
        if(write_flag)
        {
            unordered_set<OS_page_entry*>& owners = OS_Pager::physical_memory[phys_mem_index].owners;
            for (auto iter = owners.begin(); iter != owners.end(); ++iter)
            {
                (*iter)->isReferenced = true;
                (*iter)->isDirty = true;
                (*iter)->original_entry_ptr->read_enable = 1;
                (*iter)->original_entry_ptr->write_enable = 1;
            }
        }
        
        //OS_Pager::check_consistent();
        return 0;
    }
    // never get there put here for compile
    return 0;
}

void vm_destroy(){
    
    delete OS_Pager::processes[OS_Pager::curr_process];
    //OS_Pager::check_consistent();
    OS_Pager::processes.erase(OS_Pager::curr_process);
}

