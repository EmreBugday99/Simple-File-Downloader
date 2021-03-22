default: server.o
	gcc server.o -pthread -O3 -o server

main.o: server.c
	gcc -c server.c

clean:
	rm *.o server

run:
	./server