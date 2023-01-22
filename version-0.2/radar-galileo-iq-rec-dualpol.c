#define VERSION_NUMBER "0.3"

// radar-galileo-iq-rec-dualpol.c
// Development Version

///----------------------------------------------------------------
/// Acquisition software for dual-pol IQ data from 94-GHz radar
/// Chris Walden - December 2022
/// Uses code developed by John Nicol (Univ. Reading)
/// and on additional code by Mark Fortescue (Control Loop Concepts)
/// Mark Fortescue - December 2019
/// John Nicol - April 2014
/// Based on radar-galileo-rec (version 0.6)
/// Owain Davies / Ed Pavelin - May 2004
///
/// ---------------------------------------------------------------
/// REVISION HISTORY
/// ---------------------------------------------------------------
/// 08/04/14: JCN: Created v0.1
/// 13/03/15: JCN: Dual-pol v0.2
/// 28/11/22: CJW: Start to implement mode switching for WIVERN
/// 26/12/22: CJW: Finalising code for WIVERNex_UK campaign

#define NDEBUG

#include <stdio.h>
#include <stdbool.h>
#include <netcdf.h>
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

#ifdef HAVE_DISLIN
#include <dislin.h>
#endif /* HAVE_DISLIN */

/* PCI DIO24 card stuff - used to control mode switching */
#include <ctype.h>
#include <stdint.h>
#include <sys/ioctl.h>

#include "ixpio.h"

char *device = "/dev/ixpio1";

#include <radar.h> // Master header file for the Universal Radar Code
#include <RSP.h>   // Include file for the RSP package
#include <RDQ.h>   // Include file for the RDQ package
#include <RNC.h>   // Include file for the RNC package
#include <RSM.h>   // Include file for the RSM package
#include <RTS.h>   // Include file for the RTS package

#include "radar-galileo-iq-rec-dualpol.h"

#define RSP_MOMENTS 3 /* Original is 3. Now 5 is also supported. */

/* below defines the com port that the serial message arrives on */
#define SERIALMESSAGE_PORT "/dev/ttyS1"

typedef struct TimeSeriesObs_t
{
	int azimuthid;
	int elevationid;
	int tsid;
	int rayend_tsid;
	int dish_tsid;
	int pulse_modeid;
	int IHid;
	int QHid;
	int IVid;
	int QVid;
	int TxPower1id;
	int TxPower2id;
	int VnotHid;
	int RawLogid;

	uint16_t *IH;
	uint16_t *QH;
	uint16_t *IV;
	uint16_t *QV;
	uint16_t *TxPower1;
	uint16_t *TxPower2;
	uint16_t *VnotH;
	uint16_t *RawLog;
} TimeSeriesObs_t;

#if 0
typedef struct StringStruct_t
{
	uint32_t len;
	char *string;
} StringStruct_t;

StringStruct_t InitialiseString(char * string)
{
	uint32_t stringsize = strlen(string)+1;
	char
}

void fwrite_uint16(fid, uint16_t val)
{
	uint8_t bytes[2];
	bytes[0] = (val)&0xFF;		  // low byte
	bytes[1] = (val >> 8) & 0xFF; // high byte
	fwrite((char *)bytes, 2);
}

void fwrite_uint32(fid, uint16_t val)
{
	uint8_t bytes[4];
	bytes[0] = (val) & 0xFF;
	bytes[1] = (val >> 8) & 0xFF;
	bytes[2] = (val >> 16) & 0xFF;
	bytes[3] = (val >> 24) & 0xFF;
	fwrite((char *)bytes, 4);
}
#endif

#if 0
typedef struct TimeSeriesBinaryHeader_t
{
	char[10] file_signature = "chilrad-ts";
	uint16_t major_version;
	uint16_t minor_version;
	string_str file_creation_timestamp;
	string_str radar = {};
	string_str history;
	string_str scantype;
	string_str operator;
	string_str project_name;
	string_str project_tag;
	uint32_t adc_clock;
	string_str adc_clock_units = "Hz" 
	u32 adc_channels 8 u32 adc_clock_divfactor 2 u32 adc_delay_clocks 2 u32 adc_bits_per_sample 12 u32 samples_per_pulse 200 u32 pulses_per_ray 6144 u32 pulse_width
		string_str pulse_width_units "nanosec" u32 radar_frequency 94008000000 string_str radar_frequency_units "Hz" u32 prf 6250 string_str prf_units "Hz" u32 transmit_power 1600 string_str transmit_power_units "watt" u32 antenna_diameter 460 string_str antenna_diameter_units "mm" u32 antenna_focal_length
			string_str antenna_focal_length_units "mm" u32 beamwidth_h 30 string_str beamwidth_h_units "arcminute" u32 beamwidth_v 30 string_str beamwidth_v_units "arcminute" u32 pulse_offset 20000 string_str pulse_offset_units "nanosecond" u8 alternating_modes 1 u8 long_pulse_mode 0 u8 mode0 6 u8 mode1 3 u32 nrays_mode0 1 u32 nrays_mode1 1 u32 azimuth_offset 0 string_str azimuth_offset_units arcminute
				u32 elevation_offset 0 string_str elevation_offset_units arcminute
					u32 antenna_ellipsoidal_altitude 8500 string_str antenna_ellipsoidal_altitude_units "cm" u32 antenna_altitude_agl
						string_str antenna_altitude_agl_units "mm" u32 latitude
							string_str latitude_units "arcsecond_north" u32 longitude
								string_str longitude_units "arcsecond_east"

} TimeSeriesBinaryFileHeader_t;
#endif

static void sig_handler(int sig);

static void SetupTimeSeriesVariables(TimeSeriesObs_t *obs, int ncid,
									 RSP_ParamStruct *param, URC_ScanStruct *scan,
									 RNC_DimensionStruct *dimensions,
									 RSP_ObservablesStruct *posobs);

static void WriteTimeSeriesBinaryHeader(FILE *tsbinfid, struct timespec *tspec, int radar,
										const URC_ScanStruct *scan,
										const RSP_ParamStruct *param,
										int argc, char *const argv[]);

/* in declaration below the moment variable is for compatibility with
 * possible changes to allow averaging over moments, and would index
 * each moment in the average */
static void WriteOutTimeSeriesData(int ncid, const RSP_ParamStruct *param,
								   RSP_ObservablesStruct *posobs,
								   TimeSeriesObs_t *obs, int moment);

static void WriteOutTimeSeriesDataBinary(FILE *tsbinfid, const RSP_ParamStruct *param,
								   RSP_ObservablesStruct *posobs,
								   TimeSeriesObs_t *obs, int moment);

#ifdef HAVE_DISLIN
static float zmat[256][250];
static int counter_x, counter_y;
#endif

//-----------------------------
// GLOBAL VARIABLE DEFINITIONS
//-----------------------------

int dmux_table[8];

/*---------------------------------------------------*
 * used for communication between the signal handler *
 * and the main program                              *
 *---------------------------------------------------*/
static bool exit_now = false;

/* Defaults for command line switches */
static bool debug = false;
static bool swap_iq_channels = false;
static bool tsdump = true;
static bool TextTimeSeries = false;
static bool NetCDFTimeSeries = false;

/* Disable position message for fixed position operation */
static bool positionMessageAct = false; // default is OFF

// Displays a welcome message with version information
static inline void
disp_welcome_message(void)
{
	printf("\nradar-galileo-iq-rec-dualpol: Version %s\n\n", VERSION_NUMBER);
}

static void
sig_handler(int sig)
{
	if (!exit_now)
	{
		exit_now = true;
		printf("***********************************\n");
		printf("* Received signal %d. Exiting soon *\n", sig);
		printf("***********************************\n");
	}
}

/*!
 * \brief Displays help on command-line arguments
 *
 * \param prog
 */
static inline void
disp_help(const char *prog)
{
	const char *pt = strrchr(prog, '/');

	if (pt++ != NULL)
		prog = pt;

	printf("Usage: %s [options]\n\n", prog);
	printf("Options:\n");
	printf("--------\n");
	// printf ("\n Configuration options (how to record the data)\n");
	// printf (" ----------------------------------------------\n");
	// printf (" -config filename      Specify alternative config file\n");
	// printf (" -phase                Record absolute phase and std. dev. of phase\n");
	// printf (" -rawzed               Switch off range normalisation and calibration for Z\n");
	// printf (" -spec                 Record power spectra\n");
	printf(" -tsdump               Dump time series to a binary file\n");
	printf(" -tsdump.nc           Dump time series to a NetCDF file\n");
	printf(" -tsdump.txt           Dump time series to a text file\n");
	printf(" -tssamples <n>        Only dump first <n> time series samples. 20 < n <= <max-gates>\n");
	printf(" -tsrange <n>          Only dump first <n> km of time series samples. 1.2 < n <= <max-range>\n");
	printf(" -position-msg         Enable 25m Antenna position message reading\n");
	printf(" -long-pulse           Enable single mode long-pulses\n");
	printf(" -alt_modes            Enable alternating mode short-pulses\n");
	printf(" -mode0 <mode>         Set long, short or alternating mode0 mode\n");
	printf(" -mode1 <mode>         Set alternating mode1 mode\n");
	printf(" -nrays_mode0 <rays>   Set number of rays in mode0 for alternating modes\n");
	printf(" -nrays_mode1 <rays>   Set number of rays in mode1 for alternating modes\n");
	printf(" -pulses <num>         Set number of pulses per ray: New not implemented yet\n");
	printf(" -cellwidth <range>    Set cell size (m)           : New not implemented yet\n");
	printf(" -range <min> <max>    Set min/max range (km)\n");
	printf(" -debug                Enable extra output during acquisition\n");
	printf(" -swap                 Swap Co/Cross I/Q ADC data\n");
	// printf (" -quiet                Do not generate lots of text output\n");
	printf("\n Scanning options (what scan to expect)\n");
	printf(" --------------------------------------\n");
	printf(" -ppi az1 az2          Record PPI scan\n");
	printf(" -rhi el1 el2          Record RHI scan\n");
	printf(" -fix n az1 el1        Fixed dwell for n seconds\n");
	printf(" -csp el1 el2          Record CSP scan\n");
	printf(" -single az1 el1       Record a single ray\n");
	printf(" -man az1 el1          Record Manual/Tracking scan\n");
	printf(" -track az1 el1        Record Manual/Tracking scan\n");
	printf(" -cal az1 el1          Record Calibration scan\n");
	printf(" -c   az1 el1          Record Calibration scan\n");
	printf("\n Control block options (stored in NetCDF output file)\n");
	printf(" ----------------------------------------------------\n");
	printf(" -sv vel               Specify scan velocity\n");
	printf(" -scan_angle el1       Specify scan angle for PPI scan\n");
	printf(" -scan_angle az1       Specify scan angle for RHI/CSP scan\n");
	printf(" -file nnnn            Specify file (tape) number: Obsolete\n");
	printf(" -scan nnnn            Specify scan number\n");
	printf(" -id nn                Specify experiment ID\n");
	printf(" -date yyyymmddhhmmss  Specify scan date\n");
	printf(" -op xxx               Specify radar operator\n");
	printf("\n");
}

/*!
 * \brief
 *
 * \param argc
 * \param argv
 * \param param
 * \param scan
 * \param start_day
 * \return int
 */
