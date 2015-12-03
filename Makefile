# makefile for autostereogram utils
#
# USAGE:    
#       make all    - make all required binaries
#       make clean  - delete all intermediate files and binaries
#

# specify libraries
LIBS = -lm

# specify compiler flags
CFLAGS = -Wall

# supply source file here
SRC=$(wildcard *.c)

all: $(SRC)
	@echo "### Output generated from file(s): ###\n"$(SRC)
	$(foreach cmd, $(SRC:.c=),gcc $(CFLAGS) $(cmd).c $(LIBS) -o $(cmd) ;)

#stereogram: $(SRC)
#	gcc -o $@ $^ $(CFLAGS) $(LIBS)

imagePrep:
	gcc $(CFLAGS) float01uchar.c $(LIBS) -o float01uchar
	gcc $(CFLAGS) float02uchar.c $(LIBS) -o float02uchar

encoder:
	gcc $(CFLAGS) autostereogram.c $(LIBS) -o autostereogram

decoder:
	gcc $(CFLAGS) decoder.c $(LIBS) -o decoder

# delete all intermediate and output files
clean:
	$(foreach cmd, $(SRC:.c=),rm -f $(cmd).o $(cmd) ;)
