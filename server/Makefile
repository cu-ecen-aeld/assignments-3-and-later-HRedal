ifdef $(CROSS_COMPILE)
  CC=$(CROSS_COMPILE)
else
  CC=gcc
endif

CFLAGS=-I. -g

ODIR=obj

OBJ = aesdsocket.o

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

aesdsocket : $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
	
clean:
	rm -f ./*.o *~
	
all: clean aesdsocket
default: aesdsocket 
