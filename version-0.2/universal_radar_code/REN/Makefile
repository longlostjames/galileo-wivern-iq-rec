# Makefile for Radar GPIB ENET Package (REN)
# Owain Davies 04/02/2004 

# This is the base path for the package
ROOTPATH=/home/chilbolton_software/universal_radar_code/REN/

# This is where the bits and pieces are kept
SRCDIR = $(ROOTPATH)src/
LIBDIR = $(ROOTPATH)lib/
BINDIR = $(ROOTPATH)bin/
INCDIR = $(ROOTPATH)include/

CC=gcc
CFLAGS = -Wall -ffast-math -O3 -I$(INCDIR)
LIBS= -lm

# The master header file
INC = $(INCDIR)REN.h

# Top level rule
all : $(LIBDIR)ren.a

# The main library
$(LIBDIR)ren.a : $(BINDIR)REN_GpibEnet.o 
	ar r $(LIBDIR)ren.a $(BINDIR)REN_GpibEnet.o 

$(BINDIR)REN_GpibEnet.o : $(SRCDIR)REN_GpibEnet.c $(INC)
	$(CC) $(CFLAGS) -c $(SRCDIR)REN_GpibEnet.c -o $(BINDIR)REN_GpibEnet.o

clean :
	rm -f $(BINDIR)REN_GpibEnet.o
	rm -f $(LIBDIR)ren.a

