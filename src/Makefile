all : project4 p4diff
project4 : project4.o
	gcc -Wall -g project4.o -o project4 -fsanitize=address -lm
project4.o : project4.c
	gcc -Wall -g -c project4.c
p4diff : p4diff.c
	gcc -Wall -g p4diff.c -o p4diff -fsanitize=address -lm