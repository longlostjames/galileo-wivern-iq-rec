# Makefile for Radar Data Acquisition Package (RDQ)
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
INC = $(INCDIR)RDQ.h

# Top level rule
all : $(LIBDIR)rdq.a

# The main library
$(LIBDIR)rdq.a : $(BINDIR)RDQ_DataAcquisition.o 
	ar r $(LIBDIR)rdq.a $(BINDIR)RDQ_DataAcquisition.o 

$(BINDIR)RDQ_DataAcquisition.o : $(SRCDIR)RDQ_DataAcquisition.c $(INC)
	$(CC) $(CFLAGS) -c $(SRCDIR)RDQ_DataAcquisition.c -o $(BINDIR)RDQ_DataAcquisition.o

clean :
	rm -f $(BINDIR)RDQ_DataAcquisition.o
	rm -f $(LIBDIR)rdq.a