static inline int
parseargs(int argc, char *argv[], RSP_ParamStruct *param, URC_ScanStruct *scan, int *start_day)
{
	/* Parse command line args */

	time_t system_time;
	struct tm tm;
	const char *operator;
	int i;

	/*-------------------------------------*
	 * Initialise defaults for scan params *
	 *-------------------------------------*/
	scan->scanType = SCAN_FIX;
	scan->file_number = 0;
	scan->scan_number = 0;
	scan->experiment_id = 0;
	scan->scan_velocity = -9999;
	scan->dwelltime = -1;

	param->samples_per_pulse_ts = param->samples_per_pulse;

	operator= getenv("USER");
	if (operator== NULL)
		operator= getenv("USERNAME");
	if (operator== NULL)
		operator= getenv("LOGNAME");
	if (operator== NULL)
		operator= "<unknown>";
	strncpy(scan->operator, operator, sizeof(scan->operator) - 1);
	scan->operator[sizeof(scan->operator) - 1] = '\0';

	system_time = time(NULL);
	gmtime_r(&system_time, &tm);
	strftime(scan->date, sizeof(scan->date), "%Y%m%d%H%M%S", &tm);
	*start_day = tm.tm_mday;

	/* by default real_time_spectra_display is off */
	param->real_time_spectra_display = 0;

	/*--------------------------------------*
	 * Now parse any command line arguments *
	 *--------------------------------------*/
	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-real_time_spectra_display"))
		{
#ifdef HAVE_DISLIN
			printf("Real time spectra display enabled\n");
			param->real_time_spectra_display = 1;
#else
			printf("Real time spectra display not available\n");
#endif /* HAVE_DISLIN */
		}
		else if (!strcmp(argv[i], "-ppi"))
		{
			/* -------- *
			 * PPI SCAN *
			 * -------- */
			scan->min_angle = atof(argv[++i]);
			scan->max_angle = atof(argv[++i]);
			printf("PPI Scan %f-%f deg\n", scan->min_angle, scan->max_angle);
			scan->scanType = SCAN_PPI;
		}
		else if (!strcmp(argv[i], "-rhi"))
		{
			/* -------- *
			 * RHI SCAN *
			 * -------- */
			scan->min_angle = atof(argv[++i]);
			scan->max_angle = atof(argv[++i]);
			printf("RHI Scan %f-%f deg\n", scan->min_angle, scan->max_angle);
			scan->scanType = SCAN_RHI;
		}
		else if (!strcmp(argv[i], "-csp"))
		{
			/* ---------------- *
			 * SLANT PLANE SCAN *
			 * ---------------- */
			scan->min_angle = atof(argv[++i]);
			scan->max_angle = atof(argv[++i]);
			printf("CSP Scan %f-%f deg\n", scan->min_angle, scan->max_angle);
			scan->scanType = SCAN_CSP;
		}
		else if (!strcmp(argv[i], "-fix"))
		{
			/* ----------- *
			 * FIXED DWELL *
			 * ----------- */
			scan->dwelltime = atof(argv[++i]);
			scan->scan_angle = atof(argv[++i]);
			scan->min_angle = atof(argv[++i]);
			printf("Fixed dwell for %f seconds\n", scan->dwelltime);
			printf("Position Az: %f, El: %f deg\n",
				   scan->scan_angle, scan->min_angle);
			scan->scanType = SCAN_FIX;
			scan->max_angle = scan->min_angle;
			scan->scan_velocity = 0;
		}
		else if (!strcmp(argv[i], "-single"))
		{
			/* ---------------- *
			 * FIXED SINGLE RAY *
			 * ---------------- */
			scan->scan_angle = atof(argv[++i]);
			scan->min_angle = atof(argv[++i]);
			printf("Fixed single ray\n");
			printf("Position Az: %f, El: %f deg\n", scan->scan_angle, scan->min_angle);
			scan->scanType = SCAN_SGL;
			scan->max_angle = scan->min_angle; /* Make weird semantics consistant */
			scan->scan_velocity = 0;
		}
		else if (!strcmp(argv[i], "-man") ||
				 !strcmp(argv[i], "-track"))
		{
			/* ------------- *
			 * TRACKING SCAN *
			 * ------------- */
			scan->scan_angle = atof(argv[++i]);
			scan->min_angle = atof(argv[++i]);
			printf("Track Scan %f-%f deg\n", scan->scan_angle, scan->min_angle);
			scan->scanType = SCAN_MAN;
			scan->max_angle = scan->min_angle; /* Make weird semantics consistant */
		}
		else if (!strcmp(argv[i], "-cal") ||
				 !strcmp(argv[i], "-c"))
		{
			/* ---------------- *
			 * CALIBRATION SCAN *
			 * ---------------- */
			scan->scan_angle = atof(argv[++i]);
			scan->min_angle = atof(argv[++i]);
			printf("Calibration Scan %f-%f deg\n", scan->scan_angle, scan->min_angle);
			scan->scanType = SCAN_CAL;
			scan->max_angle = scan->min_angle; /* Make weird semantics consistant */
		}
		else if (!strcmp(argv[i], "-position-msg"))
		{
			/* ---------------------------- *
			 * 25m Antenna POSITION MESSAGE *
			 * ---------------------------- */
			positionMessageAct = true;
			printf("25m Antenna position message enabled\n");
		}
		else if (!strcmp(argv[i], "-sv"))
		{
			/* ------------- *
			 * SCAN VELOCITY *
			 * ------------- */
			scan->scan_velocity = atof(argv[++i]);
		}
		else if (!strcmp(argv[i], "-file"))
		{
			/* ----------- *
			 * FILE NUMBER *
			 * ----------- */
			printf("%s %s is obsolete\n", argv[i], argv[i + 1]);
			scan->file_number = atoi(argv[++i]);
			printf("File number   : %04d\n", scan->file_number);
		}
		else if (!strcmp(argv[i], "-scan"))
		{
			/* ----------- *
			 * SCAN NUMBER *
			 * ----------- */
			scan->scan_number = atoi(argv[++i]);
			printf("Scan number   : %04d\n", scan->scan_number);
		}
		else if (!strcmp(argv[i], "-id"))
		{
			/* ------------- *
			 * EXPERIMENT ID *
			 * ------------- */
			scan->experiment_id = atoi(argv[++i]);
			printf("Experiment id : %d\n", scan->experiment_id);
		}
		else if (!strcmp(argv[i], "-scan_angle"))
		{
			/* ---------- *
			 * SCAN ANGLE *
			 * ---------- */
			scan->scan_angle = atof(argv[++i]);
			printf("Scan angle : %f\n", scan->scan_angle);
		}
		else if (!strcmp(argv[i], "-date"))
		{
			/* ---- *
			 * DATE *
			 * ---- */
			/* MTF: Fix buffer overrun when date is too big */
			strncpy(scan->date, argv[++i], sizeof(scan->date) - 1);
			scan->date[sizeof(scan->date) - 1] = '\0';
		}
		else if (!strcmp(argv[i], "-op"))
		{
			/* -------- *
			 * OPERATOR *
			 * -------- */
			/* MTF: Fix buffer overrun when operator id is too big */
			strncpy(scan->operator, argv[++i], sizeof(scan->operator) - 1);
			scan->operator[sizeof(scan->operator) - 1] = '\0';
		}
		else if (!strcmp(argv[i], "-long_pulse"))
		{
			/* -------------------- *
			 * LONG PULSE Selection *
			 * -------------------- */
			param->long_pulse_mode = 1; /* Changed to boolena arg */
		}
		else if (!strcmp(argv[i], "-alt_modes"))
		{
			/* --------------------- *
			 * ALTERNATE PULSE modes *
			 * --------------------- */
			param->alternate_modes = 1; /* Changed to boolena arg */
		}
		else if (!strcmp(argv[i], "-mode0"))
		{
			/* ----------------- *
			 * MODE 0 Pulse Mode *
			 * ----------------- */
			param->mode0 = atoi(argv[++i]);
		}
		else if (!strcmp(argv[i], "-mode1"))
		{
			/* ----------------- *
			 * MODE 1 Pulse Mode *
			 * ----------------- */
			param->mode1 = atoi(argv[++i]);
		}
		else if (!strcmp(argv[i], "-nrays_mode0"))
		{
			/* ---------------------- *
			 * MODE 0 Pulse Ray count *
			 * ---------------------- */
			param->nrays_mode0 = atoi(argv[++i]);
		}
		else if (!strcmp(argv[i], "-nrays_mode1"))
		{
			/* ---------------------- *
			 * MODE 1 Pulse Ray count *
			 * ---------------------- */
			param->nrays_mode1 = atoi(argv[++i]);
		}
		else if (!strcmp(argv[i], "-debug"))
		{
			/* ---------------------------- *
			 * Output Debugging information *
			 * ---------------------------- */
			debug = true;
		}
		else if (!strcmp(argv[i], "-swap"))
		{
			/* ---------------------------------- *
			 * Swap Co/Cross polar ADC I/Q inputs *
			 * ---------------------------------- */
			swap_iq_channels = true;
		}
		else if (!strcmp(argv[i], "-tsdump"))
		{
			/* ------------------ *
			 * TIME SERIES SWITCH *
			 * ------------------ */
			tsdump = true;
			TextTimeSeries = false;
			printf("Time series recording on");
		}
		else if (!strcmp(argv[i], "-tsdump.txt"))
		{
			/* ------------------ *
			 * TIME SERIES SWITCH *
			 * ------------------ */
			tsdump = true;
			TextTimeSeries = true;
			printf("Text time series recording on");
		}
		else if (!strcmp(argv[i], "-tsdump.nc"))
		{
			/* ------------------ *
			 * TIME SERIES SWITCH *
			 * ------------------ */
			tsdump = true;
			NetCDFTimeSeries = true;
			printf("NetCDF time series recording on");
		}
		else if (!strcmp(argv[i], "-tssamples"))
		{
			int samples;

			samples = atoi(argv[++i]);
			if (samples < 20)
			{
				printf("Invalid number of gates for time series recording\n");
				return -1;
			}
			param->samples_per_pulse_ts = samples;
		}
		else if (!strcmp(argv[i], "-tsrange"))
		{
			const double gate_width = (double)param->clock_divfactor * (SPEED_LIGHT / 2.0) / param->clock;
			int samples;

			samples = (int)((atof(argv[++i]) * 1000.0 / gate_width) + 0.5);
			if (samples < 20)
			{
				printf("Invalid range for time series recording\n");
				return -1;
			}
			param->samples_per_pulse_ts = samples;
		}
		else if (!strcmp(argv[i], "-range"))
		{
			const double gate_width = (double)param->clock_divfactor * (SPEED_LIGHT / 2.0) / param->clock;
			int min_gate;
			int max_gate;

			/* ------------------------ *
			 * SELECT MIN AND MAX GATES *
			 * ------------------------ */
			min_gate = (int)(atof(argv[++i]) * 1000.0 / gate_width);
			max_gate = (int)((atof(argv[++i]) * 1000.0 / gate_width) + 0.5);
			if (min_gate < 2)
				min_gate = 2;

			/*
			 * For Galileo this is currently fixed in config file
			 * so override any value entered.
			 */
			min_gate = (param->delay_clocks / param->clock_divfactor) + 1;

			if (max_gate <= min_gate)
				max_gate = min_gate + 1;

			scan->min_gate = min_gate;
			scan->max_gate = max_gate;
			printf("Range: %8.3f to %8.3f km\n",
				   scan->min_gate * gate_width / 1000.0,
				   scan->max_gate * gate_width / 1000.0);

			param->delay_clocks = (scan->min_gate - 1) * param->clock_divfactor;
			param->samples_per_pulse = scan->max_gate - scan->min_gate;
		}
		else if (!strcmp(argv[i], "-help"))
		{
			disp_help(argv[0]);
			return 1;
		}
		else
		{
			/* ------------- *
			 * PARSING ERROR *
			 * ------------- */
			printf("** UNKNOWN COMMAND LINE ARGUMENT '%s'!\n", argv[i]);
			disp_help(argv[0]);
			return -1;
		}
	}

	/* Make sure time series samples <= samples */
	if (param->samples_per_pulse_ts > param->samples_per_pulse)
	{
		param->samples_per_pulse_ts = param->samples_per_pulse;
	}

	/* -------------------------------------------------------------*
	 * This is to remove the effect of Chilbolton azimuths          *
	 * care may have to be taken when the radar is not on the dish  *
	 * since this may cause an error in the metadata                *
	 * -------------------------------------------------------------*/
	if (scan->scanType == SCAN_PPI)
	{
		scan->min_angle -= 90.0;
		scan->max_angle -= 90.0;
	}
	else
	{
		scan->scan_angle -= 90.0;
	}
	return 0;
}

/*!
 * \brief Read the radar config file
 *
 * \param filename
 * \param param
 * \param scan
 * \param is_coded
 */
static inline void
get_config(char *filename, RSP_ParamStruct *param, URC_ScanStruct *scan, int is_coded)
{
	char codefile[255];

	printf("Accessing config file: %s\n", filename);

	param->frequency = RNC_GetConfigDouble(filename, "radar-frequency");
	param->prf = RNC_GetConfigDouble(filename, "prf");
	param->transmit_power = RNC_GetConfigDouble(filename, "transmit-power");
	param->pulses_per_daq_cycle = RNC_GetConfigDouble(filename, "pulses");
	param->samples_per_pulse = RNC_GetConfigDouble(filename, "samples");
	param->ADC_channels = RNC_GetConfigDouble(filename, "adc-channels");
	param->clock_divfactor = RNC_GetConfigDouble(filename, "adc-divfactor");
	param->delay_clocks = RNC_GetConfigDouble(filename, "adc-delayclocks");
	param->pulse_period = RNC_GetConfigDouble(filename, "chip-length");
	param->pulses_coherently_averaged = RNC_GetConfigDouble(filename, "num-coh-avg");
	param->spectra_averaged = RNC_GetConfigDouble(filename, "num-spec-avg");
	param->moments_averaged = RNC_GetConfigDouble(filename, "num-moments-avg");
	param->fft_bins_interpolated = RNC_GetConfigDouble(filename, "reject-clutter-bins");
	param->clock = RNC_GetConfigDouble(filename, "adc-clock");
	param->num_peaks = RNC_GetConfigDouble(filename, "num-peaks");

	param->antenna_diameter = RNC_GetConfigFloat(filename, "antenna_diameter");
	param->beamwidthH = RNC_GetConfigFloat(filename, "beamwidthH");
	param->beamwidthV = RNC_GetConfigFloat(filename, "beamwidthV");
	param->height = RNC_GetConfigFloat(filename, "height");
	param->azimuth_offset = RNC_GetConfigFloat(filename, "azimuth_offset");
	param->dump_spectra = RNC_GetConfigFloat(filename, "dump_spectra");
	param->dump_spectra_rapid = RNC_GetConfigFloat(filename, "dump_spectra_rapid");
	param->num_interleave = RNC_GetConfigFloat(filename, "num-interleave");
	param->num_tx_pol = RNC_GetConfigFloat(filename, "num-tx-pol");
	param->pulse_offset = RNC_GetConfigFloat(filename, "pulse_offset");
	param->phidp_offset = RNC_GetConfigFloat(filename, "phidp_offset");

	param->long_pulse_mode = (int)RNC_GetConfigFloat(filename, "long_pulse_mode");
	param->alternate_modes = (int)RNC_GetConfigFloat(filename, "alternate_modes");
	param->mode0 = (int)RNC_GetConfigFloat(filename, "mode0");
	param->mode1 = (int)RNC_GetConfigFloat(filename, "mode1");
	param->nrays_mode0 = (int)RNC_GetConfigFloat(filename, "nrays_mode0");
	param->nrays_mode1 = (int)RNC_GetConfigFloat(filename, "nrays_mode1");

	/* this is for when the radar is fixed pointing in the cradle */
	/* please take care when the radar is tilted */
	scan->min_angle = RNC_GetConfigFloat(filename, "antenna_elevation");
	scan->max_angle = RNC_GetConfigFloat(filename, "antenna_elevation");
	scan->scan_angle = RNC_GetConfigFloat(filename, "antenna_azimuth");

	RNC_GetConfig(filename, "code-file", codefile, sizeof(codefile));

	strcpy(param->code_name, "NOT YET IMPLEMENTED\0");

	/* For non-coded pulses */
	param->code_length = 1;
	param->number_of_codes = 1;
}

//
/*!
 * \brief Loads calibration information from *.cal file
 *
 * \param param
 * \param calfile
 */
