#include "File_server.h"
#include "fs_server.h"
#include "fs_crypt.h"

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <exception>
#include <thread>
#include <memory>
#include <sstream>
#include <cassert>
#include <vector>

using namespace std;
using std::cout; // My IDE has issues w/o this for some reason -Kevin

constexpr unsigned ROOT_BLOCK = 0;
constexpr unsigned FS_SESSION = 0, FS_READBLOCK = 1, FS_WRITEBLOCK = 2, \
                   FS_CREATE = 3, FS_DELETE = 4;

ostream& operator<<(ostream& os, const File_server::Request& rhs){
    os << "Print Request: " << endl;
    os << "      username: " << rhs.username << endl;
    os << "      request type: " << rhs.request_type << endl;
    os << "      session: " << rhs.session << endl;
    os << "      sequence: " << rhs.sequence << endl;
    os << "      pathname: " << rhs.pathname << endl;
    os << "      block: " << rhs.block << endl;
    os << "      fs_create_type: " << rhs.fs_create_type << endl;
    os << "      fs_write_data_index: " << rhs.fs_write_data_index << endl;
    return os;
}

void File_server::search_inode(const fs_inode& inode, string& pathname)noexcept{
    if(inode.type == 'd'){
        fs_direntry table[FS_DIRENTRIES];
        for(unsigned i = 0; i < inode.size; ++i){
            free_blocks.erase(inode.blocks[i]);
            
            disk_readblock(inode.blocks[i], table);
            for(unsigned j = 0; j < FS_DIRENTRIES; ++j){
                // if this is an unused directory entry
                unsigned curr_inode_block = table[j].inode_block;
                if(curr_inode_block == 0) continue;
                
                free_blocks.erase(curr_inode_block);
                fs_inode entry_inode;
                disk_readblock(curr_inode_block, &entry_inode);
                
                pathname += table[j].name;
                blocks_locks[pathname];
                if(entry_inode.type == 'd') pathname += '/';
                search_inode(entry_inode, pathname);
                if(entry_inode.type == 'd') pathname.pop_back();
                pathname.erase(pathname.end() - strlen(table[j].name), pathname.end());
            }
        }
    }
    else{
        for(unsigned i = 0; i < inode.size; ++i){
            free_blocks.erase(inode.blocks[i]);
        }
    }
}

