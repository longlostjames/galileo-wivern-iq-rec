#define VERSION_NUMBER "0.1"

// radar-galileo-iq-rec.c
// Development Version
// ---------------------------------------------------------------
// Acquisition software for IQ data from 94-GHz radar
//
// John Nicol - April 2014
// Based on radar-galileo-rec (version 0.6) 
// Owain Davies / Ed Pavelin - May 2004
//
// ---------------------------------------------------------------
// REVISION HISTORY
// ---------------------------------------------------------------
// 08/04/14: JCN: Created v0.1 

#define NDEBUG

#include <stdio.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>

#include "readconf.h"
#include "median.h"
#include <fftw3.h>
#include <dislin.h>

#define PI 3.141592654

// Master header file for the Universal Radar Code
#include "/root/universal_radar_code/include/radar.h"
// Include file for the RSP package
#include "/root/universal_radar_code/RSP/include/RSP.h"
// Include file for the RDQ package
#include "/root/universal_radar_code/RDQ/include/RDQ.h"
// Include file for the RNC package
#include "/root/universal_radar_code/RNC/include/RNC.h"
// Include file for the REL package
#include "/root/universal_radar_code/REL/include/REL.h"
// Include file for the RSM package
#include "/root/universal_radar_code/RSM/include/RSM.h"

/* header file */
#include "radar-galileo-iq-rec.h"
/* below defines the com port that the serial message arrives on */
/* 1 is /dev/ttyS0 or COM1 in MS-DOS langauge */
#define SERIALMESSAGE_PORT			"/dev/ttyS1"

float 	zmat[256][250];
int	counter_x, counter_y;

//-----------------------------
// GLOBAL VARIABLE DEFINITIONS
//-----------------------------

int     dmux_table[8];

/* used for communication between the signal handler */
/* and the main program */
int	exit_now = 0; 

/* function prototype declaration */
void 	sig_handler(int sig);

// Displays a welcome message with version information
void disp_welcome_message(void)
{
  char buffer[80];

  strcpy(buffer,VERSION_NUMBER);
  printf("\n");
  printf("radar-galileo-iq-rec: Version %s\n\n",buffer);
}

/* signal handler */
void sig_handler(int sig)
{
   if (exit_now == 0)
   {
        exit_now = 1;
        printf("***********************************\n");
        printf("* Received signal %d. Exiting soon *\n", sig);
        printf("***********************************\n");
   }
}

// Disable serial message for fixed position operation
int serialMessageAct=1;	// default is ON


