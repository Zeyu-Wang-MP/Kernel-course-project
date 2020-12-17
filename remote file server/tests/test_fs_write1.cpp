#include <iostream>
#include <cstdlib>
#include "fs_client.h"

using std::cout;
using std::endl;

/*
    Tests for correct writing (and reading) when there is a null in sent data.
*/

int main(int argc, char *argv[])

{
    char *server;
    int server_port;
    unsigned int session, session_two, seq=0;

    const char *writedata = "We\0hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, -- That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying its foundation on such principles and organizing its powers in such form, as to them shall seem most likely to effect their Safety and Happiness.";
    
    char readdata[FS_BLOCKSIZE];

    if (argc != 3) {
        cout << "error: usage: " << argv[0] << " <server> <serverPort>\n";
        exit(1);
    }
    server = argv[1];
    server_port = atoi(argv[2]);

    int test = fs_clientinit(server, server_port);
    if(test != 0) cout << "fs_clientinit error" << std::endl;

    fs_session("user1", "password1", &session, seq++);
    fs_create("user1", "password1", session, seq++, "/user1", 'd');
    fs_create("user1", "password1", session, seq++, "/user1/file", 'f');
    fs_writeblock("user1", "password1", session, seq++, "/user1/file", 0, writedata);

    // Read from directory
    fs_readblock("user1", "password1", session, seq++, "/user1/file", 0, readdata);

    for(unsigned i = 0; i < FS_BLOCKSIZE; ++i)
    {
        cout << readdata[i]; 
    }
    cout << endl;
}