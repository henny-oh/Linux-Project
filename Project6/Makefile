ssu_mntr : ssu_mntr.o tree.o
	gcc ssu_mntr.o tree.o -o ssu_mntr -pthread
ssu_mntr.o: ssu_mntr.c
	gcc -c ssu_mntr.c -pthread
tree.o:tree.c
	gcc -c tree.c
clean :
	rm *.o
	rm a.out
