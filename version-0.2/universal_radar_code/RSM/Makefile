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
LIBS= -lm

# The master header file
INC = $(INCDIR)RSM.h

# Top level rule
all : $(LIBDIR)rsm.a

# The main library
$(LIBDIR)rsm.a : $(BINDIR)RSM_SerialMessage.o 
	ar r $(LIBDIR)rsm.a $(BINDIR)RSM_SerialMessage.o 

$(BINDIR)RSM_SerialMessage.o : $(SRCDIR)RSM_SerialMessage.c $(INC)
	$(CC) $(CFLAGS) -c $(SRCDIR)RSM_SerialMessage.c -o $(BINDIR)RSM_SerialMessage.o

clean :
	rm -f $(BINDIR)RSM_SerialMessage.o
	rm -f $(LIBDIR)rsm.a

