CC=gcc

CFLAGS=-Iascii-view/include

LIBS=-lcurl -lcjson -lm

SRC=main.c \
    ascii-view/src/image.c \
    ascii-view/src/print_image.c

roam: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o roam $(LIBS)

clean:
	rm -f roam