int parseargs(int argc, char *argv[], RSP_ParamStruct *param, URC_ScanStruct *scan)
{
  //-------------------------
  // Parse command line args
  //-------------------------

  time_t    system_time;
  struct tm *time_ptr;
  char      datestring[15];

  int year, month, day, hour, minute, second;
  int i;

  // Initialise defaults for scan params
  scan->scanType=SCAN_FIX;
  scan->file_number=0;
  scan->scan_number=0;
  scan->experiment_id=0;
  scan->scan_velocity=-9999;
  scan->dwelltime=-1;
  strcpy(scan->operator, "jif");

  system_time = time(NULL);
  time_ptr = gmtime(&system_time);
  year = time_ptr->tm_year+1900;
  month= time_ptr->tm_mon+1;
  day  = time_ptr->tm_mday;
  hour = time_ptr->tm_hour;
  minute=time_ptr->tm_min;
  second=time_ptr->tm_sec;
  sprintf(datestring,"%04d%02d%02d%02d%02d%02d",year,month,day,hour,minute,second);
  strcpy( scan->date, datestring);

/* parse command line arguments */
/* by default real_time_spectra_display is off */
param->real_time_spectra_display = 0;
if(argc>1)
{
	for(i=1;i<argc;i++)
	{
        	if (strcmp(argv[i],"-real_time_spectra_display")==0)
		{
         		printf("Real time spectra display enabled\n");
			param->real_time_spectra_display = 1;
		} else if (strcmp(argv[i],"-ppi")==0) {
          		i++;
          		scan->min_angle=atof(argv[i]);
          		i++;
          		scan->max_angle=atof(argv[i]);
          		printf("PPI Scan %f-%f deg\n",scan->min_angle,scan->max_angle);
          		scan->scanType=SCAN_PPI;
        	} else if (strcmp(argv[i],"-rhi")==0) {
          		i++;
          		scan->min_angle=atof(argv[i]);
         		i++;
          		scan->max_angle=atof(argv[i]);
          		printf("RHI Scan %f-%f deg\n",scan->min_angle,scan->max_angle);
          		scan->scanType=SCAN_RHI;
        	} else if (strcmp(argv[i],"-csp")==0) {
          		i++;
          		scan->min_angle=atof(argv[i]);
          		i++;
          		scan->max_angle=atof(argv[i]);
          		printf("CSP Scan %f-%f deg\n",scan->min_angle,scan->max_angle);
          		scan->scanType=SCAN_CSP;
        	} else if (strcmp(argv[i],"-fix")==0) {
          		i++;
          		scan->dwelltime=atof(argv[i]);
          		i++;
          		scan->min_angle=atof(argv[i]);
          		i++;
          		scan->max_angle=atof(argv[i]);
          		printf("Fixed dwell for %f seconds\n",scan->dwelltime);
          		scan->scanType=SCAN_FIX;
		} else if(strcmp(argv[i],"--noserial")==0) {
			i++;
			serialMessageAct=0;
		} else if(strcmp(argv[i],"-sv")==0) {
          		i++;
          		scan->scan_velocity = atof(argv[i]);
        	} else if(strcmp(argv[i],"-file")==0) {
          		i++;
          		scan->file_number = atoi(argv[i]);
          		printf("File number   : %04d\n", scan->file_number);
        	} else if(strcmp(argv[i],"-scan")==0) {
          		i++;
          		scan->scan_number = atoi(argv[i]);
          		printf("Scan number   : %04d\n", scan->scan_number);
        	} else if(strcmp(argv[i],"-id")==0) {
          		i++;
          		scan->experiment_id = atoi(argv[i]);
          		printf("Experiment id : %d\n", scan->experiment_id);
        	} else if(strcmp(argv[i],"-scan_angle")==0) {
          		i++;
          		scan->scan_angle = atof(argv[i]);
         	 	printf("Scan angle : %f\n", scan->scan_angle);
        	} else if(strcmp(argv[i],"-date")==0) {
          		i++;
          		strcpy(scan->date,argv[i]);
        	} else if(strcmp(argv[i],"-op")==0) {
          		i++;
          		strcpy(scan->operator,argv[i]);
        	} else {
          		printf("** UNKNOWN COMMAND LINE ARGUMENT '%s'!\n",argv[i]);
          		return(1);
		}
        }
}

/* this is to remove the effect of Chilbolton azimuths */
/* care may have to be taken when the radar is not on the dish */
/* since this may cause an error in the metadata */
if ( scan->scanType == SCAN_PPI ) {
   scan->min_angle = scan->min_angle - 90;
   scan->max_angle = scan->max_angle - 90;
} else {
   scan->scan_angle = scan->scan_angle - 90;
}


return(0);

}


//--------------------------------------------------------------------
// get_config : reads radar config file
void get_config(char *filename,RSP_ParamStruct *param, URC_ScanStruct *scan, int is_coded)
{
    char codefile[255];

    printf("Accessing config file: %s\n", filename);

    param->frequency                  = getconf_double(filename,"radar-frequency");
    param->prf                        = getconf_double(filename,"prf");
    param->transmit_power             = getconf_double(filename,"transmit-power");
    param->pulses_per_daq_cycle       = getconf_double(filename,"pulses");
    param->samples_per_pulse          = getconf_double(filename,"samples");
    param->ADC_channels               = getconf_double(filename,"adc-channels");
    param->clock_divfactor            = getconf_double(filename,"adc-divfactor");
    param->delay_clocks               = getconf_double(filename,"adc-delayclocks");
    param->pulse_period               = getconf_double(filename,"chip-length");
    param->pulses_coherently_averaged = getconf_double(filename,"num-coh-avg");
    param->spectra_averaged           = getconf_double(filename,"num-spec-avg");
    param->moments_averaged           = getconf_double(filename,"num-moments-avg");
    param->fft_bins_interpolated      = getconf_double(filename,"reject-clutter-bins");
    param->clock                      = getconf_double(filename,"adc-clock");
    param->num_peaks                  = getconf_double(filename,"num-peaks");
    param->antenna_diameter 	= getconf_float(filename, "antenna_diameter");
    param->beamwidthH		= getconf_float(filename, "beamwidthH");
    param->beamwidthV		= getconf_float(filename, "beamwidthV");
    param->height		= getconf_float(filename, "height");
    param->azimuth_offset	= getconf_float(filename, "azimuth_offset");
    param->dump_spectra		= getconf_float(filename, "dump_spectra");
    param->dump_spectra_rapid   = getconf_float(filename, "dump_spectra_rapid");
    param->num_interleave       = getconf_float(filename, "num-interleave");
    param->num_tx_pol           = getconf_float(filename, "num-tx-pol");

/* this is for when the radar is fixed pointing in the cradle */
/* please take care when the radar is tilted */
    scan->min_angle		= getconf_float(filename, "antenna_elevation");
    scan->max_angle 		= getconf_float(filename, "antenna_elevation");
    scan->scan_angle		= getconf_float(filename, "antenna_azimuth");

    getconf(filename,"code-file",codefile);

    strcpy(param->code_name, "NOT YET IMPLEMENTED\0"); 

    // For non-coded pulses
    param->code_length=1;
    param->number_of_codes=1;
}

