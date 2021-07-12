CC=gcc
FLAGS=-g -Wall -Wextra
SRC=*.c *.h

main: $(SRC)
	$(CC) $(FLAGS) $(SRC) -o exec.out
clean:
	rm exec.out

