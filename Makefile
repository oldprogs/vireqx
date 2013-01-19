# Makefile for VIREQ/x

vireq vimkidx: vimkidx.c vireq.c
	$(CC) $(CFLAGS) -o vireq vireq.c
	$(CC) $(CFLAGS) -o vimkidx vimkidx.c
