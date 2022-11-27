# Makefile for Radar NetCDF Package (RNC)
# Owain Davies 04/02/2004 

# This is the base path for the package
ROOTPATH=./

# This is where the bits and pieces are kept
SRCDIR = $(ROOTPATH)src/
LIBDIR = $(ROOTPATH)lib/
BINDIR = $(ROOTPATH)bin/
INCDIR = $(ROOTPATH)include/

CC=gcc
CFLAGS = -Wall -ffast-math -O3 -I$(INCDIR)
LIBS= -lm -lpthread

# The master header file
INC = $(INCDIR)REL.h

# Top level rule
all : $(LIBDIR)rel.a

# The main library
$(LIBDIR)rel.a : $(BINDIR)REL_SerialMessage.o 
	ar r $(LIBDIR)rel.a $(BINDIR)REL_SerialMessage.o 

$(BINDIR)REL_SerialMessage.o : $(SRCDIR)REL_SerialMessage.c $(INC)
	$(CC) $(CFLAGS) -c $(SRCDIR)REL_SerialMessage.c -o $(BINDIR)REL_SerialMessage.o

test_REL : $(SRCDIR)test_REL.c $(SRCDIR)REL_SerialMessage.c $(INC)
	$(CC) $(SRCDIR)test_REL.c $(BINDIR)REL_SerialMessage.o -o test_REL $(LIBS)

clean :
	rm -f $(BINDIR)REL_SerialMessage.o
	rm -f $(LIBDIR)rel.a

