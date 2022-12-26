# make file for radar-galileo-rec.c
# Owain Davies 22/5/2003
# Ed Pavelin 18/5/2004

CC=gcc
#CFLAGS= -Wall -ffast-math -O3 -I /usr/local/dislin -L/usr/X11R6/lib -L/usr/local/dislin
CFLAGS= -Wall -ffast-math -I /usr/local/dislin -L/usr/X11R6/lib -L/usr/local/dislin -ggdb -ggdb3
LIBS= -lm -lpthread -lnetcdf -lfftw3 -ldislin -lX11
# explanation of the flags
# Wall prints lots of extra warning messages
# lm includes the maths library
# ffast-math breaks some rules in the quest for execution speed
# O3 does as much optimising as it can (trade off, longer compilation time)
#URC_PATH=/root/universal_radar_code/
#URC_PATH=/home/chilbolton_software/universal_radar_code/
#URC_PATH=/home/chilbolton_software/radar-galileo/radar-galileo-iq-rec/version-0.2/universal_radar_code/
URC_PATH=universal_radar_code
PATH_RSP=$(URC_PATH)RSP/lib/
PATH_RDQ=$(URC_PATH)RDQ/lib/
PATH_RNC=$(URC_PATH)RNC/lib/
PATH_RSM=$(URC_PATH)RSM/lib/
PATH_REL=$(URC_PATH)REL/lib/
INSTALL_DIR=/usr/local/bin/

override CFLAGS += -I$(URC_PATH)/RDQ/include
override CFLAGS += -I$(URC_PATH)/RSP/include
override CFLAGS += -I$(URC_PATH)/RNC/include
override CFLAGS += -I$(URC_PATH)/RSM/include
override CFLAGS += -I$(URC_PATH)/include

EXE=radar-galileo-iq-rec-dualpol

# Universal radar code libraries
URC_LIBS = $(PATH_RDQ)/librdq12.a $(PATH_RSM)/librsm.a \
	   $(PATH_RSP)/librsp.a $(PATH_RNC)/librnc.a
LDFLAGS  = -L $(PATH_RDQ) -L $(PATH_RSM) -L $(PATH_RSP) -L $(PATH_RNC) \
	    -lrdq12 -lrsm -lrsp -lrnc


$(EXE) : radar-galileo-iq-rec-dualpol.o readconf.o median.o $(PATH_RSP)rsp.a $(PATH_RDQ)rdq.a $(PATH_RNC)rnc.a $(PATH_RSM)rsm.a $(PATH_REL)rel.a
	$(CC) $(CFLAGS) radar-galileo-iq-rec-dualpol.o median.o $(PATH_RSP)rsp.a $(PATH_RDQ)rdq.a $(PATH_RNC)rnc.a $(PATH_RSM)rsm.a $(PATH_REL)rel.a readconf.o -o $(EXE) $(LIBS) 

readconf.o : readconf.c readconf.h
	$(CC) $(CFLAGS) -c readconf.c

median.o : median.c median.h
	$(CC) $(CFLAGS) -c median.c

radar-galileo-iq-rec-dualpol.o : radar-galileo-iq-rec-dualpol.c radar-galileo-iq-rec-dualpol.h
	$(CC) $(CFLAGS) -c radar-galileo-iq-rec-dualpol.c

clean : 
	rm -f *.o radar-galileo-iq-rec-dualpol

install : 
	echo "Installing radar-galileo-iq-rec-dualpol"
	echo $(INSTALL_DIR) 
	cp $(EXE) $(INSTALL_DIR).
	chown jif $(INSTALL_DIR)$(EXE)
	chgrp radar_dt $(INSTALL_DIR)$(EXE)
	chmod ug+s  $(INSTALL_DIR)$(EXE)
	cp -a run_galileo-iqdata_acquisition-dualpol.pl $(INSTALL_DIR).
	cp -a start_radar-galileo-iq-dualpol.sh $(INSTALL_DIR).
	cp -a stop_radar-galileo-iq-dualpol.sh $(INSTALL_DIR).




 
