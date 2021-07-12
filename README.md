# cgc
This is a barebones garbage collector for C. The only test I did for it is 
in the main.c file, so I'm not even sure if it's correct. The output of 
valgrind's memcheck said that all heap blocks were freed, so I'm assuming 
it somewhat works. Needs more testing to verify.
