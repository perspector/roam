CC=gcc

SRC=main.c \
    cJSON.c \
    ascii-view/src/image.c \
    ascii-view/src/print_image.c

roam: $(SRC)
        $(CC) $(SRC) -O2 -s -Iascii-view/include -o roam -lcurl -lm

release: roam
        rm -rf release
        mkdir release
        cp roam.exe release/
        ldd roam.exe | grep '/ucrt64/bin/' | awk '{print $$3}' | xargs -I{} cp "{}" release/

clean:
        rm -f roam.exe
        rm -rf release
