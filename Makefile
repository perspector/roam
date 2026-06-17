CC=gcc

SRC=main.c \
	cJSON.c \
	ascii-view/src/image.c \
	ascii-view/src/print_image.c

roam: $(SRC)
		$(CC) $(SRC) -O2 -s -Iascii-view/include -o roam -lcurl -lm

clean:
		rm -f roam