static inline void
get_cal(RSP_ParamStruct *param, char *calfile)
{
	param->ZED_calibration_offset = RNC_GetConfigDouble(calfile, "z-calibration");
	param->ZDR_calibration_offset = RNC_GetConfigDouble(calfile, "zdr-calibration");
	param->LDR_calibration_offset = RNC_GetConfigDouble(calfile, "ldr-calibration");
	param->ZED_incoherent_calibration_offset = RNC_GetConfigDouble(calfile, "z-incoherent-calibration");
	param->ZED_incoherent_noise = RNC_GetConfigDouble(calfile, "z-incoherent-noise");
	param->range_offset = RNC_GetConfigDouble(calfile, "range-offset");
}

/*!
 * \brief Generate lookup table used to extract channels from DMA buffer
 *
 * \param channels 		the number of channels
 * \param dmux_table
 */
static inline void
make_dmux_table(int channels, int dmux_table[8])
{
	if (channels == 4)
	{
		dmux_table[0] = 0;
		dmux_table[1] = 2;
		dmux_table[2] = 1;
		dmux_table[3] = 3;
		/*
		 * Channels 4 to 7 do not exist in the 4-channel system.
		 *
		 * MTF: They need to be allocated to a valid channel as
		 * the usage macros do not check the index returned from
		 * the dmux_table before using it.
		 */
		dmux_table[4] = 3; /* Needs to be in range 0..3 */
		dmux_table[5] = 3; /* Needs to be in range 0..3 */
		dmux_table[6] = 3; /* Needs to be in range 0..3 */
		dmux_table[7] = 3; /* Needs to be in range 0..3 */
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

/*!
 * \brief Routine to wait for start of scan
 *
 * \param scantype
 * \param position_msg
 * \param min_angle
 * \param max_angle
 */
static inline void
wait_scan_start(int scantype, RSM_PositionMessageStruct *position_msg, float min_angle, float max_angle)
{
	float angle;

	switch (scantype)
	{
	case SCAN_PPI:
	case SCAN_RHI:
	case SCAN_CSP:
		break;
	default:
		return;
	}

	printf("Waiting to get outside scan range...\n");
	do
	{
		RSM_ReadPositionMessage(position_msg);
		switch (scantype)
		{
		case SCAN_PPI:
			angle = position_msg->az;
			break;
		case SCAN_RHI:
			angle = position_msg->el;
			break;
		case SCAN_CSP:
			angle = position_msg->el;
			break;
		default:
			angle = 0.0;
			break;
		}
	} while (angle >= min_angle && angle <= max_angle && !exit_now);

	if (exit_now)
	{
		return;
	}

	printf("Waiting to get within scan range...\n");
	printf("min_angle: %.1f degrees  max_angle: %.1f\n",
		   min_angle, max_angle);
	do
	{
		RSM_ReadPositionMessage(position_msg);
		switch (scantype)
		{
		case SCAN_PPI:
			angle = position_msg->az;
			break;
		case SCAN_RHI:
			angle = position_msg->el;
			break;
		case SCAN_CSP:
			angle = position_msg->el;
			break;
		default:
			angle = 0.0;
			break;
		}
	} while ((angle < min_angle || angle > max_angle) && !exit_now);
}

/*!
 * \brief Routine to wait for end of scan
 *
 * \param scantype
 * \param position_msg
 * \param min_angle
 * \param max_angle
 * \return true
 * \return false
 */
static inline bool
scanEnd_test(int scantype, RSM_PositionMessageStruct *position_msg, float min_angle, float max_angle)
{
	float angle;

	switch (scantype)
	{
	case SCAN_PPI:
		angle = position_msg->az;
		break;
	case SCAN_RHI:
		angle = position_msg->el;
		break;
	case SCAN_CSP:
		angle = position_msg->el;
		break;
	case SCAN_SGL:
		return true;
	default:
		return false;
	}

	return (angle > max_angle || angle < min_angle);
}

/*------------------------------------------------------------------------*
 * Routines for mode-switching bitmask                                    *
 * -----------------------------------------------------------------------*/

static uint16_t
dec2bcd_r(uint16_t dec)
{
	return (dec) ? ((dec2bcd_r(dec / 10) << 4) + (dec % 10)) : 0;
}

#if 0
static inline char *
ul16toBinary (uint16_t a)
{
	char bitmask[] = "0000000000000000";
	uint16_t i;

	int j = 0;
	for (i = 0x8000; i != 0; i >>= 1)
	{
		bitmask[j]= (a & i) ? '1' : '0';
		j++;
	}
	return strdup (bitmask);
}

static inline char *
ul8toBinary (uint8_t a)
{
	char bitmask[] = "00000000";
	uint8_t i;

	int j = 0;
	for (i = 0x80; i != 0; i >>= 1)
	{
		bitmask[j]= (a & i) ? '1' : '0';
		j++;
	}
	return strdup (bitmask);
}
#endif

#if 0
/* Routine to calculate incoherent power */
static inline float
calc_incoherent_power(fftw_complex *in, int nfft)
{
	double power = 0.0;
	register int i;

	for (i = 0; i < nfft; i++)
	{
		/* cabs (in[i]) * cabs (in[i]) only doing it directly avoids 2*sqrt() */
		power += (fftw_real(in[i]) * fftw_real(in[i]) +
				  fftw_imag(in[i]) * fftw_imag(in[i]));
	}
	power /= nfft;
	return power;
}
#endif

//========================= M A I N   C O D E =======================
//            [ See disp_help() for command-line options ]
//-------------------------------------------------------------------
int main(int argc, char *argv[])
{
#ifdef USE_HOST_EXT
	const char *host_ext = USE_HOST_EXT;
#else  /* USE_HOST_EXT */
	const char *host_ext = NULL;
#endif /* USE_HOST_EXT */

	/* DIO card */
	int fd;
	ixpio_reg_t bank_A, bank_B, bank_C, bank_0;

	/* For binary time series file */
	FILE *tsbinfid = NULL;
	char filename[255];

	int num_pulses;
	int amcc_fd = 0;		   /* file descriptor for the PCICARD */
	caddr_t dma_buffer = NULL; /* size of dma buffer */
	uint16_t *dma_banks[2];	   /* pointers to the dma banks */
	int dma_bank = 0;
	int proc_bank = 1;
	int tcount; /* number of bytes to be transferred during the DMA */
	uint16_t *data;

	int count, sample, samples_per_pulse;
	int total_samples;
	int start_day = 0;
	// int year, month, day, hour, minute, second, centisecond;
	int nspectra;
	int status;
	long num_data;
	long RetriggerDelayTime;

	// float *current_PSD;
	register int i, j;
	int temp_int = 0;
	// float HH_noise_level;
	// float HV_noise_level;
	// float VV_noise_level;
	// float VH_noise_level;
	// int noisegate1, noisegate2;
	int gate_offset;
	int mode_gate_offset;
	time_t system_time;
	time_t start_time = 0;
	// time_t spectra_time = 0;
	// time_t spectra_rapid_time = 0;
	// time_t temp_time_t;
	//  struct tm *time_ptr;
	char datestring[25];
	char tmp_string[25];
	unsigned short sizeofstring;

	// float * uncoded_mean_vsq; // Used in sigma vbar calculation
	// float * uncoded_mean_Zsq; // Used in sigma Zbar calculation

	/* time variables */
	struct timeval tv;
	struct tm tm;
	struct timezone tz;

	struct timespec file_time;


	PolPSDStruct *PSD;
	// PolPSDStruct	PSD[1000];
	URC_ScanStruct scan;
	RNC_DimensionStruct dimensions;

	/* netCDF file pointer */
	int ncid;
	int ncidts = -1;
	// int spectra_ncid = -1;
	// int spectra_rapid_ncid = -1;
	// int PSD_varid[PSD_varidSize];
	// int PSD_rapid_varid[RapidPSD_varidSize];
	//  int PSD_varid[4];
	//  int PSD_rapid_varid[4];
	int file_stateid = 0;
	bool scanEnd = false;
	float norm_uncoded;

	/* Time series text file pointer */
	FILE *tsfid = NULL;

	//  static      RSM_SerialMessageStruct m_serialmsg;

	RSM_PositionMessageStruct position_msg;

	/* signal */
	struct sigaction sig_struct;

	int nm;
	int ray_count;
	int remainder;

	// The following are shortcut pointers to the elements of
	// the tsobs structure
	uint16_t *I_uncoded_H;
	uint16_t *Q_uncoded_H;
	uint16_t *I_uncoded_V;
	uint16_t *Q_uncoded_V;
	uint16_t *log_raw;
	uint16_t *TX1data;
	uint16_t *TX2data;
	uint16_t *V_not_H;

	int horizontal_first = 1;

	RSP_ParamStruct param;
	RSP_ComplexType *timeseries;
	RSP_ObservablesStruct obs;
	// RSP_ObservablesStruct PSD_obs;
	// RSP_ObservablesStruct PSD_RAPID_obs;
	TimeSeriesObs_t tsobs;

	int newfile = 1;
	int obtain_index;
	int store_index;
	//IQStruct IQStruct;

	disp_welcome_message();

	//---------------------------
	// Set up the signal handler
	//---------------------------
	/* Set up the sig_struct variable */
	sig_struct.sa_handler = sig_handler;
	sigemptyset(&sig_struct.sa_mask);
	sig_struct.sa_flags = 0;
	/* Install signal handler and check for error */
	if (sigaction(SIGINT, &sig_struct, NULL) != 0)
	{
		perror("Error installing signal handler\n");
		exit(1);
	}

	//------------------------
	// Read radar config file
	//------------------------
	get_config(CONFIG_FILE, &param, &scan, 0); // Do it for uncoded pulses

	//------------------------------
	// Parse command line arguments
	//------------------------------
	// overwrite config with command line parameters
	if (parseargs(argc, argv, &param, &scan, &start_day))
	{
		exit(1);
	}

	// Read calibration file
	get_cal(&param, CAL_FILE);

	/*-----------------------------------------------------*
	 * PCI DIO Card stuff - for setting radar mode         *
	 *-----------------------------------------------------*/
	uint16_t pulse_offset_n100ns = (uint16_t)((param.pulse_offset - 0.2 + 100) * 10 + 0.5f); /* 100us added to set bits for correct operation */
	uint16_t pulse_offset_bcd = dec2bcd_r(pulse_offset_n100ns);
	uint8_t pulse_offset_byte_a = pulse_offset_bcd & 0xFF;
	uint8_t pulse_offset_byte_b = pulse_offset_bcd >> 8;

	/* Set bits for chip length (number of 100ns units) */
	uint8_t chip_length_n100ns = (uint8_t)(param.pulse_period / 100 + 0.5f);

	uint8_t mode, mode0, mode1, wivern_mode[2];

	if (param.long_pulse_mode == 0)
	{
		/*----------------------------------------------------------*
		 * Short pulses                                             *
		 * Set mode0 and mode1 - radar will alternate between these *
		 * if param.alternate_modes==1                              *
		 *----------------------------------------------------------*/
		mode0 = (uint8_t)(param.mode0 | 0x08); // Most significant bit 1 to select short pulses
		wivern_mode[0] = (mode0 << 4) | chip_length_n100ns;

		mode1 = (uint8_t)(param.mode1 | 0x08); // Most significant bit 1 to select short pulses
		wivern_mode[1] = (mode1 << 4) | chip_length_n100ns;
	}
	else
	{
		/*--------------*
		 * Long pulses  *
		 *--------------*/
		// Need to double check effect of chip_length parameter here (CJW 20161216)
		mode0 = (uint8_t)param.mode0; // Most significant bit 0
		wivern_mode[0] = (mode0 << 4) | chip_length_n100ns;
		mode1 = (uint8_t)param.mode1; // Most significant bit 0
		wivern_mode[1] = (mode1 << 4) | chip_length_n100ns;
	}
	mode = mode0; // Select starting mode

	/*--------------------------------------*
	 * Open up the route to the serial port *
	 * and apply the right settings         *
	 *--------------------------------------*/
	if (positionMessageAct)
	{
		temp_int = RSM_InitialisePositionMessage(SERIALMESSAGE_PORT);
		if (temp_int != 0)
		{
			printf("Detected a problem with initialising the position message port\n");
			return 1;
		}

		/* wait for valid dish position */
		do
		{
			RSM_ReadPositionMessage(&position_msg);
			/* MTF: Add in breakout on Ctrl-C */
		} while (!exit_now && position_msg.month == 0); // Check msg is valid

		/* Check msg is valid */
		if (exit_now)
		{
			return 1;
		}

		printf("Az: %.3f   El: %.3f\n", position_msg.az, position_msg.el);
	}

	/* Fixup param.num_tx_pol for array creation */
	param.mode0 &= 0x07;
	switch (param.mode0)
	{
	case PM_Undefined0:
		param.num_tx_pol = 1;
		break;
	case PM_Single_H:
	case PM_Single_V:
	case PM_Double_H:
	case PM_Double_V:
		param.num_tx_pol = 1;
		break;
	case PM_Single_HV:
	case PM_Double_HV_VH:
	case PM_Double_HV_HV:
		param.num_tx_pol = 2;
		break;
	}
	if (param.alternate_modes != 0)
	{
		param.mode1 &= 0x07;
		switch (param.mode1)
		{
		case PM_Undefined0:
			break;
		case PM_Single_H:
		case PM_Single_V:
		case PM_Double_H:
		case PM_Double_V:
			break;
		case PM_Single_HV:
		case PM_Double_HV_VH:
		case PM_Double_HV_HV:
			param.num_tx_pol = 2;
			break;
		}
	}

	//------------------------------------
	// Initialise RSP parameter structure
	//------------------------------------
	RSP_InitialiseParams(&param); // This param is used for uncoded pulses

	gate_offset = (int)(0.5 + (param.pulse_offset * 1e-6) / param.sample_period);

	printf("Pulse offset: %f\n", param.pulse_offset);
	printf("Sample_period: %f\n", param.sample_period * 1e9);
	printf("Gate offset: %d\n", gate_offset);
	printf("nfft: %d\n", param.nfft);
	printf("npsd: %d\n", param.npsd);
	printf("frequency: %f\n", param.frequency);
	printf("spectra_averaged: %d\n", param.spectra_averaged);
	printf("Max num tx pol: %d\n", param.num_tx_pol);
	printf("PRT: %f\n", param.prt);
	printf("Phidp offset: %f\n", param.phidp_offset);
	printf("Mode 0: %x\n", wivern_mode[0]);
	printf("Mode 1: %x\n", wivern_mode[1]);

	printf("\nDisplay parameters:\n");
	RSP_DisplayParams(&param);

	RetriggerDelayTime = param.prt * 2.0e6;

	/* Remove a suitable amount to ensure good trigger */
	if (RetriggerDelayTime % 500 < 100)
		RetriggerDelayTime -= 500;

	/* Round down to nearest 500us */
	RetriggerDelayTime -= (RetriggerDelayTime % 500);

	// Sample extra pulses at end so that we have entire code sequence
	num_pulses = (int)(param.pulses_per_daq_cycle);
	tcount = num_pulses * param.spectra_averaged * param.samples_per_pulse * param.ADC_channels * sizeof(uint16_t);

	printf("Num pulses: %d\n", num_pulses);

	// Number of data points to allocate per data stream
	num_data = param.pulses_per_daq_cycle * param.samples_per_pulse;

// Allocate memory for IQ data streams
#if 0
	I_uncoded_copolar_H             = calloc(num_data, sizeof(uint16_t));
	Q_uncoded_copolar_H             = calloc(num_data, sizeof(uint16_t));
	I_uncoded_crosspolar_H          = calloc(num_data, sizeof(uint16_t));
	Q_uncoded_crosspolar_H          = calloc(num_data, sizeof(uint16_t));
	IQStruct.I_uncoded_copolar_H    = calloc(param.samples_per_pulse * param.nfft * param.num_tx_pol * param.spectra_averaged, sizeof(uint16_t));
	IQStruct.Q_uncoded_copolar_H    = calloc(param.samples_per_pulse * param.nfft * param.num_tx_pol * param.spectra_averaged, sizeof(uint16_t));
	IQStruct.I_uncoded_crosspolar_H = calloc(param.samples_per_pulse * param.nfft * param.num_tx_pol * param.spectra_averaged, sizeof(uint16_t));
	IQStruct.Q_uncoded_crosspolar_H = calloc(param.samples_per_pulse * param.nfft * param.num_tx_pol * param.spectra_averaged, sizeof(uint16_t));


	if (I_uncoded_copolar_H == NULL || Q_uncoded_copolar_H == NULL ||
		I_uncoded_crosspolar_H == NULL || Q_uncoded_crosspolar_H == NULL ||
		IQStruct.I_uncoded_copolar_H == NULL || IQStruct.Q_uncoded_copolar_H == NULL ||
		IQStruct.I_uncoded_crosspolar_H == NULL || IQStruct.Q_uncoded_crosspolar_H == NULL)
	{
		fprintf(stderr, "Memory allocation error: %m\n");
		return 3;
	}
#endif

	I_uncoded_H = calloc(num_data, sizeof(uint16_t));
	Q_uncoded_H = calloc(num_data, sizeof(uint16_t));
	I_uncoded_V = calloc(num_data, sizeof(uint16_t));
	Q_uncoded_V = calloc(num_data, sizeof(uint16_t));

#if 0
	IQStruct.I_uncoded_copolar_H    = calloc(param.samples_per_pulse * param.nfft * param.num_tx_pol * param.spectra_averaged, sizeof(uint16_t));
	IQStruct.Q_uncoded_copolar_H    = calloc(param.samples_per_pulse * param.nfft * param.num_tx_pol * param.spectra_averaged, sizeof(uint16_t));
	IQStruct.I_uncoded_crosspolar_H = calloc(param.samples_per_pulse * param.nfft * param.num_tx_pol * param.spectra_averaged, sizeof(uint16_t));
	IQStruct.Q_uncoded_crosspolar_H = calloc(param.samples_per_pulse * param.nfft * param.num_tx_pol * param.spectra_averaged, sizeof(uint16_t));


	if (I_uncoded_copolar_H == NULL || Q_uncoded_copolar_H == NULL ||
		I_uncoded_crosspolar_H == NULL || Q_uncoded_crosspolar_H == NULL ||
		IQStruct.I_uncoded_copolar_H == NULL || IQStruct.Q_uncoded_copolar_H == NULL ||
		IQStruct.I_uncoded_crosspolar_H == NULL || IQStruct.Q_uncoded_crosspolar_H == NULL)
	{
		fprintf(stderr, "Memory allocation error: %m\n");
		return 3;
	}
#endif

#if 0
	IQStruct.I_uncoded_H = calloc(param.samples_per_pulse * param.nfft * param.num_tx_pol * param.spectra_averaged, sizeof(uint16_t));
	IQStruct.Q_uncoded_H = calloc(param.samples_per_pulse * param.nfft * param.num_tx_pol * param.spectra_averaged, sizeof(uint16_t));
	IQStruct.I_uncoded_V = calloc(param.samples_per_pulse * param.nfft * param.num_tx_pol * param.spectra_averaged, sizeof(uint16_t));
	IQStruct.Q_uncoded_V = calloc(param.samples_per_pulse * param.nfft * param.num_tx_pol * param.spectra_averaged, sizeof(uint16_t));
#endif

	if (I_uncoded_H == NULL || Q_uncoded_H == NULL ||
		I_uncoded_V == NULL || Q_uncoded_V == NULL)
	{
		fprintf(stderr, "Memory allocation error: %m\n");
		return 3;
	}

	log_raw = calloc(num_data, sizeof(uint16_t));
	TX1data = calloc(num_data, sizeof(uint16_t));
	TX2data = calloc(num_data, sizeof(uint16_t));
	V_not_H = calloc(num_data, sizeof(uint16_t));

	num_data *= param.spectra_averaged;

	tsobs.IH = malloc(num_data * 8 * sizeof(uint16_t));
	tsobs.QH = tsobs.IH + num_data;
	tsobs.IV = tsobs.QH + num_data;
	tsobs.QV = tsobs.IV + num_data;
	tsobs.TxPower1 = tsobs.QV + num_data;
	tsobs.TxPower2 = tsobs.TxPower1 + num_data;
	tsobs.VnotH = tsobs.TxPower2 + num_data;
	tsobs.RawLog = tsobs.VnotH + num_data;

	if (tsobs.VnotH == NULL || log_raw == NULL ||
		TX1data == NULL || TX2data == NULL || V_not_H == NULL)
	{
		fprintf(stderr, "Memory allocation error: %m\n");
		return 3;
	}

	/*-----------------------------------------*
	 * Set initial mode                        *
	 *-----------------------------------------*/

#ifndef NO_DIO
	/* Open PCI DIO card */
	fd = open(device, O_RDWR);
	if (fd < 0)
	{
		perror("PCI DIO card not present\n");
		printf("PCI DIO card not present %s: %m\n", device);
		return 2;
	}
#endif /* NO_DIO */

	/* Set config port identifier */
	bank_0.id = IXPIO_PC;

	/* Set all ports for output */
	bank_0.value = 0x07;

#ifndef NO_DIO
	if (ioctl(fd, IXPIO_WRITE_REG, &bank_0))
	{
		perror("Can't set write mode for bank_0");
		printf("Can't set write mode for bank_0\n");
		close(fd);
		return 2;
	}
#endif /* NO_DIO */

	/* Set inital mode to mode0 */
	int new_mode = 0;

	/* Set port identifiers for output */
	bank_A.id = IXPIO_P0;
	bank_B.id = IXPIO_P1;
	bank_C.id = IXPIO_P2;

	/* Set bank structure values for writing */
	bank_A.value = pulse_offset_byte_a;
	bank_B.value = pulse_offset_byte_b;
	bank_C.value = wivern_mode[new_mode];

	mode = (new_mode == 1) ? param.mode1 : param.mode0;
	mode_gate_offset = (mode < PM_Double_H) ? param.samples_per_pulse : gate_offset;

	/* Fixup param.num_tx_pol for mode */
	switch (mode & 0x07)
	{
	case PM_Undefined0:
		param.num_tx_pol = 1;
		break;
	case PM_Single_H:
	case PM_Single_V:
	case PM_Double_H:
	case PM_Double_V:
		param.num_tx_pol = 1;
		break;
	case PM_Single_HV:
	case PM_Double_HV_VH:
	case PM_Double_HV_HV:
		param.num_tx_pol = 2;
		break;
	}

#ifndef NO_DIO
	/* Write out values to ports */
	if (ioctl(fd, IXPIO_WRITE_REG, &bank_A))
	{
		perror("Can't write bank_A");
		printf("Can't write bank_A value 0x%x\n", bank_A.value);
	}
	else
	{
		printf("Writing 0x%x to bank_A\n", bank_A.value);
	}

	if (ioctl(fd, IXPIO_WRITE_REG, &bank_B))
	{
		perror("Can't write bank_B");
		printf("Can't write bank_B value 0x%x\n", bank_B.value);
	}
	else
	{
		printf("Writing 0x%x to bank_B\n", bank_B.value);
	}

	if (ioctl(fd, IXPIO_WRITE_REG, &bank_C))
	{
		perror("Can't write bank_C");
		printf("Can't write bank_C value 0x%x\n", bank_C.value);
	}
	else
	{
		printf("Writing 0x%x to bank_C\n", bank_C.value);
	}
#endif /* NO_DIO */

	new_mode = -1;

	//-------------------------------------------
	// Set up the data acquisition
	//-------------------------------------------
	printf("** Initialising ISACTRL...\n");
	RDQ_InitialiseISACTRL(num_pulses * param.spectra_averaged, param.samples_per_pulse,
						  param.clock_divfactor, param.delay_clocks);

	printf("** Initialising PCICARD...\n");
	amcc_fd = RDQ_InitialisePCICARD_New(&dma_buffer, DMA_BUFFER_SIZE);

	// Initialise pointers to DMA banks
	dma_banks[0] = (uint16_t *)dma_buffer;
	dma_banks[1] = (uint16_t *)(dma_buffer + (DMA_BUFFER_SIZE / 2));

	make_dmux_table(param.ADC_channels, dmux_table);

	/* store current pulse mode*/
	obs.pulse_mode = mode;

	/* load in current dish_time */
	if (positionMessageAct)
	{
		RSM_ReadPositionMessage(&position_msg);

		obs.azimuth = position_msg.az;
		obs.elevation = position_msg.el;
		obs.dish_year = position_msg.year;
		obs.dish_month = position_msg.month;
		obs.dish_day = position_msg.day;
		obs.dish_hour = position_msg.hour;
		obs.dish_minute = position_msg.min;
		obs.dish_second = position_msg.sec;
		obs.dish_centisecond = position_msg.centi_sec;
	}
	else
	{
		struct tm tm;
		struct timeval tv;

		gettimeofday(&tv, NULL);
		gmtime_r(&tv.tv_sec, &tm);
		obs.dish_year = tm.tm_year + 1900;
		obs.dish_month = tm.tm_mon + 1;
		obs.dish_day = tm.tm_mday;
		obs.dish_hour = tm.tm_hour;
		obs.dish_minute = tm.tm_min;
		obs.dish_second = tm.tm_sec;
		obs.dish_centisecond = tv.tv_usec / 10000U;
		obs.azimuth = scan.scan_angle;
		obs.elevation = scan.min_angle;
	}

	// RDQ_StartAcquisition(amcc_fd, dma_bank,
	//					 (short *)dma_banks[dma_bank], tcount);

	if (tsdump)
	{
		if (TextTimeSeries)
		{
			/* Setup the time-series dump file */
			tsfid = RTS_OpenTSFile(GetRadarName(GALILEO), scan.date, host_ext,
								   GetScanTypeName(scan.scanType));

			if (tsfid == NULL)
			{
				tsdump = false;
				printf("**** Can't open time series file: %m ****\n");
				printf("**** Time series recording off ****\n");
			}
			else
			{
				fprintf(tsfid, "npulse: %d\n", num_pulses * param.spectra_averaged);
				fprintf(tsfid, "nsample: %d\n", param.samples_per_pulse_ts);
				fprintf(tsfid, "divfactor: %d\n", param.clock_divfactor);
				fprintf(tsfid, "delayclocks: %d\n", param.delay_clocks);
				fprintf(tsfid, "ADC_channels: %d\n", param.ADC_channels);
			}
		}
		else if (NetCDFTimeSeries)
		{
			/* NetCDF Time series */
			RNC_DimensionStruct dimensionsts;

			memset(&dimensionsts, 0, sizeof(dimensionsts));
			ncidts = RNC_OpenNetcdfFile(GetRadarName(GALILEO), "ts",
										scan.date, host_ext,
										GetScanTypeName(scan.scanType),
										"", "ts");
			RNC_SetupTimeSeriesDimensions(ncidts, &param, &dimensionsts);
			RNC_SetupGlobalAttributes(ncidts, GALILEO, &scan, &param, argc, argv);
			RNC_SetupStaticVariables(ncidts, GALILEO, &param);
			RNC_SetupRange(ncidts, &param, &dimensionsts);
			SetupTimeSeriesVariables(&tsobs, ncidts, &param, &scan, &dimensionsts, &obs);

			status = nc_enddef(ncidts);
			if (status != NC_NOERR)
				check_netcdf_handle_error(status);
		}
		else
		{
			/* Setup binary time-series dump file */
			/* get timeofday */
			struct timespec tspec;
    		timespec_get(&tspec, TIME_UTC);
    
			tsbinfid = RTS_OpenTSFileBinary(GetRadarName(GALILEO), scan.date, host_ext,
											GetScanTypeName(scan.scanType));

			if (tsbinfid == NULL)
			{
				tsdump = false;
				printf("**** Can't open time series file: %m ****\n");
				printf("**** Time series recording off ****\n");
			}
			else
			{
				WriteTimeSeriesBinaryHeader(tsbinfid, &tspec, GALILEO, &scan, &param, argc, argv);

				//int int_value = num_pulses * param.pulses_per_daq_cycle;
				//fwrite(&int_value, sizeof(int), 1, tsbinfid);
				//fwrite(&param.samples_per_pulse_ts, sizeof(int), 1, tsbinfid);
				//fwrite(&param.clock_divfactor, sizeof(int), 1, tsbinfid);
				//fwrite(&param.delay_clocks, sizeof(int), 1, tsbinfid);
				//fwrite(&param.ADC_channels, sizeof(int), 1, tsbinfid);
			}
		}
	}

	/*---------------------*
	 * Wait for scan start *
	 *---------------------*/
	if (positionMessageAct)
	{
		wait_scan_start(scan.scanType, &position_msg,
						scan.min_angle, scan.max_angle);
		if (exit_now)
			goto exit_endacquisition;
	}

	printf("** Starting acquisition...\n");

	/* get timeofday */
	start_time = time(NULL);

	gettimeofday(&tv, NULL);
	gmtime_r(&tv.tv_sec, &tm);

	int raystart_year = tm.tm_year + 1900;
	int raystart_month = tm.tm_mon + 1;
	int raystart_day = tm.tm_mday;
	int raystart_hour = tm.tm_hour;
	int raystart_minute = tm.tm_min;
	int raystart_second = tm.tm_sec;
	int raystart_centisecond = (int)tv.tv_usec / 10000;

	int raystart_dish_year, raystart_dish_month, raystart_dish_day;
	int raystart_dish_hour, raystart_dish_minute, raystart_dish_second;
	int raystart_dish_centisecond;
	float raystart_azimuth, raystart_elevation;

	/* obtain dish time */
	if (positionMessageAct)
	{
		RSM_ReadPositionMessage(&position_msg);
		raystart_azimuth = position_msg.az;
		raystart_elevation = position_msg.el;
		raystart_dish_year = position_msg.year;
		raystart_dish_month = position_msg.month;
		raystart_dish_day = position_msg.day;
		raystart_dish_hour = position_msg.hour;
		raystart_dish_minute = position_msg.min;
		raystart_dish_second = position_msg.sec;
		raystart_dish_centisecond = position_msg.centi_sec;
	}
	else
	{
		raystart_dish_year = raystart_year;
		raystart_dish_month = raystart_month;
		raystart_dish_day = raystart_day;
		raystart_dish_hour = raystart_hour;
		raystart_dish_minute = raystart_minute;
		raystart_dish_second = raystart_second;
		raystart_dish_centisecond = raystart_centisecond;

		raystart_azimuth = scan.scan_angle;
		raystart_elevation = scan.min_angle;
	}

	sprintf(datestring, "%04d/%02d/%02d %02d:%02d:%02d.%02d",
			raystart_year, raystart_month, raystart_day,
			raystart_hour, raystart_minute, raystart_second, raystart_centisecond);
	printf("Ray start: %s\n", datestring);

	printf("Az = %5.2f, El = %5.2f",raystart_azimuth,raystart_elevation);


	RDQ_StartAcquisition(amcc_fd, dma_bank,
						 (short *)(dma_banks[dma_bank]), tcount);

	obs.ray_number = 0;
	ray_count = 0;
	remainder = -1;

	total_samples = (int)(param.samples_per_pulse * param.nfft);

	/*-----------------------------------------*
	 * THIS IS THE START OF THE OUTER RAY LOOP *
	 *-----------------------------------------*/
	while (!scanEnd && !exit_now)
	{
		printf("ray number: %d\n", obs.ray_number);
		printf("\n<< PRESS CTRL-C TO EXIT >>\n");

		printf("Ray count: %d\n");
		ray_count++;
		printf("Ray count: %d\n");

		if (new_mode >= 0)
		{
			printf("Setting new mode\n");
			/* Wait for acquisition to complete before setting new mode */
			/* CJW: commented out as this if statement will not be accessed for ray 0 */
			// status = RDQ_WaitForAcquisitionToComplete(amcc_fd);
			// if (status != 0)
			//	printf("There was a problem in WaitForAcquisitionToComplete\n");

			/* Swap around the areas used for storing data and processing from */
			dma_bank = 1 - dma_bank;
			proc_bank = 1 - proc_bank;

			/* Set new mode */
			bank_C.id = IXPIO_P2;
			bank_C.value = wivern_mode[new_mode];

			mode = (new_mode == 1) ? param.mode1 : param.mode0;
			mode_gate_offset = (mode < PM_Double_H) ? param.samples_per_pulse : gate_offset;

			/* Fixup param.num_tx_pol for mode */
			switch (mode & 0x07)
			{
			case PM_Undefined0:
				param.num_tx_pol = 1;
				break;
			case PM_Single_H:
			case PM_Single_V:
			case PM_Double_H:
			case PM_Double_V:
				param.num_tx_pol = 1;
				break;
			case PM_Single_HV:
			case PM_Double_HV_VH:
			case PM_Double_HV_HV:
				param.num_tx_pol = 2;
				break;
			}

#ifndef NO_DIO
			if (ioctl(fd, IXPIO_WRITE_REG, &bank_C))
			{
				perror("Can't write bank_C");
				printf("Can't write bank_C value 0x%x\n", bank_C.value);
			}
			else
			{
				printf("Writing 0x%x to bank_C\n", bank_C.value);
			}
#endif /* NO_DIO */
			new_mode = -1;
		}

		/* store current pulse mode*/
		obs.pulse_mode = mode;

#if 0
			/*----------------------------------------------------------------*
			 * Extract data from DMA memory                                   *
			 *----------------------------------------------------------------*/
			for (i = 0; i < num_pulses; i++)
			{
				register int count_reg;

				for (j = 0; j < param.samples_per_pulse; j++)
				{
					count_reg = (i * param.samples_per_pulse) + j;
					if (swap_iq_channels)
					{
						tsobs.ICOH[idx]   = I_uncoded_copolar_H[count_reg]    = GET_CHANNEL (data, SWAP_CHAN_Ic);
						tsobs.QCOH[idx]   = Q_uncoded_copolar_H[count_reg]    = GET_CHANNEL (data, SWAP_CHAN_Qc);
						tsobs.ICXH[idx]   = I_uncoded_crosspolar_H[count_reg] = GET_CHANNEL (data, SWAP_CHAN_Ix);
						tsobs.QCXH[idx]   = Q_uncoded_crosspolar_H[count_reg] = GET_CHANNEL (data, SWAP_CHAN_Qx);
					}
					else
					{
						tsobs.ICOH[idx]   = I_uncoded_copolar_H[count_reg]    = GET_CHANNEL (data, CHAN_Ic);
						tsobs.QCOH[idx]   = Q_uncoded_copolar_H[count_reg]    = GET_CHANNEL (data, CHAN_Qc);
						tsobs.ICXH[idx]   = I_uncoded_crosspolar_H[count_reg] = GET_CHANNEL (data, CHAN_Ix);
						tsobs.QCXH[idx]   = Q_uncoded_crosspolar_H[count_reg] = GET_CHANNEL (data, CHAN_Qx);
					}

					tsobs.TxPower1[idx] = TX1data[count_reg] = GET_CHANNEL (data, CHAN_T1);
					tsobs.TxPower2[idx] = TX2data[count_reg] = GET_CHANNEL (data, CHAN_T2);
					tsobs.VnotH[idx]    = V_not_H[count_reg] = GET_CHANNEL (data, CHAN_V_not_H);
					tsobs.RawLog[idx]   = log_raw[count_reg] = GET_CHANNEL (data, CHAN_INC);

					INC_POINTER (data, param.ADC_channels);
					idx++;

					if (tsfid != NULL && (j < param.samples_per_pulse_ts))
					{
						/* time-series dump */
						fprintf (tsfid, "%d %d %hu %hu %hu %hu %hu %hu %hu\n",
								i + (nspectra * num_pulses), j,
								I_uncoded_copolar_H[count_reg],
								Q_uncoded_copolar_H[count_reg],
								I_uncoded_crosspolar_H[count_reg],
								Q_uncoded_crosspolar_H[count_reg],
								TX1data[count_reg],
								TX2data[count_reg],
								V_not_H[count_reg]);
					}
				}
			}

#endif

		/* Wait for data acquisition to complete */
		status = RDQ_WaitForAcquisitionToComplete(amcc_fd);
		if (status != 0)
			printf("There was a problem in WaitForAcquisitionToComplete\n");

		/* get timeofday */
		gettimeofday(&tv, NULL);
		gmtime_r(&tv.tv_sec, &tm);

		obs.rayend_year = tm.tm_year + 1900;
		obs.rayend_month = tm.tm_mon + 1;
		obs.rayend_day = tm.tm_mday;
		obs.rayend_hour = tm.tm_hour;
		obs.rayend_minute = tm.tm_min;
		obs.rayend_second = tm.tm_sec;
		obs.rayend_centisecond = (int)tv.tv_usec / 10000;

		sprintf(datestring, "%04d/%02d/%02d %02d:%02d:%02d.%02d",
				obs.rayend_year, obs.rayend_month, obs.rayend_day,
				obs.rayend_hour, obs.rayend_minute, obs.rayend_second, obs.rayend_centisecond);
		printf("Ray end: %s\n", datestring);

		/* Swap around the areas used for storing daq and processing from */
		dma_bank = 1 - dma_bank;
		proc_bank = 1 - proc_bank;

		/*---------------------------------------------------------------------*
		 * Wait until just before next H pulse to prevent HV timeout.          *
		 *---------------------------------------------------------------------*/
		usleep(RetriggerDelayTime);

		/* Extract data for processing */
		data = dma_banks[proc_bank];

		/* get timeofday */
		gettimeofday(&tv, NULL);
		gmtime_r(&tv.tv_sec, &tm);

		obs.year = raystart_year;
		obs.month = raystart_month;
		obs.day = raystart_day;
		obs.hour = raystart_hour;
		obs.minute = raystart_minute;
		obs.second = raystart_second;
		obs.centisecond = raystart_centisecond;

		obs.dish_year = raystart_dish_year;
		obs.dish_month = raystart_dish_month;
		obs.dish_day = raystart_dish_day;
		obs.dish_hour = raystart_dish_hour;
		obs.dish_minute = raystart_dish_minute;
		obs.dish_second = raystart_dish_second;
		obs.dish_centisecond = raystart_dish_centisecond;
		obs.azimuth = raystart_azimuth;
		obs.elevation = raystart_elevation;

		raystart_year = tm.tm_year + 1900;
		raystart_month = tm.tm_mon + 1;
		raystart_day = tm.tm_mday;
		raystart_hour = tm.tm_hour;
		raystart_minute = tm.tm_min;
		raystart_second = tm.tm_sec;
		raystart_centisecond = (int)tv.tv_usec / 10000;

		sprintf(datestring, "%04d/%02d/%02d %02d:%02d:%02d.%02d",
				raystart_year, raystart_month, raystart_day,
				raystart_hour, raystart_minute, raystart_second, raystart_centisecond);
		printf("Ray start: %s\n", datestring);

		/* obtain dish time */
		if (positionMessageAct)
		{
			RSM_ReadPositionMessage(&position_msg);
			raystart_azimuth = position_msg.az;
			raystart_elevation = position_msg.el;
			raystart_dish_year = position_msg.year;
			raystart_dish_month = position_msg.month;
			raystart_dish_day = position_msg.day;
			raystart_dish_hour = position_msg.hour;
			raystart_dish_minute = position_msg.min;
			raystart_dish_second = position_msg.sec;
			raystart_dish_centisecond = position_msg.centi_sec;
		}
		else
		{
			raystart_dish_year = raystart_year;
			raystart_dish_month = raystart_month;
			raystart_dish_day = raystart_day;
			raystart_dish_hour = raystart_hour;
			raystart_dish_minute = raystart_minute;
			raystart_dish_second = raystart_second;
			raystart_dish_centisecond = raystart_centisecond;

			raystart_azimuth = scan.scan_angle;
			raystart_elevation = scan.min_angle;
		}

		RDQ_StartAcquisition(amcc_fd, dma_bank,
							 (short *)(dma_banks[dma_bank]), tcount);

		if (tsfid != NULL)
		{
			/* time-series ray header for text file */
			fprintf(tsfid, "Ray_number: %d, %d\n", obs.ray_number, nm);
			fprintf(tsfid, "Date_time: %s\n", datestring);
			fprintf(tsfid, "Az: %7.2f, El: %7.2f\n",
					obs.azimuth, obs.elevation);
		}

		if (tsbinfid != NULL)
		{
			sprintf(tmp_string, "ray_number");
			sizeofstring = strlen(tmp_string) + 1;
			fwrite(&sizeofstring, sizeof(unsigned short), 1, tsbinfid);
			fwrite(&tmp_string, sizeof(char), sizeofstring, tsbinfid);
			fwrite(&obs.ray_number, sizeof(int), 1, tsbinfid);

			sprintf(tmp_string, "azimuth");
			sizeofstring = strlen(tmp_string) + 1;
			fwrite(&sizeofstring, sizeof(unsigned short), 1, tsbinfid);
			fwrite(&tmp_string, sizeof(char), sizeofstring, tsbinfid);
			fwrite(&obs.azimuth, sizeof(float), 1, tsbinfid);

			sprintf(tmp_string, "elevation");
			sizeofstring = strlen(tmp_string) + 1;
			fwrite(&sizeofstring, sizeof(unsigned short), 1, tsbinfid);
			fwrite(&tmp_string, sizeof(char), sizeofstring, tsbinfid);
			fwrite(&obs.elevation, sizeof(float), 1, tsbinfid);

			sizeofstring = strlen(datestring) + 1;
			fwrite(&sizeofstring, sizeof(unsigned short), 1, tsbinfid);
			fwrite(&datestring, sizeof(char), sizeofstring, tsbinfid);
		}

		/* Start of loop over spectra */
		for (int idx = nspectra = 0; nspectra < param.spectra_averaged; nspectra++)
		{
			/*----------------------------------------------------------------*
			 * Extract data for each channel                                  *
			 *----------------------------------------------------------------*/
			for (i = 0; i < num_pulses; i++)
			{
				register int count_reg;

				for (j = 0; j < param.samples_per_pulse; j++)
				{
					count_reg = (i * param.samples_per_pulse) + j;
					if (swap_iq_channels)
					{
						tsobs.IH[idx] = I_uncoded_H[count_reg] = GET_CHANNEL(data, SWAP_CHAN_IH);
						tsobs.QH[idx] = Q_uncoded_H[count_reg] = GET_CHANNEL(data, SWAP_CHAN_QH);
						tsobs.IV[idx] = I_uncoded_V[count_reg] = GET_CHANNEL(data, SWAP_CHAN_IV);
						tsobs.QV[idx] = Q_uncoded_V[count_reg] = GET_CHANNEL(data, SWAP_CHAN_QV);
					}
					else
					{
						tsobs.IH[idx] = I_uncoded_H[count_reg] = GET_CHANNEL(data, CHAN_IH);
						tsobs.QH[idx] = Q_uncoded_H[count_reg] = GET_CHANNEL(data, CHAN_QH);
						tsobs.IV[idx] = I_uncoded_H[count_reg] = GET_CHANNEL(data, CHAN_IV);
						tsobs.QV[idx] = Q_uncoded_H[count_reg] = GET_CHANNEL(data, CHAN_QV);
					}

					tsobs.TxPower1[idx] = TX1data[count_reg] = GET_CHANNEL(data, CHAN_T1);
					tsobs.TxPower2[idx] = TX2data[count_reg] = GET_CHANNEL(data, CHAN_T2);
					tsobs.VnotH[idx] = V_not_H[count_reg] = GET_CHANNEL(data, CHAN_V_not_H);
					tsobs.RawLog[idx] = log_raw[count_reg] = GET_CHANNEL(data, CHAN_INC);

					INC_POINTER(data, param.ADC_channels);
					idx++;

					if (tsfid != NULL && (j < param.samples_per_pulse_ts))
					{
						/* Text time-series dump */
						fprintf(tsfid, "%d %d %hu %hu %hu %hu %hu %hu %hu\n",
								i + (nspectra * num_pulses), j,
								I_uncoded_H[count_reg],
								Q_uncoded_H[count_reg],
								I_uncoded_V[count_reg],
								Q_uncoded_V[count_reg],
								TX1data[count_reg],
								TX2data[count_reg],
								V_not_H[count_reg]);
					}
				}
			}

#if 0
			//			fwrite(obs.year, sizeof(int), 1, pFile);
			//			fwrite(obs.month, sizeof(int), 1, pFile);
			//			fwrite(obs.day, sizeof(int), 1, pFile);
			//			fwrite(obs.hour, sizeof(int), 1, pFile);
			//			fwrite(obs.minute, sizeof(int), 1, pFile);
			//			fwrite(obs.second, sizeof(int), 1, pFile);
			//			fwrite(obs.centisecond, sizeof(int), 1, pFile);

			/* store I and Q for each pulse */
			/* nspectra defines the spectra number */
		
			//for (i = 0; i < param.nfft; i++)
			//{
			//	obtain_index = i * param.samples_per_pulse;
			//	store_index = (i * param.samples_per_pulse) + (nspectra * param.samples_per_pulse * param.nfft);
			//	for (j = 0; j < param.samples_per_pulse; j++)
			//	{
			//		IQStruct.I_uncoded_H[store_index] = I_uncoded_H[obtain_index];
			//		IQStruct.Q_uncoded_H[store_index] = Q_uncoded_H[obtain_index];
			//		IQStruct.I_uncoded_V[store_index] = I_uncoded_V[obtain_index];
			//		IQStruct.Q_uncoded_V[store_index] = Q_uncoded_V[obtain_index];
			//		store_index = store_index + 1;
			//		obtain_index = obtain_index + 1;
			//	}
			//}

			//			fwrite(IQStruct.I_uncoded_H, sizeof(short), total_samples, pFile);
			//			fwrite(IQStruct.Q_uncoded_H, sizeof(short), total_samples, pFile);
			//			fwrite(IQStruct.I_uncoded_V, sizeof(short), total_samples, pFile);
			//			fwrite(IQStruct.Q_uncoded_V, sizeof(short), total_samples, pFile);

			//			printf("completed storing IQs\n");
#endif

		} /* End of loop over spectra */

		if (!exit_now && tsdump && !TextTimeSeries)
		{
			if (NetCDFTimeSeries)
			{
				printf("Writing timeseries variables to NetCDF...\n");
				WriteOutTimeSeriesData(ncidts, &param, &obs, &tsobs, 0);
				status = nc_sync(ncidts);
				if (status != NC_NOERR)
					check_netcdf_handle_error(status);
			}
			else
			{
				printf("Writing timeseries variables to binary file...\n");
				WriteOutTimeSeriesDataBinary(tsbinfid, &param, &obs, &tsobs, 0);
				#if 0
				sprintf(tmp_string, "I_uncoded_H");
				sizeofstring = strlen(tmp_string) + 1;
				fwrite(&sizeofstring, sizeof(unsigned short), 1, tsbinfid);
				fwrite(&tmp_string, sizeof(char), sizeofstring, tsbinfid);
				fwrite(I_uncoded_H, sizeof(uint16_t), total_samples, tsbinfid);

				sprintf(tmp_string, "Q_uncoded_H");
				sizeofstring = strlen(tmp_string) + 1;
				fwrite(&sizeofstring, sizeof(unsigned short), 1, tsbinfid);
				fwrite(&tmp_string, sizeof(char), sizeofstring, tsbinfid);
				fwrite(Q_uncoded_H, sizeof(uint16_t), total_samples, tsbinfid);

				sprintf(tmp_string, "I_uncoded_V");
				sizeofstring = strlen(tmp_string) + 1;
				fwrite(&sizeofstring, sizeof(unsigned short), 1, tsbinfid);
				fwrite(&tmp_string, sizeof(char), sizeofstring, tsbinfid);
				fwrite(I_uncoded_V, sizeof(uint16_t), total_samples, tsbinfid);

				sprintf(tmp_string, "Q_uncoded_V");
				sizeofstring = strlen(tmp_string) + 1;
				fwrite(&sizeofstring, sizeof(unsigned short), 1, tsbinfid);
				fwrite(&tmp_string, sizeof(char), sizeofstring, tsbinfid);
				fwrite(Q_uncoded_V, sizeof(uint16_t), total_samples, tsbinfid);

				sprintf(tmp_string, "TX1data");
				sizeofstring = strlen(tmp_string) + 1;
				fwrite(&sizeofstring, sizeof(unsigned short), 1, tsbinfid);
				fwrite(&tmp_string, sizeof(char), sizeofstring, tsbinfid);
				fwrite(TX1data, sizeof(uint16_t), total_samples, tsbinfid);

				sprintf(tmp_string, "TX2data");
				sizeofstring = strlen(tmp_string) + 1;
				fwrite(&sizeofstring, sizeof(unsigned short), 1, tsbinfid);
				fwrite(&tmp_string, sizeof(char), sizeofstring, tsbinfid);
				fwrite(TX2data, sizeof(uint16_t), total_samples, tsbinfid);

				sprintf(tmp_string, "V_not_H");
				sizeofstring = strlen(tmp_string) + 1;
				fwrite(&sizeofstring, sizeof(unsigned short), 1, tsbinfid);
				fwrite(&tmp_string, sizeof(char), sizeofstring, tsbinfid);
				fwrite(V_not_H, sizeof(uint16_t), total_samples, tsbinfid);

				sprintf(tmp_string, "log_raw");
				sizeofstring = strlen(tmp_string) + 1;
				fwrite(&sizeofstring, sizeof(unsigned short), 1, tsbinfid);
				fwrite(&tmp_string, sizeof(char), sizeofstring, tsbinfid);
				fwrite(log_raw, sizeof(uint16_t), total_samples, tsbinfid);
				#endif
			}
		}

#ifdef HAVE_DISLIN
		/* check to see if the real time spectra display option has been selected */
		if (param.real_time_spectra_display == 1)
		{
			/* real time display */
			/* write out data */
			for (i = 0; i < param.samples_per_pulse; i++)
			{
				/* calculate the log10 of the PSD */
				for (j = 0; j < param.npsd; j++)
				{
					zmat[j][i] = 10 * log10(PSD[i].HH[j]);
				}
			}
			crvmat((float *)zmat, param.npsd, param.samples_per_pulse, 1, 1);
			height(20);
			title();
			printf("x window updated\n");
		}
#endif /* HAVE_DISLIN */

		/*--------------------------------------------------------------------*
		 * check to see if we have exceeded scan duration for this file       *
		 *--------------------------------------------------------------------*/
		system_time = time(NULL);
		double scan_duration;
		scan_duration = difftime(system_time, start_time);
		//if (tm.tm_hour != obs.hour)
		//{
		//	printf("***** New hour rollover detected.\n");
		//	break; /* Exit loop */
		//}
		if (scan_duration>898)
		{
			printf("***** Default scan duration elapsed.\n");
			break; /* Exit loop */
		}

#if 0
		if (positionMessageAct)
		{
			/* Read position message again */
			RSM_ReadPositionMessage(&position_msg);
			obs.azimuth = position_msg.az;
			obs.elevation = position_msg.el;
			obs.dish_year = position_msg.year;
			obs.dish_month = position_msg.month;
			obs.dish_day = position_msg.day;
			obs.dish_hour = position_msg.hour;
			obs.dish_minute = position_msg.min;
			obs.dish_second = position_msg.sec;
			obs.dish_centisecond = position_msg.centi_sec;
		}
		else
		{
			struct tm tm;
			struct timeval tv;

			gettimeofday(&tv, NULL);
			gmtime_r(&tv.tv_sec, &tm);
			obs.dish_year = tm.tm_year + 1900;
			obs.dish_month = tm.tm_mon + 1;
			obs.dish_day = tm.tm_mday;
			obs.dish_hour = tm.tm_hour;
			obs.dish_minute = tm.tm_min;
			obs.dish_second = tm.tm_sec;
			obs.dish_centisecond = tv.tv_usec / 10000U;
		}
#endif

		/* Test for end of scan */
		if (positionMessageAct || scan.scanType == SCAN_SGL)
		{
			scanEnd = scanEnd_test(scan.scanType, &position_msg,
								   scan.min_angle, scan.max_angle);
		}

		/* Prepare for possible switch to new mode next ray */
		if ((param.alternate_modes != 0) & (param.long_pulse_mode == 0))
		{
			remainder = ray_count % (param.nrays_mode0 + param.nrays_mode1);
			printf("remainder calculated\n");
			if (remainder == 0)
			{
				new_mode = 0;
				ray_count = 0;
			}
			else if (remainder == param.nrays_mode0)
			{
				new_mode = 1;
			}
			else
			{
				new_mode = -1;
			}
		}
		else
		{
			/* Don't alternate modes (remain in mode defined by mode0) */
			new_mode = -1;
		}
		obs.ray_number++;
	}

	/*-------------------------------------------------------------------------- *
	 * END OF SCAN LOOP -------------------------------------------------------- *
	 *========================================================================== */

exit_endacquisition:
	// Finish off
	printf("*** Closing PCICARD...\n");

	RDQ_ClosePCICARD_New(amcc_fd, &dma_buffer, DMA_BUFFER_SIZE);

	if (tsbinfid != NULL)
	{
		// Close binary time-series data file
		fclose(tsbinfid);
	}

	if (tsfid != NULL)
	{
		/* Close time-series text file */
		fclose(tsfid);
	}

	if (tsdump)
	{
		if (NetCDFTimeSeries)
		{
			printf("About to sync ts.\n");
			status = nc_sync(ncidts);
			if (status != NC_NOERR)
				check_netcdf_handle_error(status);
			printf("About to close ts.\n");
			status = nc_close(ncidts);
			printf("Status = %d\n", status);
			if (status != NC_NOERR)
				check_netcdf_handle_error(status);
		}
		free(tsobs.IH);
	}

	//---------------------------
	// Unallocate all the memory
	//---------------------------

	RSP_FreeMemory(&param); // Free memory allocated by RSP package

#if 0
	free(IQStruct.I_uncoded_H);
	free(IQStruct.Q_uncoded_H);
	free(IQStruct.I_uncoded_V);
	free(IQStruct.Q_uncoded_V);
#endif

	//=========
	// THE END
	//=========
	printf("All done.\n");
	return (0);
}

/*****************************************************************************/
/*!
 *
 * \brief Helper function for setting up observables.
 *
 * \param [in] ncid            NetCDF file handle.
 * \param [in] name            Observable/variable name.
 * \param [in] variable_shape  Shape array for variable (all have 2 dims)
 * \param [in] std_name        Chilbolton standard name.
 * \param [in] long_name       Long name.
 * \param [in] units           Units string.
 *
 * \return NetCDF veriable id.
 *
 * \details This function assumes that the variable_shape has been correctly
 * 			set up for the first three dimensions: time,pulses,unaveraged_range
 * 			as required.
 *
 *****************************************************************************/
static int
TimeSeriesTemplate(int ncid, const char *name, const int *variable_shape,
				   const char *std_name, const char *long_name, const char *units)
{
	int varid;
	int status;

	status = nc_def_var(ncid, name, NC_SHORT, 3, variable_shape, &varid);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	status = nc_put_att_text(ncid, varid, "chilbolton_standard_name",
							 strlen(std_name) + 1, std_name);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	status = nc_put_att_text(ncid, varid, "long_name",
							 strlen(long_name) + 1, long_name);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	status = nc_put_att_text(ncid, varid, "units",
							 strlen(units) + 1, units);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	return varid;
}

/******************************************************************************/
/*!
 *
 * \brief blah
 *
 * \param obs			aaa
 * \param ncid			bbb
 * \param param			ccc
 * \param scan			ddd
 * \param dimensions	eee
 * \param posobs		fff
 *
 *****************************************************************************/
static void
SetupTimeSeriesVariables(TimeSeriesObs_t *obs, int ncid, RSP_ParamStruct *param,
						 URC_ScanStruct *scan, RNC_DimensionStruct *dimensions,
						 RSP_ObservablesStruct *posobs)
{
	short Bias = 2047;
	double PowerScale = 3000.0 / 4096.0;
	int status;
	int dims[3];
	const char *variable;
	char buffer[1024];

	/****************************************************************************
	 * define pulses dimension
	 ****************************************************************************/
	// status = nc_def_dim(ncid, "pulses",
	//					param->pulses_per_daq_cycle * param->spectra_averaged,
	//					&dims[1]);
	// if (status != NC_NOERR)
	//	check_netcdf_handle_error(status);

	/****************************************************************************
	 * define samples dimension
	 ****************************************************************************/
	// status = nc_def_dim(ncid, "samples",
	//					param->samples_per_pulse_ts,
	//					&dims[2]);
	// if (status != NC_NOERR)
	//	check_netcdf_handle_error(status);

	dims[0] = dimensions->time_dim;
	dims[1] = dimensions->pulses_dim;
	dims[2] = dimensions->range_dim;

	/*--------------------------------------------------------------------------*
	 * time definition                                                          *
	 *--------------------------------------------------------------------------*/
	status = nc_def_var(ncid, "time",
						NC_FLOAT, 1, dims, &obs->tsid);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	variable = "time";
	status = nc_put_att_text(ncid, obs->tsid, "chilbolton_standard_name",
							 strlen(variable) + 1, variable);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	variable = "time";
	status = nc_put_att_text(ncid, obs->tsid, "long_name",
							 strlen(variable) + 1, variable);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	variable = "%.2f";
	status = nc_put_att_text(ncid, obs->tsid, "C_format",
							 strlen(variable) + 1, variable);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	sprintf(buffer, "seconds since %c%c%c%c-%c%c-%c%c 00:00:00 +00:00",
			scan->date[0], scan->date[1], scan->date[2], scan->date[3],
			scan->date[4], scan->date[5],
			scan->date[6], scan->date[7]);
	status = nc_put_att_text(ncid, obs->tsid, "units",
							 strlen(buffer) + 1, buffer);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	/*--------------------------------------------------------------------------*
	 * ray end time definition                                                  *
	 *--------------------------------------------------------------------------*/
	status = nc_def_var(ncid, "ray_end_time",
						NC_FLOAT, 1, dims, &obs->rayend_tsid);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	variable = "ray_end_time";
	status = nc_put_att_text(ncid, obs->rayend_tsid, "chilbolton_standard_name",
							 strlen(variable) + 1, variable);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	variable = "time_ray_end";
	status = nc_put_att_text(ncid, obs->rayend_tsid, "long_name",
							 strlen(variable) + 1, variable);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	variable = "%.2f";
	status = nc_put_att_text(ncid, obs->rayend_tsid, "C_format",
							 strlen(variable) + 1, variable);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	sprintf(buffer, "seconds since %c%c%c%c-%c%c-%c%c 00:00:00 +00:00",
			scan->date[0], scan->date[1], scan->date[2], scan->date[3],
			scan->date[4], scan->date[5],
			scan->date[6], scan->date[7]);
	status = nc_put_att_text(ncid, obs->rayend_tsid, "units",
							 strlen(buffer) + 1, buffer);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	/*--------------------------------------------------------------------------*
	 * dish_time definition                                                     *
	 *--------------------------------------------------------------------------*/
	status = nc_def_var(ncid, "dish_time",
						NC_FLOAT, 1, dims, &obs->dish_tsid);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	variable = "dish_time";
	status = nc_put_att_text(ncid, obs->dish_tsid, "chilbolton_standard_name",
							 strlen(variable) + 1, variable);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	variable = "dish_time";
	status = nc_put_att_text(ncid, obs->dish_tsid, "long_name",
							 strlen(variable) + 1, variable);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	variable = "%.2f";
	status = nc_put_att_text(ncid, obs->tsid, "C_format",
							 strlen(variable) + 1, variable);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	sprintf(buffer, "seconds since %04d-%02d-%02d 00:00:00 +00:00",
			posobs->dish_year,
			posobs->dish_month,
			posobs->dish_day);
	status = nc_put_att_text(ncid, obs->dish_tsid, "units",
							 strlen(buffer) + 1, buffer);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	/*--------------------------------------------------------------------------*
	 * elevation definition                                                     *
	 *--------------------------------------------------------------------------*/
	status = nc_def_var(ncid, "elevation",
						NC_FLOAT, 1, dims, &obs->elevationid);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	variable = "elevation angle above the horizon at the start of the beamwidth";
	status = nc_put_att_text(ncid, obs->elevationid, "long_name",
							 strlen(variable) + 1, variable);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	variable = "%.3f";
	status = nc_put_att_text(ncid, obs->elevationid, "C_format",
							 strlen(variable) + 1, variable);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	variable = "degree";
	status = nc_put_att_text(ncid, obs->elevationid, "units",
							 strlen(variable) + 1, variable);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	/*--------------------------------------------------------------------------*
	 * azimuth definition                                                       *
	 *--------------------------------------------------------------------------*/
	status = nc_def_var(ncid, "azimuth",
						NC_FLOAT, 1, dims, &obs->azimuthid);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	variable = "azimuth angle clockwise from the grid north at the start of the beamwidth";
	status = nc_put_att_text(ncid, obs->azimuthid, "long_name",
							 strlen(variable) + 1, variable);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	variable = "%.3f";
	status = nc_put_att_text(ncid, obs->azimuthid, "C_format",
							 strlen(variable) + 1, variable);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	variable = "degree";
	status = nc_put_att_text(ncid, obs->azimuthid, "units",
							 strlen(variable) + 1, variable);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	status = nc_put_att_float(ncid, obs->azimuthid, "azimuth_offset",
							  NC_FLOAT, 1, &param->azimuth_offset);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	/*--------------------------------------------------------------------------*
	 * pulse_mode definition                                                    *
	 *--------------------------------------------------------------------------*/
	status = nc_def_var(ncid, "pulse_mode",
						NC_SHORT, 1, dims, &obs->pulse_modeid);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	variable = "pulse mode for ray";
	status = nc_put_att_text(ncid, obs->pulse_modeid, "long_name",
							 strlen(variable) + 1, variable);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	//variable = "%.3f";
	//status = nc_put_att_text(ncid, obs->azimuthid, "C_format",
	//						 strlen(variable) + 1, variable);
	//if (status != NC_NOERR)
	//	check_netcdf_handle_error(status);

	variable = "";
	status = nc_put_att_text(ncid, obs->pulse_modeid, "units",
							 strlen(variable) + 1, variable);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

#if 0
	obs->ICOHid = TimeSeriesTemplate(
		ncid, "ICOH",
		dims,
		"I_uncoded_copolar_H",
		"I uncoded copolar H",
		"counts");
	status = nc_put_att_short(ncid, obs->ICOHid,
							  "nominal_bias", NC_SHORT, 1,
							  &Bias);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	obs->QCOHid = TimeSeriesTemplate(
		ncid, "QCOH",
		dims,
		"Q_uncoded_copolar_H",
		"Q uncoded copolar H",
		"counts");
	status = nc_put_att_short(ncid, obs->QCOHid,
							  "nominal_bias", NC_SHORT, 1,
							  &Bias);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	obs->ICXHid = TimeSeriesTemplate(
		ncid, "ICXH",
		dims,
		"I_uncoded_crosspolar_H",
		"I uncoded crosspolar H",
		"counts");
	status = nc_put_att_short(ncid, obs->ICXHid,
							  "nominal_bias", NC_SHORT, 1,
							  &Bias);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	obs->QCXHid = TimeSeriesTemplate(
		ncid, "QCXH",
		dims,
		"Q_uncoded_crosspolar_H",
		"Q uncoded crosspolar H",
		"counts");
	status = nc_put_att_short(ncid, obs->QCXHid,
							  "nominal_bias", NC_SHORT, 1,
							  &Bias);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);
#endif

	obs->IHid = TimeSeriesTemplate(
		ncid, "IH",
		dims,
		"I_uncoded_H",
		"I uncoded H",
		"counts");
	status = nc_put_att_short(ncid, obs->IHid,
							  "nominal_bias", NC_SHORT, 1,
							  &Bias);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	obs->QHid = TimeSeriesTemplate(
		ncid, "QH",
		dims,
		"Q_uncoded_H",
		"Q uncoded H",
		"counts");
	status = nc_put_att_short(ncid, obs->QHid,
							  "nominal_bias", NC_SHORT, 1,
							  &Bias);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	obs->IVid = TimeSeriesTemplate(
		ncid, "IV",
		dims,
		"I_uncoded_V",
		"I uncoded V",
		"counts");
	status = nc_put_att_short(ncid, obs->IVid,
							  "nominal_bias", NC_SHORT, 1,
							  &Bias);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	obs->QVid = TimeSeriesTemplate(
		ncid, "QV",
		dims,
		"Q_uncoded_V",
		"Q uncoded V",
		"counts");
	status = nc_put_att_short(ncid, obs->QVid,
							  "nominal_bias", NC_SHORT, 1,
							  &Bias);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	obs->TxPower1id = TimeSeriesTemplate(
		ncid, "TXP1",
		dims,
		"Internal_Tx_Power",
		"Internal Tx Power",
		"counts");
	status = nc_put_att_double(ncid, obs->TxPower1id,
							   "mV scale", NC_DOUBLE, 1,
							   &PowerScale);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	//obs->TxPower2id = TimeSeriesTemplate(
	//	ncid, "TXP2",
	//	dims,
	//	"External_Tx_Power",
	//	"External Tx Power",
	//	"");
	//status = nc_put_att_double(ncid, obs->TxPower2id,
	//						   "mV scale", NC_DOUBLE, 1,
	//						   &PowerScale);
	//if (status != NC_NOERR)
	//	check_netcdf_handle_error(status);

	obs->VnotHid = TimeSeriesTemplate(
		ncid, "VnotH",
		dims,
		"Pulse_polaration_V_not_H",
		"Pulse polaration V not H",
		"");

	//obs->RawLogid = TimeSeriesTemplate(
	//	ncid, "LOG",
	//	dims,
	//	"Raw_Log",
	//	"Raw Log",
	//	"counts");
	//status = nc_put_att_double(ncid, obs->RawLogid,
	//						   "mV scale", NC_DOUBLE, 1,
	//						   &PowerScale);
	//if (status != NC_NOERR)
	//	check_netcdf_handle_error(status);
}

static void WriteTimeSeriesBinaryHeader(FILE *tsbinfid, struct timespec *tspec, int radar,
										const URC_ScanStruct *scan,
										const RSP_ParamStruct *param,
										int argc, char *const argv[])
{
	int n;

	uint16_t sizeofstring;
	uint16_t major_version = 1;
	uint16_t minor_version = 0;
	uint16_t revision = 0;

	char buff[1024];
	char file_timestamp[100];
	char *pt;

	sprintf(buff, "CHILRAD-TS-%d.%d.%d\n",major_version,minor_version,revision);
    sizeofstring = strlen(buff)+1;
    fwrite(&buff, sizeof(char), sizeofstring, tsbinfid);

    long centisec = tspec->tv_nsec/10000000L;

    strftime(file_timestamp, sizeof(file_timestamp), "%F %T", gmtime(&tspec->tv_sec));
    sprintf(buff, "File created: %s.%02ldZ\n", file_timestamp, centisec);
    sizeofstring = strlen(buff)+1;
    fwrite(&buff, sizeof(char), sizeofstring, tsbinfid);

	sprintf(buff, "radar = %s\n", GetRadarName(GALILEO));
	sizeofstring = strlen(buff)+1;
    fwrite(&buff, sizeof(char), sizeofstring, tsbinfid);

	sprintf(buff, "history = %s\n", GetRadarName(GALILEO));
	sizeofstring = strlen(buff)+1;
    fwrite(&buff, sizeof(char), sizeofstring, tsbinfid);

	/*--------------------------------------------------------------------------*
     * history                                                                  *
     *--------------------------------------------------------------------------*/
    n = sprintf(buff, "history: %sZ\n", file_timestamp);

	if (n < 0) n = 0;

	pt = stpcpy(buff + n, " - ");
	for (n = 0; n < argc; n++)
	{
		pt = stpcpy(pt, argv[n]);
		pt = stpcpy(pt, " ");
	}

	sizeofstring = strlen(buff)+1;
    fwrite(&buff, sizeof(char), sizeofstring, tsbinfid);


	//uint16_t int_value = param->pulses_per_daq_cycle * param->spectra_averaged;

					//int int_value = num_pulses * param.pulses_per_daq_cycle;
				//fwrite(&int_value, sizeof(int), 1, tsbinfid);
				//fwrite(&param.samples_per_pulse_ts, sizeof(int), 1, tsbinfid);
				//fwrite(&param.clock_divfactor, sizeof(int), 1, tsbinfid);
				//fwrite(&param.delay_clocks, sizeof(int), 1, tsbinfid);
				//fwrite(&param.ADC_channels, sizeof(int), 1, tsbinfid);
	
	/*
	radar = {};
	string_str history;
	string_str scantype;
	string_str operator;
	string_str project_name;
	string_str project_tag;
	uint32_t adc_clock;
	string_str adc_clock_units = "Hz" 
	u32 adc_channels 8 u32 adc_clock_divfactor 2 u32 adc_delay_clocks 2 u32 adc_bits_per_sample 12 u32 samples_per_pulse 200 u32 pulses_per_ray 6144 u32 pulse_width
		string_str pulse_width_units "nanosec" u32 radar_frequency 94008000000 string_str radar_frequency_units "Hz" u32 prf 6250 string_str prf_units "Hz" u32 transmit_power 1600 string_str transmit_power_units "watt" u32 antenna_diameter 460 string_str antenna_diameter_units "mm" u32 antenna_focal_length
			string_str antenna_focal_length_units "mm" u32 beamwidth_h 30 string_str beamwidth_h_units "arcminute" u32 beamwidth_v 30 string_str beamwidth_v_units "arcminute" u32 pulse_offset 20000 string_str pulse_offset_units "nanosecond" u8 alternating_modes 1 u8 long_pulse_mode 0 u8 mode0 6 u8 mode1 3 u32 nrays_mode0 1 u32 nrays_mode1 1 u32 azimuth_offset 0 string_str azimuth_offset_units arcminute
				u32 elevation_offset 0 string_str elevation_offset_units arcminute
					u32 antenna_ellipsoidal_altitude 8500 string_str antenna_ellipsoidal_altitude_units "cm" u32 antenna_altitude_agl
						string_str antenna_altitude_agl_units "mm" u32 latitude
							string_str latitude_units "arcsecond_north" u32 longitude
								string_str longitude_units "arcsecond_east"
								*/

}


static void WriteOutTimeSeriesDataBinary(FILE *tsbinfid, const RSP_ParamStruct *param,
								   RSP_ObservablesStruct *posobs,
								   TimeSeriesObs_t *obs, int nm)
{
	char buffer[255];
	uint16_t sizeofstring;

    int data_size = param->samples_per_pulse_ts*param->pulses_per_daq_cycle*param->spectra_averaged;

	printf("data size = %d\n",data_size);

	sprintf(buffer, "IH");
	sizeofstring = strlen(buffer) + 1;
	fwrite(&sizeofstring, sizeof(uint16_t), 1, tsbinfid);
	fwrite(&buffer, sizeof(char), sizeofstring, tsbinfid);
	fwrite(obs->IH, sizeof(uint16_t), data_size, tsbinfid);

	sprintf(buffer, "QH");
	sizeofstring = strlen(buffer) + 1;
	fwrite(&sizeofstring, sizeof(uint16_t), 1, tsbinfid);
	fwrite(&buffer, sizeof(char), sizeofstring, tsbinfid);
	fwrite(obs->QH, sizeof(uint16_t), data_size, tsbinfid);

	sprintf(buffer, "IV");
	sizeofstring = strlen(buffer) + 1;
	fwrite(&sizeofstring, sizeof(uint16_t), 1, tsbinfid);
	fwrite(&buffer, sizeof(char), sizeofstring, tsbinfid);
	fwrite(obs->IV, sizeof(uint16_t), data_size, tsbinfid);

	sprintf(buffer, "QV");
	sizeofstring = strlen(buffer) + 1;
	fwrite(&sizeofstring, sizeof(uint16_t), 1, tsbinfid);
	fwrite(&buffer, sizeof(char), sizeofstring, tsbinfid);
	fwrite(obs->QV, sizeof(uint16_t), data_size, tsbinfid);

	sprintf(buffer, "TX1POWER");
	sizeofstring = strlen(buffer) + 1;
	fwrite(&sizeofstring, sizeof(uint16_t), 1, tsbinfid);
	fwrite(&buffer, sizeof(char), sizeofstring, tsbinfid);
	fwrite(obs->TxPower1, sizeof(uint16_t), data_size, tsbinfid);

	//sprintf(buffer, "TX2POWER");
	//sizeofstring = strlen(buffer) + 1;
	//fwrite(&sizeofstring, sizeof(uint16_t), 1, tsbinfid);
	//fwrite(&buffer, sizeof(char), sizeofstring, tsbinfid);
	//fwrite(obs->TxPower2, sizeof(uint16_t), data_size, tsbinfid);

	sprintf(buffer, "V_not_H");
	sizeofstring = strlen(buffer) + 1;
	fwrite(&sizeofstring, sizeof(uint16_t), 1, tsbinfid);
	fwrite(&buffer, sizeof(char), sizeofstring, tsbinfid);
	fwrite(obs->VnotH, sizeof(uint16_t), data_size, tsbinfid);

	//sprintf(buffer, "log_raw");
	//sizeofstring = strlen(buffer) + 1;
	//fwrite(&sizeofstring, sizeof(uint16_t), 1, tsbinfid);
	//fwrite(&buffer, sizeof(char), sizeofstring, tsbinfid);
	//fwrite(obs->RawLog, sizeof(uint16_t), data_size, tsbinfid);

#if 0
	/*--------------------------------------------------------------------------*
	 * write time                                                               *
	 *--------------------------------------------------------------------------*/
	variable_start[0] = posobs->ray_number;
	temp_float = (((int)posobs->hour * 3600) + ((int)posobs->minute * 60) +
				  posobs->second + ((float)posobs->centisecond / 100.0));
	status = nc_put_var1_float(ncid, obs->tsid, variable_start, &temp_float);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	/*--------------------------------------------------------------------------*
	 * write dish_time                                                          *
	 *--------------------------------------------------------------------------*/
	temp_float = (((int)posobs->dish_hour * 3600) +
				  ((int)posobs->dish_minute * 60) + posobs->dish_second +
				  ((float)posobs->dish_centisecond / 100.0));
	status = nc_put_var1_float(ncid, obs->dish_tsid,
							   variable_start, &temp_float);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	/*--------------------------------------------------------------------------*
	 * write elevation                                                          *
	 *--------------------------------------------------------------------------*/
	status = nc_put_var1_float(ncid, obs->elevationid,
							   variable_start, &posobs->elevation);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	/*--------------------------------------------------------------------------*
	 * write azimuth                                                            *
	 *--------------------------------------------------------------------------*/
	temp_float = posobs->azimuth + param->azimuth_offset;
	status = nc_put_var1_float(ncid, obs->azimuthid,
							   variable_start, &temp_float);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	/*--------------------------------------------------------------------------*
	 * write radar observables                                                  *
	 *--------------------------------------------------------------------------*/
	variable_start[0] = (posobs->ray_number * param->moments_averaged) + nm;
	variable_start[1] = 0;
	variable_start[2] = 0;
	variable_count[0] = 1;
	variable_count[1] = param->pulses_per_daq_cycle * param->spectra_averaged;
	variable_count[2] = param->samples_per_pulse_ts;
	variable_stride[0] = 1;
	variable_stride[1] = 1;
	variable_stride[2] = 1;
	variable_imap[0] = 1;
	variable_imap[1] = param->samples_per_pulse;
	variable_imap[2] = 1;

#endif

#if 0
	status = nc_put_varm_short(ncid, obs->ICOHid, variable_start,
							   variable_count, variable_stride,
							   variable_imap, (short *)obs->ICOH);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);
	status = nc_put_varm_short(ncid, obs->QCOHid, variable_start,
							   variable_count, variable_stride,
							   variable_imap, (short *)obs->QCOH);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);
	status = nc_put_varm_short(ncid, obs->ICXHid, variable_start,
							   variable_count, variable_stride,
							   variable_imap, (short *)obs->ICXH);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);
	status = nc_put_varm_short(ncid, obs->QCXHid, variable_start,
							   variable_count, variable_stride,
							   variable_imap, (short *)obs->QCXH);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);
