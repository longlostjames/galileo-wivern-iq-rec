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
URC_PATH=/home/chilbolton_software/universal_radar_code/
PATH_RSP=$(URC_PATH)RSP/lib/
PATH_RDQ=$(URC_PATH)RDQ/lib/
PATH_RNC=$(URC_PATH)RNC/lib/
PATH_RSM=$(URC_PATH)RSM/lib/
PATH_REL=$(URC_PATH)REL/lib/
INSTALL_DIR=/usr/local/bin/
EXE=radar-galileo-iq-rec

$(EXE) : radar-galileo-iq-rec.o readconf.o median.o $(PATH_RSP)rsp.a $(PATH_RDQ)rdq.a $(PATH_RNC)rnc.a $(PATH_RSM)rsm.a $(PATH_REL)rel.a
	$(CC) $(CFLAGS) radar-galileo-iq-rec.o median.o $(PATH_RSP)rsp.a $(PATH_RDQ)rdq.a $(PATH_RNC)rnc.a $(PATH_RSM)rsm.a $(PATH_REL)rel.a readconf.o -o $(EXE) $(LIBS) 

readconf.o : readconf.c readconf.h
	$(CC) $(CFLAGS) -c readconf.c

median.o : median.c median.h
	$(CC) $(CFLAGS) -c median.c

radar-galileo-iq-rec.o : radar-galileo-iq-rec.c radar-galileo-iq-rec.h
	$(CC) $(CFLAGS) -c radar-galileo-iq-rec.c

clean : 
	rm -f *.o radar-galileo-iq-rec

install : 
	echo "Installing radar-galileo-iq-rec"
	echo $(INSTALL_DIR) 
	cp $(EXE) $(INSTALL_DIR).
	chown jif $(INSTALL_DIR)$(EXE)
	chgrp radar_dt $(INSTALL_DIR)$(EXE)
	chmod ug+s  $(INSTALL_DIR)$(EXE)
	cp -a run_galileo-iqdata_acquisition.pl $(INSTALL_DIR).
	cp -a start_radar-galileo-iq.sh $(INSTALL_DIR).
	cp -a stop_radar-galileo-iq.sh $(INSTALL_DIR).




 
