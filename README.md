# fimer
Timer daemon for file operations

BUILD
------
```Shell
$ make all
```

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
$ ./fimerd 

$ ./fimerclient filepath time_delay opcode <permissions|new_path>
Supported operations: 
1. Change file permissions (opcode = 1)
2. Delete a file (opcode = 2)
3. Truncate a file (opcode = 3)
4. Rename/Move a file (opcode = 4)

Supported time delays: 
s[seconds] | m[minutes] | h[hours] | d[days] | M[months] | y[years]

```

Example commands
------
```C
* ./fimerclient ~/mysecretfile 2d 1 0755
* ./fimerclient ~/mysecretfile 1M 2
* ./fimerclient ~/mylogfile 7d 3
* ./fimerclient ~/myfile 60m 4 /home/myaccount/files/backupfile

```

## Demo
[![Project Fimer Demo](http://img.youtube.com/vi/-FFIowmf-wA/0.jpg)](http://www.youtube.com/watch?v=-FFIowmf-wA)
