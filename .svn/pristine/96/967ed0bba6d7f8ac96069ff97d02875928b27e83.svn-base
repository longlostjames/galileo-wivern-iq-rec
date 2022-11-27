# Makefile for Radar Data Acquisition Package (RDQ)
# Owain Davies 04/02/2004 

# This is the base path for the package
ROOTPATH=./

# This is where the bits and pieces are kept
SRCDIR=$(ROOTPATH)src/
LIBDIR=$(ROOTPATH)lib/
BINDIR=$(ROOTPATH)bin/
INCDIR=$(ROOTPATH)include/

CONDIR=/root/d2k-dask_175/samples/conio/

PROG=RAT_AdlinkDataAcquisition

URC_PATH=./
PATH_RNC=$(URC_PATH)RNC/lib/

CC=gcc
CFLAGS= -Wall -ffast-math -O3 -I /usr/local/dislin -L/usr/X11R6/lib -L/usr/local/dislin -I /usr/include/netcdf-3/ -L /usr/lib/netcdf-3/ -L /usr/lib/ -I /root/d2k-dask_175/include/ -I $(INCDIR)
LIBS= -lm -lpthread -lnetcdf -lfftw3 -ldislin -lX11 -lpci_dask2k

# The master header file
INC = $(INCDIR)RAT.h

# Top level rule
all : $(LIBDIR)rat.a

diag : $(BINDIR)RAT_diag.o $(BINDIR)conio.o $(LIBDIR)rat.a
	$(CC) $(CFLAGS) $(BINDIR)RAT_diag.o $(BINDIR)conio.o $(LIBDIR)rat.a $(PATH_RNC)rnc.a -o $(BINDIR)RAT_diag $(LIBS)

$(BINDIR)RAT_diag.o : $(SRCDIR)RAT_diag.c
	$(CC) $(CFLAGS) -o $(BINDIR)RAT_diag.o -c $(SRCDIR)RAT_diag.c	

# The main library
$(LIBDIR)rat.a : $(BINDIR)$(PROG).o 
	ar r $(LIBDIR)rat.a $(BINDIR)$(PROG).o 

$(BINDIR)$(PROG).o : $(SRCDIR)$(PROG).c 
	$(CC) $(CFLAGS) -c $(SRCDIR)$(PROG).c -o $(BINDIR)$(PROG).o

$(BINDIR)conio.o : $(CONDIR)conio.c
	$(CC) $(CFLAGS) -o $(BINDIR)conio.o -c $(CONDIR)conio.c

clean :
	rm -f $(BINDIR)$(PROG).o
	rm -f $(LIBDIR)rat.a
	rm -f $(BINDIR)RAT_diag
	rm -f $(BINDIR)RAT_diag.o

