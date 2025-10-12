all:
	gcc nanosleep_cb.c -c
	gcc main.c -c
	gcc main.o nanosleep_cb.o -o test

clean:
	rm *.o
	rm test

