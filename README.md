# Kernel-course-project
This is a kernel project of an OS course. <br />
<br />
Implemented the C++ thread, mutex, condition variable library that can run on the infrastructure provided by libcpu.o. To use the library, just include "thread.h" and compile with libcpu.o and libthread.o. There is an example program using this thread library (app.cpp). <br />
<br />
Implemented the virtual memory management system that can run on the infrastructure provided by libvm_app.o and libvm_pager.o.<br />
<br />
Implemented a remote file system. To use it, run createfs to create a empty file system on server disk. Then run fs and give it a list of username - password pairs through stdin to start the file server. Then you can write a client progrom in another host to create/delete/read/write file or directories by calling functions provided in fs_client.h and link it with libfs_client.o. There is an example client program (app.cpp).