File_server::File_server(int port):next_session_number(0){
    string user, password;
    while(cin >> user >> password){
        users[user] = password;
    }
    for(unsigned i = 1; i < FS_DISKSIZE; ++i){
        free_blocks.insert(i);
    }

    // start to update free_blocks
    fs_inode root;
    disk_readblock(ROOT_BLOCK, &root);
    string pathname("/");
    blocks_locks[pathname];
    search_inode(root, pathname);
    
    sockaddr_in serv_addr;
    try{
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if(server_fd == -1) throw runtime_error("Opening socket fail");

        int enable = 1;
        if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1){
            throw runtime_error("Setting socket fail");
        }
        
        
        memset(&serv_addr, 0, sizeof(sockaddr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(port);

        if(bind(server_fd, (sockaddr*)&serv_addr, sizeof(sockaddr_in)) == -1){
            throw runtime_error("Binding fail");
        }
        
        socklen_t length = sizeof(sockaddr_in);
        getsockname(server_fd, (sockaddr*)&serv_addr, &length);

        if(listen(server_fd, 30) == -1){
            throw runtime_error("Listening fail");
        }
    }
    catch(const runtime_error e){
        close(server_fd);
        throw e;
    }

    cout << "\n@@@ port " << ntohs(serv_addr.sin_port) << endl;
}

void File_server::start() noexcept{
    while(true){
        sockaddr_in client_addr;
        socklen_t addr_len;
        int client_fd = accept(server_fd, (sockaddr*)&client_addr, &addr_len);
        if(client_fd == -1) continue;

        thread t(&File_server::serve_client, this, client_fd);
        t.detach();
    }
}

void File_server::send_helper(int client_fd, const std::string& message, const Request& request){
    string password;
    {
        std::lock_guard<mutex> raii_lock(file_server_lock);
        password = users[request.username];
    }
    
    unique_ptr<char[]> encrypted(new char[2 * message.size() + 64]);

    unsigned size_encrypted = fs_encrypt(password.c_str(), message.data(), message.size(), encrypted.get());
    
    string response_header = to_string(size_encrypted);
    response_header.push_back('\0');

    if(send(client_fd, response_header.data(), response_header.size(), MSG_NOSIGNAL) == -1){
        throw runtime_error("Sending failed");
    }
    if(send(client_fd, encrypted.get(), size_encrypted, MSG_NOSIGNAL) == -1){
        throw runtime_error("Sending failed");
    }

    close(client_fd);
}

void File_server::serve_client(int client_fd) noexcept{
    try{
        // receive clear text
        string username, password;
        unsigned encrypted_request_size;
        // throw if find errors when parse the clear text received
        parse_cleartext(client_fd, username, encrypted_request_size);
        // check if the username is valid
        {
            std::lock_guard<mutex> raii_lock(file_server_lock);
            if(!users.count(username)){
                throw runtime_error("Unknown username");
            }
            password = users[username];    
        }
        string request_buffer;
        Request request = parse_request(client_fd, request_buffer, encrypted_request_size, password, username);
        
        // cout << request << endl;

        switch(request.request_type){
            case FS_SESSION:{
                handle_fs_session(request);
            }break;
            case FS_READBLOCK:{
                handle_fs_read(request);
            }break;
            case FS_WRITEBLOCK:{
                handle_fs_write(request, request_buffer);
            }break;
            case FS_CREATE:{
                handle_fs_create(request);
            }break;
            case FS_DELETE:{
                handle_fs_delete(request);
            }break;
        }
    }
    catch(const exception& e){
        close(client_fd);
        cout_lock.lock();
        cerr << e.what() << endl;
        cout_lock.unlock();
        return;
    }
}

void File_server::check_session_sequence(const Request& request){
    {
        std::lock_guard<mutex> raii_lock(file_server_lock);
        if(!user_session_info.count(request.username) || 
            !user_session_info[request.username].count(request.session)){
            throw runtime_error("Unregistered user session");
        }
        unsigned& sequence_number = user_session_info[request.username][request.session];
        if(sequence_number >= request.sequence){
            throw runtime_error("Invalid sequence number");
        }
        sequence_number = request.sequence;
    }
}

// Zeyu
File_server::Request File_server::parse_request(int client_fd, string& request, 
                        unsigned encrypted_request_size, const string& password, const string& username){
    // receive the encrypted request
    unique_ptr<char[]> encrypted_msg(new char[encrypted_request_size]);
    if(recv(client_fd, encrypted_msg.get(), encrypted_request_size, MSG_WAITALL) == -1){
        throw runtime_error("receiving failed");
    }

    // decrypt the message
    // allocate one more char to deal with a malformed request without NULL
    string msg;
    msg.resize(encrypted_request_size);
    ssize_t request_size;
    if( (request_size = fs_decrypt(password.c_str(), encrypted_msg.get(), 
                                    encrypted_request_size, msg.data()) ) == -1){
        throw runtime_error("Can't decrypt the request message");
    }
    msg.erase(request_size);

    istringstream iss(msg);
    string request_type;
    if(!(iss >> request_type)){
        throw runtime_error("Invalid request message");
    }

    // check format, throw if find any error
    Request result;
    result.username = username;
    result.client_fd = client_fd;
    size_t location = msg.find_first_of('\0');
    if(location == string::npos) throw runtime_error("Invalid request");

    long long session = 0, sequence = 0, block = 0;
    iss >> session >> sequence;
    if(!iss || session < 0 || session > (long long)UINT32_MAX || sequence < 0 || sequence > (long long)UINT32_MAX){
        throw runtime_error("Incorrect parameter(session | sequence)");
    }

    if(request_type == "FS_SESSION"){
        if(!iss || session != 0){
            throw runtime_error("Incorrect parameter(FS_SESSION)");
        }
        result.request_type = FS_SESSION;
    }
    else if(request_type == "FS_READBLOCK"){
        iss >> result.pathname >> block;
        if(!iss || block < 0 || block > (long long)UINT32_MAX) throw runtime_error("Incorrect parameter(FS_READBLOCK)");
        result.request_type = FS_READBLOCK;
    }
    else if(request_type == "FS_WRITEBLOCK"){
        iss >> result.pathname >> block;
        if(!iss || block < 0 || block > (long long)UINT32_MAX){
            throw runtime_error("Incorrect parameter(FS_WRITEBLOCK)");
        }
        result.request_type = FS_WRITEBLOCK;
        result.fs_write_data_index = location+1;
    }
    else if(request_type == "FS_CREATE"){
        iss >> result.pathname >> result.fs_create_type;
        if(!iss || (result.fs_create_type != 'f' && result.fs_create_type != 'd') ){
            throw runtime_error("Incorrect parameter(FS_CREATE)");
        }
        result.request_type = FS_CREATE;
    }
    else if(request_type == "FS_DELETE"){
        iss >> result.pathname;
        result.pathname.pop_back();
        if(!iss || block < 0 || block > (long long)UINT32_MAX){
            throw runtime_error("Incorrect parameter(FS_DELETE)");
        }
        result.request_type = FS_DELETE;
    }
    else throw runtime_error("Invalid request");
    check_valid_pathname(result);
    result.block = (unsigned)block;
    result.sequence = (unsigned)sequence;
    result.session = (unsigned)session;

    // ensure the format is correct
    request = std::move(msg);
    return result;
}

// Kevin
void File_server::parse_cleartext(int client_fd, string& username, unsigned& size){
    
    // Recieve header <username><size><null> one byte at a time
    bool nullfound = false;
	char buffer[1];
    memset(buffer, 0, sizeof(buffer));

    string header;

    // Read until FS_MAXUSERNAME + (max size of 'size')(@2409)
	ssize_t bytes_recieved;
    unsigned MAXSIZESIZE = 5;

    while(header.size() <= FS_MAXUSERNAME + MAXSIZESIZE + 2)
    {
        bytes_recieved = recv(client_fd, buffer, 1, 0);
        header += string(buffer, bytes_recieved);

        if(buffer[0] == '\0')
        {
            nullfound = true;
            break; 
        }
    }
    
    // If null not found, invalid. Close connection without reply.
    if(!nullfound) throw std::runtime_error("Null not found in header");
    
    // Parse to username and size
    username = header.substr(0, header.find(' '));
    header.erase(0, header.find(' '));
    size = stoi(header); // this can throw error (caught above) 
    
    // Check valid username (in terms of length)
    if(username.size() > FS_MAXUSERNAME) throw runtime_error("username too long");
}

// Zeyu
void File_server::handle_fs_session(const Request& request){
    unsigned assigned_session;
    {
        lock_guard<mutex> raii_lock(file_server_lock);
        assigned_session = next_session_number++;
        user_session_info[request.username][assigned_session] = request.sequence;
    }
    string response = std::to_string(assigned_session) + ' ' + std::to_string(request.sequence);
    response.push_back('\0');
    send_helper(request.client_fd, response, request);
}

// Kevin
void File_server::handle_fs_read(const Request& request){
    check_session_sequence(request);

    if(request.block >= FS_MAXFILEBLOCKS)
    {
        throw runtime_error("handle_fs_read: Read block out of bounds (greater than max file size)");
    }
    
    fs_direntry file_parent_direntries[FS_DIRENTRIES];
    unsigned direntries_index; 

    fs_inode file_inode; 
    //unsigned file_block; // unused
    mutex* file_lock; // held after search_file_system 

    fs_inode file_parent_inode; // unused
    unsigned file_parent_block; // unused
    mutex* file_parent_lock; // held after search_file_system

    search_file_system(request, file_inode, file_parent_inode, file_parent_direntries,
                       direntries_index, file_parent_block, file_parent_lock, file_lock);

    // Check that file is indeed a file
    if(file_inode.type != 'f') 
    {
        file_lock->unlock();
        throw runtime_error("handle_fs_read: requested read is not a file");
    }

    // Check if request reads a block out of bounds (greater than size of file)
    if(request.block >= file_inode.size)
    {   
        file_lock->unlock();
        throw runtime_error("handle_fs_read: reading out of bounds");
    }
        
    // Create response string
    string response(std::to_string(request.session) + " " + std::to_string(request.sequence));
    response.push_back('\0');
    unsigned session_info_size = response.size();
    response.resize(response.size() + FS_BLOCKSIZE);

    // Read data directly into end of response string
    disk_readblock(file_inode.blocks[request.block], response.data() + session_info_size);

    file_lock->unlock();

    send_helper(request.client_fd, response, request);
}

// Naeun
void File_server::handle_fs_write(const Request& request, const string& request_buffer){
    //cout << "hanlde_fs_write start" << endl;
    check_session_sequence(request);
    // "/" path is invalid in search_file_system func so we need to validate this beforehand.
    if(request.pathname == "/"){
        throw runtime_error("Can't write to directory");
    }
    string req_str_substr = request_buffer.substr(request.fs_write_data_index);
    if (req_str_substr.length() != FS_BLOCKSIZE ) {
        throw runtime_error("String size has to be exactly equal to FS_BLOCKSIZE");
    }
    char req_str[FS_BLOCKSIZE];
    memcpy(req_str, req_str_substr.c_str(), FS_BLOCKSIZE); 
    if (request.block >= FS_MAXFILEBLOCKS) {
        throw runtime_error("Invalid write block");
    }
    mutex* last_lock = nullptr;
    mutex* this_lock = nullptr;
    fs_inode target_inode, last_inode;
    fs_direntry last_table[FS_DIRENTRIES];
    unsigned index_of_last_table;
    unsigned last_inode_block;
    unsigned target_inode_block = search_file_system(request, target_inode, last_inode, 
                                  last_table, index_of_last_table, last_inode_block, last_lock, this_lock);
    if(target_inode.type == 'd') {
        this_lock->unlock();
        throw runtime_error("Can't write to directory");
    }
    // write block may only refer to an existing block in the file or it may refer to the block immediately after the current end of the file. 
    if (request.block < 0 ||target_inode.size < request.block) {
        this_lock->unlock();
        throw runtime_error("Invalid write block");
    }
    // if write block refers to an existing block
    if (request.block < target_inode.size) {
        cout << request.block << endl;
        cout << target_inode.size;
        disk_writeblock(target_inode.blocks[request.block], &req_str);
    }
    // if write block refers to the block immediately after the eof
    else {
        unsigned free_block_index;
        {
            std::lock_guard<mutex> raii_lock(file_server_lock);
            // Check to make sure there is a free block available.
            if (free_blocks.empty()) {
                this_lock->unlock();
                throw runtime_error("No more free block");
            }
            free_block_index = *free_blocks.begin();
            free_blocks.erase(free_blocks.begin());
        }
        target_inode.blocks[target_inode.size] = free_block_index;
        target_inode.size++;
        disk_writeblock(target_inode.blocks[target_inode.size - 1], &req_str);
        disk_writeblock(target_inode_block, &target_inode);
    }
    this_lock->unlock();
    string response(std::to_string(request.session) + " " + std::to_string(request.sequence) + '\0');
    send_helper(request.client_fd, response, request);
}

// Kevin
void File_server::handle_fs_create(const Request& request){
    check_session_sequence(request);
    
    // Get path to immediate parent of requested new object // TODO: can be rewritten to be less complicated
    vector<string> parsed_path = parse_path(request.pathname);
    string new_objname = parsed_path.back();
    parsed_path.pop_back();

    if(new_objname.size() > FS_MAXFILENAME) throw runtime_error("handle_fs_create: malformed path"); 

    string parentpath;
    {
        for (auto pathobj : parsed_path)
        {
            parentpath += "/";
            parentpath += pathobj;
        }
        if (parsed_path.size() == 0) parentpath += "/";
    }
    
    // DEPRECATED
    /*
    unsigned parent_inode_block = get_inodeblock(parentpath, request.username);

    // Get parent_inode
    fs_inode parent_inode;

    // read in inode, and lock parent of new file/dir
    mutex* parent_lock = nullptr;
    {
        std::lock_guard<mutex> raii_lock(file_server_lock);
        parent_lock = &blocks_locks[parentpath];
    }

    if(parent_lock == nullptr) throw runtime_error("handle_fs_create: Directory no longer exists");
    std::lock_guard<mutex> raii_lock(*parent_lock);
    disk_readblock(parent_inode_block, &parent_inode);

    // Check parent_inode is not a file
    if(parent_inode.type == 'f') throw runtime_error("handle_fs_create: Invalid directory (file treated as directory)");

    // Check permissions of parent_inode
    if (parent_inode.owner != request.username && parentpath != "/") 
        throw runtime_error("handle_fs_create: Invalid user permissions");
    */

    
    // Create custom request for search_file_system
    Request parentrequest;
    parentrequest = request;
    parentrequest.pathname = parentpath;

    // I.e. creating 'note_1' in /482/F19/notes/note_1, (passing in /482/F19/notes/)
    fs_direntry grandparent_direntries[FS_DIRENTRIES]; // eight of /482/F19 direntries             (unused)
    unsigned direntries_index;                         // index of 'notes' direntry in table above (unused)

    fs_inode parent_inode;                             // inode of /482/F19/notes
    unsigned parent_inode_block;                       // block of /482/F19/notes
    mutex* parent_lock;                                // held lock of /482/F19/notes 

    fs_inode grandparent_inode;                        // inode of /482/F19         (unused)
    unsigned grandparent_block;                        // block number of /482/F19  (unused)
    mutex* grandparent_lock;                           // held lock of /482/F19     (unlock immediately)

    // search_file_system cannot be used with root
    if(parentpath == "/")
    {
        {
            std::lock_guard<mutex> raii_lock(file_server_lock);
            parent_lock = &blocks_locks["/"];
        }

        parent_inode_block = 0;
        parent_lock->lock();
        disk_readblock(0, &parent_inode);
    }
    else
    {
        parent_inode_block = search_file_system(parentrequest, parent_inode, grandparent_inode, grandparent_direntries,
                                             direntries_index, grandparent_block, grandparent_lock, parent_lock);

    }
    
    // Check parent_inode is not a file
    if(parent_inode.type == 'f') {
        parent_lock->unlock();
        throw runtime_error("handle_fs_create: Invalid directory (file treated as directory)");
    }

    // Check permissions of parent_inode
    if (parent_inode.owner != request.username && parentpath != "/") {
        parent_lock->unlock();
        throw runtime_error("handle_fs_create: Invalid user permissions");
    }

    // Find unused directory location for new_direntry in parent_inode
    fs_direntry block_direntries[FS_DIRENTRIES]; // direntry array where unused direntry resides
    unsigned new_direntry_block = 0;            // index of block_direntries in inode blocks array (range: < inode.size)
    unsigned new_direntry_block_index = 0;      // index of unused direntry in block_direntries (range: < FS_DIRENTRIES)
    bool index_found = false;
    
    fs_direntry tmp_block_direntries[FS_DIRENTRIES]; // temporary read buffer for searching through direntries

    // Iterate through ALL direntries to find unused direntry and possible duplicate
    for (unsigned i = 0; i < parent_inode.size; ++i)
    {
        disk_readblock(parent_inode.blocks[i], tmp_block_direntries);
        for (unsigned j = 0; j < FS_DIRENTRIES; ++j)
        {
            // Unused direntry found
            if((tmp_block_direntries[j].inode_block == 0) && !index_found)
            {
                new_direntry_block = parent_inode.blocks[i]; // Block of 8 direntries
                new_direntry_block_index = j;                // index in the 8 direntries
                index_found = true;

                // Grab copy of direentry block where unused direntry is found
                for(unsigned k = 0; k < FS_DIRENTRIES; ++k)
                {
                    block_direntries[k].inode_block = tmp_block_direntries[k].inode_block;
                    strcpy(block_direntries[k].name, tmp_block_direntries[k].name);
                }
            }

            // Check if this is a duplicate direntry name.
            if( tmp_block_direntries[j].inode_block != 0)
            {   
                int cmp_result = strcmp(tmp_block_direntries[j].name, new_objname.c_str());
                if(cmp_result == 0) 
                {
                    parent_lock->unlock();
                    throw runtime_error("handle_fs_create: Duplicate file/directory found");
                }
            }
        }
    }

    // No unused dir_entry found. Must increase size of parent_inode to fit more directories
    if(!index_found)
    {
        // Change local version of parent_inode (for writing later)
        parent_inode.size++;
        if(parent_inode.size > FS_MAXFILEBLOCKS) 
        {
            parent_lock->unlock();
            throw runtime_error("handle_fs_create: no more free space in directory");
        }

        // Find a free block for the new dir_entry array
        unsigned new_direntries_block = 0;
        {
            std::lock_guard<mutex> raii_lock(file_server_lock);
            if(free_blocks.size() == 0) 
            {
                parent_lock->unlock();
                throw runtime_error("handle_fs_create: no more free blocks");
            }
            new_direntries_block = *free_blocks.begin();
            free_blocks.erase(free_blocks.begin()); 
        }
        parent_inode.blocks[parent_inode.size - 1] = new_direntries_block; 

        // Update locations
        new_direntry_block = new_direntries_block;
        new_direntry_block_index = 0;

        // Initialize new array to be all unused
        for(unsigned i = 0; i < FS_DIRENTRIES; ++i)
        {
            block_direntries[i].inode_block = 0;
            //memset(block_direntries[0].name, 0, 60);
        }
    }

    //@2641 Autograder will accept finding free block after traversal.

    // Create new direntry for parent
    fs_direntry new_direntry;    
    strcpy(new_direntry.name, new_objname.c_str());

    // Create inode of new file/directory
    fs_inode new_inode;
    if (request.fs_create_type == 'd') new_inode.type = 'd';
    if (request.fs_create_type == 'f') new_inode.type = 'f';
    strcpy(new_inode.owner, request.username.c_str());
    new_inode.size = 0;

    // Find a free block for the new inode (no specific block required @2523)
    unsigned new_inode_block = 0;
    {
        std::lock_guard<mutex> raii_lock(file_server_lock);
        if(free_blocks.size() == 0) 
        {
            parent_lock->unlock();
            throw runtime_error("handle_fs_create: no more free blocks");
        }
        new_inode_block = *free_blocks.begin();
        free_blocks.erase(free_blocks.begin()); 
    }
    
    // Assign free block to inode
    new_direntry.inode_block = new_inode_block;

    // Insert new_direntry into local version of one of parents dir_entry blocks
    block_direntries[new_direntry_block_index] = new_direntry;

    // Write in order that allows consistency through crash
    disk_writeblock(new_inode_block, &new_inode);           // write new file/dir inode
    disk_writeblock(new_direntry_block, &block_direntries); // add to dir_entries of parent inode
    if(!index_found)
    {
        // Update parent inode if size changed
        disk_writeblock(parent_inode_block, &parent_inode);
    }

    // Update in-memory structure
    {
        std::lock_guard<mutex> raii_lock(file_server_lock);
        blocks_locks[request.pathname];
    }

    parent_lock->unlock();

    string response(std::to_string(request.session) + " " + std::to_string(request.sequence));
    response.push_back('\0');
    send_helper(request.client_fd, response, request);
}

// Zeyu
void File_server::handle_fs_delete(const Request& request){
    check_session_sequence(request);
    if(request.pathname == "/"){
        throw runtime_error("Can't delete root inode");
    }
    mutex* last_lock = nullptr;
    mutex* this_lock = nullptr;
    fs_inode target_inode, last_inode;
    fs_direntry last_table[FS_DIRENTRIES];
    unsigned index_of_last_table;
    unsigned last_inode_block;
    unsigned target_inode_block = search_file_system(request, target_inode, last_inode, 
                                  last_table, index_of_last_table, last_inode_block, last_lock, this_lock);
    
    if(target_inode.type == 'd' && target_inode.size != 0){
        this_lock->unlock();
        last_lock->unlock();
        throw runtime_error("Can't delete a non-empty directory");
    }
    
    string target_name(request.pathname.substr(request.pathname.find_last_of('/')+1));
    unsigned entry_number = 0;
    // update the directory entry table of last-level directory
    for(unsigned i = 0; i < FS_DIRENTRIES; ++i){
        if(last_table[i].inode_block == ROOT_BLOCK) continue;

        ++entry_number;
        if(target_name == last_table[i].name){
            last_table[i].inode_block = ROOT_BLOCK;
            memset(last_table[i].name, 0, FS_MAXFILENAME+1);
        }
    }

    {
        std::lock_guard<mutex> raii_lock(file_server_lock);
        free_blocks.insert(target_inode_block);
        if(target_inode.type == 'f'){
            for(unsigned i = 0; i < target_inode.size; ++i){
                free_blocks.insert(target_inode.blocks[i]);
            }
        }
        this_lock->unlock(); this_lock = nullptr;
        blocks_locks.erase(request.pathname);
        
        // if this table becomes empty
        if(entry_number == 1){
            free_blocks.insert(last_inode.blocks[index_of_last_table]);
            // shift all following entry by one
            for(unsigned i = index_of_last_table+1; i < last_inode.size; ++i){
                last_inode.blocks[i-1] = last_inode.blocks[i];
            }
            --last_inode.size;   
        }
    }

    if(entry_number == 1){
        disk_writeblock(last_inode_block, &last_inode);
    }
    else{
        // last inode didn't change, so don't need to write to disk
        disk_writeblock(last_inode.blocks[index_of_last_table], last_table);
    }
    last_lock->unlock(); last_lock = nullptr;
    
    string response(std::to_string(request.session) + " " + std::to_string(request.sequence) + '\0');
    send_helper(request.client_fd, response, request);
}

unsigned File_server::search_file_system(const Request& request, fs_inode& inode, fs_inode& last_inode, fs_direntry* table,
                                unsigned& index_of_table, unsigned& last_inode_block, std::mutex*& last, std::mutex*& this_lock){
    if(request.pathname[0] != '/') throw runtime_error("Invalid pathname");
    string curr_path("/");
    unsigned path_index = 1, curr_block = ROOT_BLOCK;
    mutex* curr_lock = nullptr;
    mutex* last_lock = nullptr;
    while(true){
        // get the directory / file name
        bool end_of_path = false;
        size_t next_slash = request.pathname.find_first_of('/', path_index);
        string curr_name(request.pathname.substr(path_index, next_slash - path_index));
        if(next_slash == string::npos) end_of_path = true;

        // read in inode
        {
            std::lock_guard<mutex> raii_lock(file_server_lock);
            curr_lock = &blocks_locks[curr_path];
        }
        curr_lock->lock();
        if(last_lock != nullptr) last_lock->unlock();
        if(curr_path != "/") curr_path.push_back('/');
        disk_readblock(curr_block, &last_inode);
        last_inode_block = curr_block;
        
        // since this inode should be a dir inode, if it's a file inode, it means pathname likes "/file1/***"
        if(last_inode.type == 'f'){
            curr_lock->unlock();
            throw runtime_error("Invalid pathname: /filename/**");
        }
        
        // check ownership
        if(curr_block != ROOT_BLOCK && request.username != last_inode.owner){
            curr_lock->unlock();
            throw runtime_error("Invalid pathname: pathname includes other's file/directory");
        }

        // search if this directory contains the required file
        unsigned target_inode_block = 0;
        for(unsigned i = 0; i < last_inode.size; ++i){
            disk_readblock(last_inode.blocks[i], table);
            for(unsigned j = 0; j < FS_DIRENTRIES; ++j){
                if(table[j].inode_block == ROOT_BLOCK) continue;
                string name(table[j].name);
                if(curr_name == name){
                    index_of_table = i;
                    target_inode_block = table[j].inode_block;
                    break;
                }
            }
            if(target_inode_block != 0) break;
        }
        // if can't find the file/directory
        if(target_inode_block == 0){
            curr_lock->unlock();
            throw runtime_error("Invalid pathname: Can't find file/directory");
        }
        // if we find the file/directory and reach the end of pathname
        if(end_of_path) {
            {
                std::lock_guard<mutex> raii_lock(file_server_lock);
                this_lock = &blocks_locks[curr_path + curr_name];
            }
            this_lock->lock();
            if(request.request_type != FS_DELETE){
                curr_lock->unlock();
                curr_lock = nullptr;
            }
            disk_readblock(target_inode_block, &inode);
            // check the owner
            if(request.username != inode.owner){
                this_lock->unlock();
                if(request.request_type == FS_DELETE) curr_lock->unlock();
                throw runtime_error("Invalid pathname: pathname includes other's file/directory");
            }
            last = curr_lock;
            return target_inode_block;
        }
        // if we find the file/directory and it's not the end
        curr_path += curr_name;
        path_index = curr_path.size()+1;
        curr_block = target_inode_block;
        last_lock = curr_lock;
        curr_lock = nullptr;
    }
}


// parse a pathname into vector of object components
vector<string> File_server::parse_path(const std::string &pathname)
{
    std::vector<string> parsed_path;

    // Parse paths
    std::string delimiter = "/";
    size_t last = 0; 
    size_t next = 0; 
    while ((next = pathname.find(delimiter, last)) != string::npos) 
    { 
        string nextobj = pathname.substr(last, next-last);
        parsed_path.push_back(nextobj);
        last = next + 1; 
    }
    string nextobj = pathname.substr(last, next-last);
    parsed_path.erase(parsed_path.begin()); // Remove empty element at start
    parsed_path.push_back(nextobj);
    
    return parsed_path;
}


void File_server::check_valid_pathname(const Request& request)
{
    // don't check for fs_session
    if(request.request_type == FS_SESSION) return;
    // Spec section 1
    if(request.pathname.size() > FS_MAXPATHNAME) throw runtime_error("pathname too long");
    if(request.pathname[0] != '/') throw runtime_error("Beginning '/' not found in pathname");
    if(request.pathname.back() == '/') throw runtime_error("pathname ends with '/'");

    // Spec section 5
    if (request.pathname.find(' ') != std::string::npos) throw runtime_error("pathname contains whitespace");
    if (request.pathname.find('\0') != std::string::npos) throw runtime_error("pathname contains null");
    
    // Check valid lengths (see FS_MAXFILENAME in fs_param.h and section 5)
    vector<string> parsed_path = parse_path(request.pathname);
    for (unsigned i = 0; i < parsed_path.size(); ++i)
    {
        if(parsed_path[i].size() > FS_MAXFILENAME)
        {
            throw runtime_error("parsing error: pathname contains an object whose name is too long");
        }

        // @2606 i.e. "/dir//file" is invalid. 
        if(parsed_path[i].empty())
        {
            throw runtime_error("parsing error: pathname contains an object whose name is empty");
        }
    }
}
