# cgc
This is a barebones garbage collector for C. It is a simple stop-the-world, 
mark and sweep GC for learning purposes. Example usage is in the 
[main.c](https://github.com/rmrt1n/cgc/blob/main/main.c) file. The output of 
valgrind's memcheck said that all heap blocks were freed, so I'm assuming it 
works. 

