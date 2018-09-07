CC     = gcc
CFLAGS = -Wall
SRC    = *.c
OUT    = packbits


$(OUT):$(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)
