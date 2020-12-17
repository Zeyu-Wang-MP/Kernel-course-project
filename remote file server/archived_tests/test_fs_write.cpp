// This test case tests "buffer has to be exactly 512 bytes" + general use
#include <iostream>
#include <cstdlib>
#include "fs_client.h"

using std::cout;

int main(int argc, char *argv[])

{
    char *server;
    int server_port;
    unsigned int session, seq=0;

    const char *writedata = "We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, -- That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying its foundation on such principles and organizing its powers in such form, as to them shall seem most likely to effect their Safety and Happiness.";

    char readdata[FS_BLOCKSIZE];

    if (argc != 3) {
        cout << "error: usage: " << argv[0] << " <server> <serverPort>\n";
        exit(1);
    }
    server = argv[1];
    server_port = atoi(argv[2]);

    int test = fs_clientinit(server, server_port);
    if(test != 0) cout << "fs_clientinit error" << std::endl;

    test = fs_session("user1", "password1", &session, seq++);

    test = fs_create("user1", "password1", session, seq++, "/dir", 'd');
    test = fs_create("user1", "password1", session, seq++, "/file", 'f');

    // should cause an error becuase dir is not a file
    test = fs_writeblock("user1", "password1", session, seq++, "/dir", 0, writedata);
    if(test != 0) cout << "write error" << std::endl;

    test = fs_writeblock("user1", "password1", session, seq++, "/file", 0, writedata);
    if(test != 0) cout << "write2 error" << std::endl;
    test = fs_readblock("user1", "password1", session, seq++, "/file", 0, readdata);
    if(test != 0) cout << "read error" << std::endl;
    for(int i = 0; i < FS_BLOCKSIZE; ++i)
    {
        cout << readdata[i];
    }
    test = fs_writeblock("user1", "password1", session, seq++, "/file", 1, writedata);
    if(test != 0) cout << "write3 error" << std::endl;
    test = fs_readblock("user1", "password1", session, seq++, "/file", 1, readdata);
    if(test != 0) cout << "read2 error" << std::endl;
    for(int i = 0; i < FS_BLOCKSIZE; ++i)
    {
        cout << readdata[i];
    }

    // This time it should cause an error because write date is less than the block size.
    writedata = "We hold these truths to be self-evident, that all men are";
    test = fs_writeblock("user1", "password1", session, seq++, "/file", 0, writedata);
    if(test != 0) cout << "write4 error" << std::endl;
    test = fs_readblock("user1", "password1", session, seq++, "/file", 0, readdata);
    if(test != 0) cout << "read2 error" << std::endl;
    for(int i = 0; i < FS_BLOCKSIZE; ++i)
    {
        cout << readdata[i];
    }
}