// Loads calibration information from *.cal file
void get_cal(RSP_ParamStruct *param, char *calfile)
{
  param->ZED_calibration_offset=getconf_double(calfile,"z-calibration");
  param->ZDR_calibration_offset=getconf_double(calfile,"zdr-calibration");
  param->LDR_calibration_offset=getconf_double(calfile,"ldr-calibration");
  param->ZED_incoherent_calibration_offset=getconf_double(calfile,"z-incoherent-calibration");
  param->ZED_incoherent_noise=getconf_double(calfile,"z-incoherent-noise");
  param->range_offset=getconf_double(calfile,"range-offset");
}

//--------------------------------------------------------------------
/* make_dmux_table generates the lookup table that is used to extract
 * channels from the DMA buffer 
 * IN:  channels  the number of channels
 * OUT: nowt */
void make_dmux_table( int channels)
{
    if (channels == 4)
    {
        dmux_table[0] = 0;
        dmux_table[1] = 2;
        dmux_table[2] = 1;
        dmux_table[3] = 3;
        dmux_table[4] = 0xFFFFFFFF; /* channels 4 to 7 do not exist in the eight channels system */
        dmux_table[5] = 0xFFFFFFFF;
        dmux_table[6] = 0xFFFFFFFF;
        dmux_table[7] = 0xFFFFFFFF;
    }
    if (channels == 8)
    {
        dmux_table[0] = 0;
        dmux_table[1] = 4;
        dmux_table[2] = 1;
        dmux_table[3] = 5;
        dmux_table[4] = 2;
        dmux_table[5] = 6;
        dmux_table[6] = 3;
        dmux_table[7] = 7;
    }
}


// Routine to wait for start of scan
void wait_scan_start(int scantype,float min_angle,float max_angle)
{
    static RSM_SerialMessageStruct serialmsg;
    float angle=0;

    printf("Waiting to get outside scan range...\n");
    do {
        RSM_ReadSerialMessage(&serialmsg);
        if(scantype==SCAN_PPI) angle=serialmsg.az;
        else if(scantype==SCAN_RHI) angle=serialmsg.el;
        else if(scantype==SCAN_CSP) angle=serialmsg.el;

        if(angle < min_angle || angle > max_angle) break;
    } while(exit_now == 0);

    printf("Waiting to get within scan range...\n");
    printf("min_angle: %.1f degrees  max_angle: %.1f\n", min_angle, max_angle);
    do {
        RSM_ReadSerialMessage(&serialmsg);
        if(scantype==SCAN_PPI) angle=serialmsg.az;
        else if(scantype==SCAN_RHI) angle=serialmsg.el;
        else if(scantype==SCAN_CSP) angle=serialmsg.el;
        if( (angle > min_angle) && (angle < max_angle)) break;
    } while( exit_now == 0);
}

//--------------------------------------------------------------------
// Routine to wait for end of scan
int scanEnd_test(int scantype, RSM_SerialMessageStruct serialmsg,float min_angle,float max_angle)
{
    float angle=0;
 
    if(scantype==SCAN_PPI) angle=serialmsg.az;
    else if(scantype==SCAN_RHI) angle=serialmsg.el;
    else if(scantype==SCAN_CSP) angle=serialmsg.el;

    if(angle > max_angle || angle < min_angle) return(1);
    else return(0);
}



