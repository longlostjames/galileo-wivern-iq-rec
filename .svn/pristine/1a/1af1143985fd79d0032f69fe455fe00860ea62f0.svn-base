
#include "/home/chilbolton_software/radar-galileo/radar-galileo-iq-rec/version-0.2/universal_radar_code/RSP/include/RSP.h"
#include "/home/chilbolton_software/radar-galileo/radar-galileo-iq-rec/version-0.2/universal_radar_code/include/radar.h"

/* this will stop any screws up with the PSD_varid */
#define PSD_HH 0
#define PSD_HV 1
#define PSD_VV 2
#define PSD_VH 3

#define PSD_HHP 4
#define PSD_HVP 5
#define PSD_VVP 6
#define PSD_VHP 7

#define IPF_HH 8
#define QPF_HH 9
#define IPF_HV 10
#define QPF_HV 11
#define IPF_VV 12
#define QPF_VV 13
#define IPF_VH 14
#define QPF_VH 15

#define IPF_HHP 16
#define QPF_HHP 17
#define IPF_HVP 18
#define QPF_HVP 19
#define IPF_VVP 20
#define QPF_VVP 21
#define IPF_VHP 22
#define QPF_VHP 23



typedef struct {
	int 	time_dim;
	int	range_dim;
	int	peak_dim;
	int	time_range_dim;
	int	fft_bin_dim;
	int	coded_fft_bin_dim;
	int	spectra_number_dim;
}	RNC_DimensionStruct;

void RNC_OpenNetcdfFile( URC_ScanStruct *args, int radar, int *ncid );

void RNC_SetupGlobalAttributes( int ncid, int radar, URC_ScanStruct *args, RSP_ParamStruct *param, int argc, char *argv[]);

void RNC_SetupPulse_Compression_Code( int ncid, RSP_ParamStruct *param); 

int RNC_SetupFile_State( int ncid);

void RNC_SetupStaticVariables( int ncid, RSP_ParamStruct *param);

void RNC_SetupRange( int ncid, RSP_ParamStruct *param, RNC_DimensionStruct *dimensions);

void RNC_SetupDimensions( int ncid, RSP_ParamStruct *param, RNC_DimensionStruct *dimensions);

void RNC_SetupRapidLogPSDDimensions ( int ncid, int radar, RSP_ParamStruct *param, RNC_DimensionStruct *dimensions );

void RNC_SetupDynamicVariables( int ncid, int radar, URC_ScanStruct *args, RSP_ParamStruct *param, RNC_DimensionStruct *dimensions, RSP_ObservablesStruct *obs );

void RNC_SetupPSDVariables( int ncid, int radar, RSP_ParamStruct *param, RNC_DimensionStruct *dimensions, int PSD_varid[] );

void RNC_SetupLogPSDVariables( int ncid, int radar, RSP_ParamStruct *param, RNC_DimensionStruct *dimensions, int PSD_varid[] );

void RNC_WriteDynamicVariables( int ncid, RSP_ParamStruct *param, RSP_ObservablesStruct *obs );

void RNC_WritePSDVariables( int ncid, int radar, RSP_ParamStruct *param, RSP_ObservablesStruct *obs, PolPSDStruct PSD[], int PSD_varid[] );

void RNC_WriteLogPSDVariables( int ncid, int radar, RSP_ParamStruct *param, RSP_ObservablesStruct *obs, PolPSDStruct PSD[], IQStruct *IQStruct, int PSD_varid[] );

void RNC_WriteRapidLogPSDVariables( int ncid, int radar, RSP_ParamStruct *param, RSP_ObservablesStruct *obs, PolPSDStruct
PSD[], int PSD_varid[] );

void check_netcdf_handle_error(int status);


double RNC_GetConfigDouble(char *filename, char *keyword);

float RNC_GetConfigFloat(char *filename, char *keyword);

int RNC_GetConfig(char *filename, char *keyword, char *value);

