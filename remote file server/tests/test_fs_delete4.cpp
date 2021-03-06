#include <iostream>
#include <cstdlib>
#include "fs_client.h"

using std::cout;

/*
    Also tests that delete correctly shifts elements across blocks when a block becomes free.
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
    fs_create("user1", "password1", session, seq++, "/child1", 'd');
    fs_create("user1", "password1", session, seq++, "/child2", 'f');
    fs_create("user1", "password1", session, seq++, "/child3", 'd');
    fs_create("user1", "password1", session, seq++, "/child4", 'f');
    fs_create("user1", "password1", session, seq++, "/child5", 'd');
    fs_create("user1", "password1", session, seq++, "/child6", 'f');
    fs_create("user1", "password1", session, seq++, "/child7", 'd');
    fs_create("user1", "password1", session, seq++, "/child8", 'f');
    fs_create("user1", "password1", session, seq++, "/child9", 'f');
    fs_create("user1", "password1", session, seq++, "/child10", 'f');
    fs_create("user1", "password1", session, seq++, "/child11", 'f');
    fs_create("user1", "password1", session, seq++, "/child12", 'f');
    fs_create("user1", "password1", session, seq++, "/child13", 'f');
    fs_create("user1", "password1", session, seq++, "/child14", 'f');
    fs_create("user1", "password1", session, seq++, "/child15", 'f');
    fs_create("user1", "password1", session, seq++, "/child16", 'f');
    fs_create("user1", "password1", session, seq++, "/child17", 'f');
    fs_create("user1", "password1", session, seq++, "/child18", 'f');
    fs_create("user1", "password1", session, seq++, "/child19", 'f');
    fs_create("user1", "password1", session, seq++, "/child20", 'f');
    fs_create("user1", "password1", session, seq++, "/child21", 'f');
    fs_create("user1", "password1", session, seq++, "/child22", 'f');
    fs_create("user1", "password1", session, seq++, "/child23", 'f');
    fs_create("user1", "password1", session, seq++, "/child24", 'f');
    fs_create("user1", "password1", session, seq++, "/child25", 'f');

    fs_delete("user1", "password1", session, seq++, "/child9");
    fs_delete("user1", "password1", session, seq++, "/child10");
    fs_delete("user1", "password1", session, seq++, "/child11");
    fs_delete("user1", "password1", session, seq++, "/child12");
    fs_delete("user1", "password1", session, seq++, "/child14");
    fs_delete("user1", "password1", session, seq++, "/child15");
    fs_delete("user1", "password1", session, seq++, "/child16");

    fs_delete("user1", "password1", session, seq++, "/child13"); // Should shift child 17 and 18.
}