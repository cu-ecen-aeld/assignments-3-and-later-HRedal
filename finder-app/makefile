ifdef $(CROSS_COMPILE)
  CC=$(CROSS_COMPILE)
else
  CC=gcc
endif

CFLAGS=-I.

ODIR=obj

OBJ = writer.o

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

writer: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
	
clean:
	rm -f ./*.o *~ 
