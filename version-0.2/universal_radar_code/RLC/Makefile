# Makefile for Radar NetCDF Package (RNC)
# Owain Davies 04/02/2004 

# This is the base path for the package
ROOTPATH=/home/chilbolton_software/universal_radar_code/RLC/

# This is where the bits and pieces are kept
SRCDIR = $(ROOTPATH)src/
LIBDIR = $(ROOTPATH)lib/
BINDIR = $(ROOTPATH)bin/
INCDIR = $(ROOTPATH)include/

CC=gcc
CFLAGS = -Wall -ffast-math -O3 -I$(INCDIR)
LIBS= -lm

# The master header file
INC = $(INCDIR)RLC.h

# Top level rule
all : $(LIBDIR)rlc.a

# The main library
$(LIBDIR)rlc.a : $(BINDIR)RLC_ProgramLecroy.o 
	ar r $(LIBDIR)rlc.a $(BINDIR)RLC_ProgramLecroy.o

$(BINDIR)RLC_ProgramLecroy.o : $(SRCDIR)RLC_ProgramLecroy.c $(INC)
	$(CC) $(CFLAGS) -c $(SRCDIR)RLC_ProgramLecroy.c -o $(BINDIR)RLC_ProgramLecroy.o


clean :
	rm -f $(BINDIR)RLC_ProgramLecroy.o
	rm -f $(LIBDIR)rlc.a

