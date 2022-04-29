CC = gcc -pthread
CFLAGS = -g

SRC = matrix.c
OBJ = matrix.o

all: $(OBJ) 
	$(CC) $(OBJ) -o matrix -lm

$(OBJ): $(SRC)
	$(CC) -MMD -MP -c $(SRC)

clean:
	rm -rf matrix *.o *.d
