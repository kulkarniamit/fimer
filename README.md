# fimer
Timer daemon for file operations

Objectives
------
1. Implement a daemon
2. Client-Server communication using TCP/IP Sockets
3. Utilize pthreads to delegate jobs to worker threads
4. Implement timers in C
5. Using linked lists for an actual application
6. Writing to syslog
7. Organize the project build using Makefile

Architecture
------
![alt text](https://github.com/kulkarniamit/fimer/blob/master/architecture.png "Architecture")

Usage
------
```C
$ ./fimerclient filepath timer_in_seconds opcode <permissions|new_path>
Supported operations: 
1. Change file permissions (opcode = 1)
2. Delete a file (opcode = 2)
3. Truncate a file (opcode = 3)
4. Rename/Move a file (opcode = 4)
```
