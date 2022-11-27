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
#CFLAGS = -Wall -ffast-math -O3 -I$(INCDIR) 
CFLAGS = -Wall -ffast-math -I$(INCDIR) -ggdb -ggdb3
LIBS= -lm

# The master header file
INC = $(INCDIR)RNC.h

# Top level rule
all : $(LIBDIR)rnc.a

# The main library
$(LIBDIR)rnc.a : $(BINDIR)RNC_NetCDF.o $(BINDIR)RNC_ReadConfig.o
	ar r $(LIBDIR)rnc.a $(BINDIR)RNC_NetCDF.o $(BINDIR)RNC_ReadConfig.o

$(BINDIR)RNC_NetCDF.o : $(SRCDIR)RNC_NetCDF.c $(INC)
	$(CC) $(CFLAGS) -c $(SRCDIR)RNC_NetCDF.c -o $(BINDIR)RNC_NetCDF.o

$(BINDIR)RNC_ReadConfig.o : $(SRCDIR)RNC_ReadConfig.c $(INC)
	$(CC) $(CFLAGS) -c $(SRCDIR)RNC_ReadConfig.c -o $(BINDIR)RNC_ReadConfig.o

clean :
	rm -f $(BINDIR)RNC_NetCDF.o
	rm -f $(LIBDIR)rnc.a