//========================= M A I N   C O D E =======================
//            [ See disp_help() for command-line options ]
//-------------------------------------------------------------------
int main(int argc, char *argv[])
{
FILE *    pFile = NULL;
char      filename[255];
char      configfile[255]="/mnt/focus_radar_data/radar-galileo/etc/radar-galileo-iq.conf";
int       num_pulses;
int       amcc_fd = 0;        /* file descriptor for the PCICARD */
caddr_t   dma_buffer = NULL;  /* size of dma buffer */
short int *dma_banks[2];      /* pointers to the dma banks */
int       dma_bank = 0;
int       proc_bank = 1;
int       tcount;             /* number of bytes to be transferred during the DMA */
short int *data;
int       count,sample,samples_per_pulse;
int       total_samples;
int       year, month, day, hour, minute, second, centisecond; 
int       nspectra;
int       status;
long      num_data;
float     *current_PSD;
register  int  i,j;
int       temp_int = 0;
float     HH_moments[3];
float HH_noise_level;
int noisegate1,noisegate2;
time_t    system_time;
time_t    start_time = 0;
time_t    spectra_time = 0;
time_t    spectra_rapid_time = 0;
time_t    temp_time_t;
struct tm *time_ptr;
char      datestring[15];

/* time variables */
struct timeval		tv;
struct timezone 	tz;

PolPSDStruct	PSD[1000];
URC_ScanStruct scan;
RNC_DimensionStruct dimensions;

/* netCDF file pointer */
int ncid;
int spectra_ncid;
int spectra_rapid_ncid;
int PSD_varid[4];
int PSD_rapid_varid[4];
int file_stateid = 0;
int	scanEnd = (-1);


static      RSM_SerialMessageStruct m_serialmsg;

/* signal */
struct      sigaction sig_struct;

int nm;

// The following are shortcut pointers to the elements of
// the obs structure

short int *I_uncoded_copolar_H;
short int *Q_uncoded_copolar_H;

RSP_ParamStruct param;

int  newfile = 1;
int     obtain_index;
int     store_index;
IQStruct      IQStruct;

disp_welcome_message();


//---------------------------
// Set up the signal handler
//---------------------------
/* Set up the sig_struct variable */
sig_struct.sa_handler = sig_handler;
sigemptyset( &sig_struct.sa_mask );
sig_struct.sa_flags = 0;
/* Install signal handler and check for error */
if (sigaction(SIGINT, &sig_struct, NULL) != 0)
{
	perror ("Error installing signal handler\n");
	exit(1);
}

//------------------------
// Read radar config file
//------------------------
get_config(configfile,&param,&scan,0);  // Do it for coded pulses

//------------------------------
// Parse command line arguments
//------------------------------
// overwrite config with command line parameters
if( parseargs(argc,argv,&param,&scan) == 1 )
{
        exit(1);
}



// Read calibration file
//get_cal(&param, CAL_FILE);

//------------------------------------
// Initialise RSP parameter structure
//------------------------------------
RSP_InitialiseParams(&param); // This param is used for uncoded pulses

printf("Display parameters:\n");
RSP_DisplayParams(param);

// Sample extra pulses at end so that we have entire code sequence
num_pulses = (int)(param.pulses_per_daq_cycle);
tcount = num_pulses * param.spectra_averaged * param.samples_per_pulse * param.ADC_channels * sizeof(short int);

// Number of data points to allocate per data stream
num_data = param.pulses_per_daq_cycle*param.samples_per_pulse;

// Allocate memory for coded and uncoded data streams
I_uncoded_copolar_H        = (short int *)malloc(sizeof(short int) * num_data);
Q_uncoded_copolar_H        = (short int *)malloc(sizeof(short int) * num_data);
IQStruct.I_uncoded_copolar_H = (short int *)malloc(sizeof(short int) * param.samples_per_pulse * param.nfft * param.spectra_averaged);
IQStruct.Q_uncoded_copolar_H = (short int *)malloc(sizeof(short int) * param.samples_per_pulse * param.nfft * param.spectra_averaged);

//-------------------------------------------
// Set up the data acquisition 
//-------------------------------------------
printf("** Initialising ISACTRL...\n");
RDQ_InitialiseISACTRL( num_pulses * param.spectra_averaged, param.samples_per_pulse,
			 param.clock_divfactor, param.delay_clocks );
  
printf("** Initialising PCICARD...\n");
amcc_fd = RDQ_InitialisePCICARD_New( &dma_buffer, DMA_BUFFER_SIZE );
  
// Initialise pointers to DMA banks
dma_banks[ 0 ] = (short int *) dma_buffer;
dma_banks[ 1 ] = (short int *) (dma_buffer + (DMA_BUFFER_SIZE/2));
  
make_dmux_table(param.ADC_channels);
  
printf("** Starting acquisition...\n");

RDQ_StartAcquisition( amcc_fd, dma_bank, dma_banks[dma_bank], tcount);

//---------------------
// Wait for scan start
//---------------------
if(scan.scanType==SCAN_PPI || scan.scanType==SCAN_RHI || scan.scanType==SCAN_CSP){
	wait_scan_start(scan.scanType,scan.min_angle,scan.max_angle);
}

 total_samples = (int)( param.samples_per_pulse * param.nfft );

// THIS IS THE START OF THE OUTER RAY LOOP
while( exit_now == 0)
{ 
	printf("\n<< PRESS CTRL-C TO EXIT >>\n");

	/* get timeofday */
	gettimeofday( &tv, &tz);
      	time_ptr = gmtime(&(tv.tv_sec));
      	year = time_ptr->tm_year+1900;
      	month= time_ptr->tm_mon+1;
      	day  = time_ptr->tm_mday;
      	hour = time_ptr->tm_hour;
      	minute=time_ptr->tm_min;
      	second=time_ptr->tm_sec;
      	centisecond=(int)tv.tv_usec/10000;
      	sprintf(datestring,"%04d%02d%02d%02d%02d%02d%02d",year,month,day,hour,minute,second,centisecond);
      	printf("Date time: %s\n",datestring);

      system_time=time(NULL);
      if  ( system_time > start_time + 60 ) 
                     newfile=1;

      if (newfile==1) {
           if (pFile != NULL)
                 fclose(pFile);
                
            start_time=time(NULL);
            sprintf(filename,"/mnt/focus_radar_data/radar-galileo/raw/ts/%s_iqdata.bin",datestring);
            pFile=fopen(filename,"wb");
            fwrite(&param.samples_per_pulse , sizeof(int), 1, pFile);
            fwrite(&param.nfft , sizeof(int), 1, pFile);
            newfile=0; 
          }


		/* wait for data acquisition to complete */
		status = RDQ_WaitForAcquisitionToComplete( amcc_fd );
                if (status != 0) printf("There was a problem in WaitForAcquisitionToComplete\n");
		//----------------------------------------------------------------
                // Swap around the areas used for storing daq and processing from
                //----------------------------------------------------------------
                dma_bank = 1 - dma_bank;
                proc_bank = 1 - proc_bank;

                data = dma_banks[proc_bank];
                RDQ_StartAcquisition( amcc_fd, dma_bank, dma_banks[dma_bank], tcount);
	
	    	//----------------------------------------------------------------
	    	// Extract data from DMA memory
	    	//----------------------------------------------------------------
	    	for (i = 0; i < num_pulses; i++) 
		{
              		register int count_reg;
	      		for (j = 0; j < param.samples_per_pulse; j++) 
			{
				count_reg = (i * param.samples_per_pulse) + j;
				I_uncoded_copolar_H[count_reg] = GET_CHANNEL( data, CHAN_Ic );
				Q_uncoded_copolar_H[count_reg] = GET_CHANNEL( data, CHAN_Qc );
				INC_POINTER ( data, param.ADC_channels);
      			}
    		} 

         fwrite(&year , sizeof(int), 1, pFile);
         fwrite(&month , sizeof(int), 1, pFile);
         fwrite(&day , sizeof(int), 1, pFile);
         fwrite(&hour , sizeof(int), 1, pFile);
         fwrite(&minute , sizeof(int), 1, pFile);
         fwrite(&second , sizeof(int), 1, pFile);
         fwrite(&centisecond , sizeof(int), 1, pFile);

                    	/* store I and Q for each pulse */
                       	/* nspectra defines the spectra number */
               	for (i = 0; i < param.nfft; i++) {
                       	obtain_index = i * param.samples_per_pulse;
                       	store_index = (i * param.samples_per_pulse) + (nspectra * param.samples_per_pulse * param.nfft);
                       	for (j = 0; j < param.samples_per_pulse; j++) {
                               	IQStruct.I_uncoded_copolar_H[store_index] = I_uncoded_copolar_H[obtain_index];
                               	IQStruct.Q_uncoded_copolar_H[store_index] = Q_uncoded_copolar_H[obtain_index];
                               	store_index = store_index + 1;
                               	obtain_index = obtain_index + 1;
                       	}
               	}
              
        fwrite(IQStruct.I_uncoded_copolar_H , sizeof(short), total_samples, pFile);
        fwrite(IQStruct.Q_uncoded_copolar_H , sizeof(short), total_samples, pFile);

        printf("completed storing IQs\n");

} 
 
  
// Finish off
printf("*** Closing PCICARD...\n");
RDQ_ClosePCICARD_New( amcc_fd, &dma_buffer, DMA_BUFFER_SIZE );
  
// Close binary data file
fclose(pFile);

//---------------------------
// Unallocate all the memory
//---------------------------

RSP_FreeMemory(param);  // Free memory allocated by RSP package

free(IQStruct.I_uncoded_copolar_H);
free(IQStruct.Q_uncoded_copolar_H);

//=========
// THE END
//=========
printf("All done.\n");
return(0);

}

