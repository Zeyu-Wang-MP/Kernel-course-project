#include <iostream>
#include <cstdlib>
#include "fs_client.h"

using std::cout;

/*
    Tests for correct error checking (sends invalid requests).
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
    fs_create("user1", "password1", session, seq++, "/user1", 'd');
    fs_create("user1", "password1", session, seq++, "/dir", 'd');
    fs_create("user1", "password1", session, seq++, "/dir/file", 'f');
    fs_writeblock("user1", "password1", session, seq++, "/dir/file", 0, writedata);

    // Write to directory
    fs_writeblock("user1", "password1", session, seq++, "/dir", 0, writedata);

    // Read from directory
    fs_readblock("user1", "password1", session, seq++, "/dir", 0, readdata);

    // Read out of range
    fs_readblock("user1", "password1", session, seq++, "/dir/file", 1, readdata);

    // Write out of range
    fs_writeblock("user1", "password1", session, seq++, "/dir/file", 2, writedata);

    // Create in file
    fs_create("user1", "password1", session, seq++, "/dir/file/file2", 'f');
    fs_create("user1", "password1", session, seq++, "/dir/file/dir2", 'd');

    // Valid actions without a session
    unsigned int session2, seq2=0;
    fs_create("user2", "password2", session2, seq++, "/dir0", 'd');
    fs_create("user2", "password2", session2, seq++, "/dir0/file", 'f');
    fs_writeblock("user2", "password2", session2, seq++, "/dir0/file", 0, writedata);
    fs_readblock("user2", "password2", session2, seq++, "/dir0/file", 0, readdata);
    fs_delete("user2", "password2", session2, seq++, "/dir0/file");

    // Bad permission access of directory/file
    fs_session("user2", "password2", &session2, seq2++);
    fs_create("user2", "password2", session2, seq2++, "/user1/user2", 'd');
    fs_readblock("user2", "password2", session2, seq2++, "/dir/file", 0, readdata);
    fs_writeblock("user2", "password2", session2, seq2++, "/dir/file", 0, writedata);
    fs_delete("user2", "password2", session2, seq2++, "/user1");

    // Bad password
    fs_session("user2", "password1", &session2, seq++);

    // Bad directory 
    fs_create("user1", "password1", session, seq++, "dir/file2", 'f');
    fs_writeblock("user1", "password1", session, seq++, "dir/file", 0, writedata);

    // Create already existing file
    fs_create("user1", "password1", session, seq++, "/dir/file", 'f');
    fs_create("user1", "password1", session, seq++, "/dir/file", 'd');
}