#endif

#if 0
	status = nc_put_varm_short(ncid, obs->IHid, variable_start,
							   variable_count, variable_stride,
							   variable_imap, (short *)obs->IH);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);
	status = nc_put_varm_short(ncid, obs->QHid, variable_start,
							   variable_count, variable_stride,
							   variable_imap, (short *)obs->QH);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);
	status = nc_put_varm_short(ncid, obs->IVid, variable_start,
							   variable_count, variable_stride,
							   variable_imap, (short *)obs->IV);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);
	status = nc_put_varm_short(ncid, obs->QVid, variable_start,
							   variable_count, variable_stride,
							   variable_imap, (short *)obs->QV);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	status = nc_put_varm_short(ncid, obs->TxPower1id, variable_start,
							   variable_count, variable_stride,
							   variable_imap, (short *)obs->TxPower1);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);
	status = nc_put_varm_short(ncid, obs->TxPower2id, variable_start,
							   variable_count, variable_stride,
							   variable_imap, (short *)obs->TxPower2);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);
	status = nc_put_varm_short(ncid, obs->VnotHid, variable_start,
							   variable_count, variable_stride,
							   variable_imap, (short *)obs->VnotH);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);
	status = nc_put_varm_short(ncid, obs->RawLogid, variable_start,
							   variable_count, variable_stride,
							   variable_imap, (short *)obs->RawLog);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);
