#ifndef _FILE_SERVER_H
#define _FILE_SERVER_H

#include "fs_server.h"

#include <unordered_map>
#include <string>
#include <unordered_set>
#include <mutex>
#include <memory>
#include <map>
#include <iostream>
#include <vector>

extern std::mutex cout_lock;

class File_server{
private:
    struct Request{
        int client_fd;
        std::string username;
        unsigned request_type; 
        unsigned session;
        unsigned sequence;
        std::string pathname;
        unsigned block;
        char fs_create_type;
        unsigned fs_write_data_index;
    };
    // use this lock when you need to change any data structure of this file_server
    // don't use it when do blocks_locks[pathname].lock();
    std::mutex file_server_lock;

    // username -> password
    std::unordered_map<std::string, std::string> users;
    std::unordered_set<unsigned> free_blocks;
    
    // pathname -> lock 
    std::map<std::string, std::mutex> blocks_locks;
    
    // username -> all sessions of this user
    // session_number -> sequence number for this session
    std::unordered_map<std::string, std::unordered_map<unsigned, unsigned>> user_session_info;
    // next session number to assign, initially 0
    unsigned next_session_number;
    
    int server_fd;
    



    // helper function to initialize the free_blocks
    // not thread safe
    void search_inode(const fs_inode& inode, std::string& pathname) noexcept;
    
    // start function for each thread
    void serve_client(int client_fd) noexcept;
    
    // throw if any error occur
    void handle_fs_session(const Request& request);
    void handle_fs_read(const Request& request);
    void handle_fs_write(const Request& request, const std::string& request_buffer);
    void handle_fs_create(const Request& request);
    void handle_fs_delete(const Request& request);

    // throw if session/sequence is invalid
    // if session/sequence is valid, we update(use up) the sequence
    void check_session_sequence(const Request& request);

    // helper function to search the pathname in the request, don't use it when the target pathname is "/" !
    // e.g. if pathname is "/482/F19/notes", this function will return the inode block number of "/482/F19/notes"
    //     Also, the lock of "/482/F19" will be holded by <last> (only for fs_delete, for other request, <last> will be nullptr)
    //         and the lock of "/482/F19/notes" will be holded by <this_lock>
    //     The inode of "/482/F19" will be in <last_inode>, the inode of "/482/F19/notes" will be in <inode>
    //     The "/482/F19"'s dir table including "/482/F19/notes" entry will be in <table>, 
    //         and the index of this table will be in <index_of_table>
    //     The <last_inode_block> will contain the inode block number of "/482/F19" 
    // throw if the pathname is invalid/contains other's directory/file, 
    //     when this function throws, no lock will be holded, but you shouldn't use any parameters anymore
    unsigned search_file_system(const Request& request, fs_inode& inode, fs_inode& last_inode, fs_direntry* table,
                                unsigned& index_of_table, unsigned& last_inode_block, std::mutex*& last, std::mutex*& this_lock);


    // receive the clear text
    // throw if receiving failed or we get an invalid cleartext
    static void parse_cleartext(int client_fd, std::string& username, unsigned& size);
    
    // receive the request 
    // throw if receiving failed or we get an malformed request
    // only check the format, shouldn't use any data structure of file system
    // return a struct 
    // transfer received buffer to request_buffer
    static Request parse_request(int client_fd, 
                std::string& request, unsigned encrypted_request_size,
                const std::string& password, const std::string& username);
    
    // Takes a response message and encrypts it. Sends a message to client_fd with a cleartext
    // header appended to beginning. Throws if sending failed. Takes in 'request' to get client
    // password for encryption
    void send_helper(int client_fd, const std::string& message, const Request& request);

    // parse pathname into vector of object components
    static std::vector<std::string> parse_path(const std::string &pathname);

    // Checks that none of the filenames in the pathname exceed FS_MAXFILENAME
    static void check_valid_pathname(const Request& request);

public:
    // throw if an error occurs
    File_server(int port);

    void start() noexcept;

    friend std::ostream& operator<<(std::ostream& os, const Request& rhs);
};
#endif