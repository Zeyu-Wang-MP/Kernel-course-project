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
    if(test != 0) cout << "client error" << std::endl;
    test = fs_session("user1", "password1", &session, seq++);
    if(test != 0) cout << "session error" << std::endl;
    test = fs_create("user1", "password1", session, seq++, "/dir", 'd');
    if(test != 0) cout << "create error" << std::endl;
    test = fs_create("user1", "password1", session, seq++, "/dir/file", 'f');
    if(test != 0) cout << "create1 error" << std::endl;
    test = fs_writeblock("user1", "password1", session, seq++, "/dir/file", 0, writedata);
    if(test != 0) cout << "write error" << std::endl;
    test = fs_readblock("user1", "password1", session, seq++, "/dir/file", 0, readdata);
    if(test != 0) cout << "read error" << std::endl;
    test = fs_delete("user1", "password1", session, seq++, "/dir/file");
    if(test != 0) cout << "delete error" << std::endl;
    test = fs_delete("user1", "password1", session, seq++, "/dir");
    if(test != 0) cout << "delete1 error" << std::endl;
}