#endif
}

static void WriteOutTimeSeriesData(int ncid, const RSP_ParamStruct *param,
								   RSP_ObservablesStruct *posobs,
								   TimeSeriesObs_t *obs, int nm)
{
	size_t variable_count[3];
	size_t variable_start[3];
	ptrdiff_t variable_stride[3];
	ptrdiff_t variable_imap[3];
	int status;
	float temp_float;

	/*--------------------------------------------------------------------------*
	 * write time                                                               *
	 *--------------------------------------------------------------------------*/
	variable_start[0] = posobs->ray_number;
	temp_float = (((int)posobs->hour * 3600) + ((int)posobs->minute * 60) +
				  posobs->second + ((float)posobs->centisecond / 100.0));
	status = nc_put_var1_float(ncid, obs->tsid, variable_start, &temp_float);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	/*--------------------------------------------------------------------------*
	 * write dish_time                                                          *
	 *--------------------------------------------------------------------------*/
	temp_float = (((int)posobs->dish_hour * 3600) +
				  ((int)posobs->dish_minute * 60) + posobs->dish_second +
				  ((float)posobs->dish_centisecond / 100.0));
	status = nc_put_var1_float(ncid, obs->dish_tsid,
							   variable_start, &temp_float);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	/*--------------------------------------------------------------------------*
	 * write rayend_time                                                        *
	 *--------------------------------------------------------------------------*/
	temp_float = (((int)posobs->rayend_hour * 3600) +
				  ((int)posobs->rayend_minute * 60) + posobs->rayend_second +
				  ((float)posobs->rayend_centisecond / 100.0));
	status = nc_put_var1_float(ncid, obs->rayend_tsid,
							   variable_start, &temp_float);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);


	/*--------------------------------------------------------------------------*
	 * write elevation                                                          *
	 *--------------------------------------------------------------------------*/
	status = nc_put_var1_float(ncid, obs->elevationid,
							   variable_start, &posobs->elevation);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	/*--------------------------------------------------------------------------*
	 * write azimuth                                                            *
	 *--------------------------------------------------------------------------*/
	temp_float = posobs->azimuth + param->azimuth_offset;
	status = nc_put_var1_float(ncid, obs->azimuthid,
							   variable_start, &temp_float);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	/*--------------------------------------------------------------------------*
	 * write pulse mode                                                         *
	 *--------------------------------------------------------------------------*/
	status = nc_put_var1_short(ncid, obs->pulse_modeid,
							   variable_start, (short *)&posobs->pulse_mode);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	/*--------------------------------------------------------------------------*
	 * write radar observables                                                  *
	 *--------------------------------------------------------------------------*/
	variable_start[0] = (posobs->ray_number * param->moments_averaged) + nm;
	variable_start[1] = 0;
	variable_start[2] = 0;
	variable_count[0] = 1;
	variable_count[1] = param->pulses_per_daq_cycle * param->spectra_averaged;
	variable_count[2] = param->samples_per_pulse_ts;
	variable_stride[0] = 1;
	variable_stride[1] = 1;
	variable_stride[2] = 1;
	variable_imap[0] = 1;
	variable_imap[1] = param->samples_per_pulse;
	variable_imap[2] = 1;

