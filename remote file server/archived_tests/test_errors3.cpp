#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <string>
#include "fs_client.h"

using std::cout;
using std::string;

/*
    Tests for correct error checking in create when directory is full
    (Note: won't run on autograder - too much output. Must create initial disk file)
    Verified with program - working.
*/
int main(int argc, char *argv[])

{
    char *server;
    int server_port;
    unsigned int session, session_two, seq=0;

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

    fs_session("user1", "password1", &session, seq++);

    string base = "/file";
    
    bool prematureError = false;
    int prematureErrorloc = 0;
    // Max is 194 Blocks per dir * 8 direntries per directory
    for (int i = 0; i < 8 * 124; ++i)
    {
        string intstring = std::to_string(i);
        string name = base + intstring;
        int error = fs_create("user1", "password1", session, seq++, name.c_str(), 'f');

        if(error == -1)
        {
            prematureError = true;
            prematureErrorloc = i;
            cout << "Error encountered prematurely" << std::endl;
            break;
        }
    }

    cout << "Finished creating" << std::endl;
    int error = fs_create("user1", "password1", session, seq++, "/file992", 'f');
    if(error == -1)
    {
        cout << "Error encountered at last create" << std::endl;
    }

    if(!prematureError)
    {
        cout << "No premature error" << std::endl;
    }
    else
    {
        cout << "Premature error encountered at " << prematureErrorloc << std::endl;
    }
}