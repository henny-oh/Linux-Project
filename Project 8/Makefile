CC = gcc
CFLAGS = -Wall -W
TARGET = ssu_score
OBJ = ssu_main.o ssu_score.o ssu_file.o ssu_student.o ssu_string.o


debug : ssu_main.c ssu_score.c ssu_file.c ssu_student.c ssu_string.c
	$(CC) $(CFLAGS) -g ssu_main.c ssu_score.c ssu_file.c ssu_student.c ssu_string.c -o ssu_score

$(TARGET) : $(OBJ)
	$(CC) -o $@ $^
	rm -f *.o

clean :
	rm -f $(TARGET)
	rm -f *.o

ssu_main.o : ssu_main.c
	$(CC) $(CFLAGS) -o $@ -c $^

ssu_score.o : ssu_score.c
	$(CC) $(CFLAGS) -o $@ -c $^

ssu_file.o : ssu_file.c
	$(CC) $(CFLAGS) -o $@ -c $^

ssu_student.o : ssu_student.c
	$(CC) $(CFLAGS) -o $@ -c $^

ssu_string.o : ssu_string.c
	$(CC) $(CFLAGS) -o $@ -c $^

