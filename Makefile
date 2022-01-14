CFLAGS?=-Os -pedantic -Wall

all:
	$(CC) $(CFLAGS) littlewm.c -L/usr/include/X11 -lX11 -o littlewm

clean:
	rm -f littlewm