#if 0
	status = nc_put_varm_short(ncid, obs->ICOHid, variable_start,
							   variable_count, variable_stride,
							   variable_imap, (short *)obs->ICOH);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);
	status = nc_put_varm_short(ncid, obs->QCOHid, variable_start,
							   variable_count, variable_stride,
							   variable_imap, (short *)obs->QCOH);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);
	status = nc_put_varm_short(ncid, obs->ICXHid, variable_start,
							   variable_count, variable_stride,
							   variable_imap, (short *)obs->ICXH);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);
	status = nc_put_varm_short(ncid, obs->QCXHid, variable_start,
							   variable_count, variable_stride,
							   variable_imap, (short *)obs->QCXH);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);
#endif

	status = nc_put_varm_short(ncid, obs->IHid, variable_start,
							   variable_count, variable_stride,
							   variable_imap, (short *)obs->IH);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);
	status = nc_put_varm_short(ncid, obs->QHid, variable_start,
							   variable_count, variable_stride,
							   variable_imap, (short *)obs->QH);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);
	status = nc_put_varm_short(ncid, obs->IVid, variable_start,
							   variable_count, variable_stride,
							   variable_imap, (short *)obs->IV);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);
	status = nc_put_varm_short(ncid, obs->QVid, variable_start,
							   variable_count, variable_stride,
							   variable_imap, (short *)obs->QV);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);

	status = nc_put_varm_short(ncid, obs->TxPower1id, variable_start,
							   variable_count, variable_stride,
							   variable_imap, (short *)obs->TxPower1);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);
	//status = nc_put_varm_short(ncid, obs->TxPower2id, variable_start,
	//						   variable_count, variable_stride,
	//						   variable_imap, (short *)obs->TxPower2);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);
	status = nc_put_varm_short(ncid, obs->VnotHid, variable_start,
							   variable_count, variable_stride,
							   variable_imap, (short *)obs->VnotH);
	if (status != NC_NOERR)
		check_netcdf_handle_error(status);
	//status = nc_put_varm_short(ncid, obs->RawLogid, variable_start,
	//						   variable_count, variable_stride,
	//						   variable_imap, (short *)obs->RawLog);
	//if (status != NC_NOERR)
	//	check_netcdf_handle_error(status);
}
