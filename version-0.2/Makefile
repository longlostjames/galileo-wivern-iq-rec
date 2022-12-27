# make file for radar-galileo-rec.c
# Owain Davies 22/5/2003
# Ed Pavelin 18/5/2004

CC     = gcc
CFLAGS = -Wall -O3 -ffast-math -I/usr/local/dislin -ggdb -ggdb3 -DHAVE_DISLIN \
	 -D_FILE_OFFSET_BITS=64
LIBS   = -L/usr/local/dislin -L/usr/X11R6/lib -ldislin -lX11 \
	 -lnetcdf -lfftw3 -lm -lpthread -lrt

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
PATH_RTS=$(URC_PATH)/RTS/lib

INSTALL_DIR=/usr/local/bin/

override CFLAGS += -I$(URC_PATH)/RDQ/include
override CFLAGS += -I$(URC_PATH)/RSP/include
override CFLAGS += -I$(URC_PATH)/RNC/include
override CFLAGS += -I$(URC_PATH)/RSM/include
override CFLAGS += -I$(URC_PATH)/RTS/include
override CFLAGS += -I$(URC_PATH)/include

EXE=radar-galileo-iq-rec-dualpol

# Universal radar code libraries
URC_LIBS = $(PATH_RDQ)/librdq12.a $(PATH_RSM)/librsm.a \
	   $(PATH_RSP)/librsp.a $(PATH_RNC)/librnc.a $(PATH_RTS)/librts.a
LDFLAGS  = -L $(PATH_RDQ) -L $(PATH_RSM) -L $(PATH_RSP) -L $(PATH_RNC) \
	   -L $(PATH_RTS) -lrdq12 -lrsm -lrsp -lrnc -lrts


all: galileo-iq

.PHONY: all clean help galileo-iq install

galileo-iq : $(EXE)

help:
	@echo
	@echo "make galileo-iq     Make Galileo IQ runtime data acquisition program"
	@echo
	@echo "make install     Install Galileo IQ runtime data acquisition program"
	@echo
	@echo "make clean       Cleanup build"
	@echo "make help        This help"
	@echo


$(EXE) : radar-galileo-iq-rec-dualpol.o median.o $(URC_LIBS)
	$(CC) $(CFLAGS) -o $@ radar-galileo-iq-rec-dualpol.o median.o \
		$(LDFLAGS) $(LIBS)


$(PATH_RDQ)/librdq12.a:
	$(MAKE) -C $(URC_PATH)/RDQ

$(PATH_RSM)/librsm.a:
	$(MAKE) -C $(URC_PATH)/RSM

$(PATH_RSP)/librsp.a:
	$(MAKE) -C $(URC_PATH)/RSP

$(PATH_RNC)/librnc.a:
	$(MAKE) -C $(URC_PATH)/RNC

$(PATH_RTS)/librts.a:
	$(MAKE) -C $(URC_PATH)/RTS


#$(EXE) : radar-galileo-iq-rec-dualpol.o readconf.o median.o $(PATH_RSP)rsp.a $(PATH_RDQ)rdq.a $(PATH_RNC)rnc.a $(PATH_RSM)rsm.a $(PATH_REL)rel.a
#	$(CC) $(CFLAGS) radar-galileo-iq-rec-dualpol.o median.o $(PATH_RSP)rsp.a $(PATH_RDQ)rdq.a $(PATH_RNC)rnc.a $(PATH_RSM)rsm.a $(PATH_REL)rel.a readconf.o -o $(EXE) $(LIBS) 

#readconf.o : readconf.c readconf.h
#	$(CC) $(CFLAGS) -c readconf.c


median.o : median.c median.h
	$(CC) $(CFLAGS) -c median.c

radar-galileo-iq-rec-dualpol.o : radar-galileo-iq-rec-dualpol.c radar-galileo-iq-rec-dualpol.h
	$(CC) $(CFLAGS) -c radar-galileo-iq-rec-dualpol.c

clean :
	$(RM) *.[doa] $(EXE)
	$(MAKE) -C $(URC_PATH)/RDQ $@
	$(MAKE) -C $(URC_PATH)/RSM $@
	$(MAKE) -C $(URC_PATH)/RSP $@
	$(MAKE) -C $(URC_PATH)/RNC $@
	$(MAKE) -C $(URC_PATH)/RTS $@
	@find . $(URC_PATH) -name "*~" -type f -print -exec rm \{\} \;

install : 
	echo "Installing $(EXE)"
	echo $(INSTALL_DIR) 
	install -d $(INSTALL_DIR)
	install -o jif -g radar_dt -m 6550 $(EXE) $(INSTALL_DIR)
	#install -o jif -g radar_dt -m 6550 dish_scan_trigger_galileo.pl $(INSTALL_DIR)
	install -o jif -g radar_dt -m 6550 run_galileo-iqdata_acquisition-dualpol.pl $(INSTALL_DIR)
	install -o jif -g radar_dt -m 6550 start_radar-galileo-iq-dualpol.sh $(INSTALL_DIR)
	install -o jif -g radar_dt -m 6550 stop_radar-galileo-iq-dualpol.sh $(INSTALL_DIR)

















 
