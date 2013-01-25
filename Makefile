# Makefile for VIREQ/x

CFLAGS+=-Wall

vireq vimkidx: vimkidx.c vireq.c
	$(CC) $(CFLAGS) -o vireq vireq.c
	$(CC) $(CFLAGS) -o vimkidx vimkidx.c

clean:
	rm -f vireq vimkidx
