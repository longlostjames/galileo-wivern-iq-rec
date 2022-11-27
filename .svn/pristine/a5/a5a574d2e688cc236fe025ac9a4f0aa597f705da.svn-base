#ifndef __RADAR_H
#define __RADAR_H

// Define radar ID numbers
// (inherited from Andrew's old CF format)
// CJW renamed HIGHLANDER to VIPER - 20080716
// CJW added scan type SCAN_MAN - 20101020
#define  CAMRA       1
#define  RABELAIS    2
#define  GALILEO     3
#define  BISTATIC    4
#define  VIPER       5
#define  SINGAPORE   6
#define  COPERNICUS  7
#define  COPERNICUS_SPECTRA 71
#define  COPERNICUS_CODED_SPECTRA 72
#define  ACROBAT     8
#define	 TEST		9
#define	 TEST_SPECTRA	91
#define  TEST_SPECTRA_RAPID 92
#define  ACROBAT_CODED_SPECTRA 73
#define  GALILEO_SPECTRA	74
#define  GALILEO_SPECTRA_RAPID  75
#define	 COPERNICUS_SPECTRA_RAPID 76

// Define scan types
#define SCAN_PPI 0  // PPI Scan
#define SCAN_RHI 1  // RHI Scan
#define SCAN_FIX 2  // Fixed dwell
#define SCAN_CSP 3  // CSP scan
#define SCAN_MAN 6  // Manual scan (also for target tracking)
#define SCAN_SGL 9  // Single ray

// URC_ScanStruct : Holds information about the scan
// (mostly from the information record)
typedef struct {
  int scanType;
  float min_angle;
  float max_angle;
  float scan_angle;
  float scan_velocity;
  float dwelltime;
  int file_number;
  int scan_number;
  int experiment_id;
  char operator[4];
  char date[14];
  int min_gate;
  int max_gate;
} URC_ScanStruct;

// This structure holds pointers to the various PSDs
typedef struct {
    float *HH;  // H copolar
    float *HV;  // H crosspolar
    float *VV;  // V copolar
    float *VH;  // V crosspolar
    float *HHP; // H copolar (pulse coded)
    float *HVP; // H crosspolar (pulse coded)
    float *VVP; // V copolar (pulse coded)
    float *VHP; // V crosspolar (pulse coded)
} PolPSDStruct;

/* this will hold the IQ data for a spectra */
typedef struct {
	long int *I_coded_copolar_H;
	long int *Q_coded_copolar_H;
	long int *I_coded_copolar_V;
	long int *Q_coded_copolar_V;
  	long int *I_coded_crosspolar_H;
  	long int *Q_coded_crosspolar_H;
	long int *I_coded_crosspolar_V;
	long int *Q_coded_crosspolar_V;
	short int *I_uncoded_copolar_H;
        short int *Q_uncoded_copolar_H;
        short int *I_uncoded_copolar_V;
        short int *Q_uncoded_copolar_V;
        short int *I_uncoded_crosspolar_H;
        short int *Q_uncoded_crosspolar_H;
        short int *I_uncoded_crosspolar_V;
        short int *Q_uncoded_crosspolar_V;
} IQStruct;


// This structure is used to address the various pulses in the
// transmit sequence
typedef struct {
    int HP;  // H pulse coded
    int VP;  // V pulse coded
    int H;   // H uncoded
    int V;   // V uncoded
  } URC_SampleStruct;

#endif
