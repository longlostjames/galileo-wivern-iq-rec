/* 
   Purpose: 	To oversee the generate of netCDF files

   	Created on:  12/02/2003
   	Created by:  Owain Davies (OTD)
   	Modified on: 29/08/2007
   	Modified by: Chris Walden (CJW)
	Modified on: 13/09/2007
	Modified by: Owain Davies
	added in the ability to dump out IQ data prior to fft
	Modified on: 29/09/2010
	Modified by: John Nicol (JCN)
	included dual-pol. parameters; phi dp and rho hv
	Modified on 04/10/2010 (JCN)
	included uncoded rapid spectra dump
        Modified on 13/11/2014 (JCN)
        included noise power counts
*/


#include <stdio.h>
#include <string.h>
#include <netcdf.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>

#include <RNC.h>
#include "/root/universal_radar_code/RSP/include/RSP.h"

void RNC_OpenNetcdfFile( URC_ScanStruct *args, int radar, int *ncid )
{
	/*
	IN:     *args : a pointer to the args structure
		ncid : the netCDF file id	
	OUT:	
	RETURN: 
 	*/ 

	char 	netcdf_pathfile[255];
	int	status;
	mode_t 	mask=002;

	umask(mask);

	netcdf_pathfile[0] = '\0';
	strcat(netcdf_pathfile, "/mnt/focus_radar_data/");

 	switch(radar)
	{
		case ACROBAT :
		{
			strcat( netcdf_pathfile, "radar-acrobat/raw/");
			break;
		}
		case COPERNICUS  :
		{
                        strcat( netcdf_pathfile, "radar-copernicus/raw/");
                        break;
                }
		case GALILEO :
		{
			strcat( netcdf_pathfile, "radar-galileo/raw/");
                        break;
                }
		case CAMRA :
                {
                        strcat( netcdf_pathfile, "radar-camra/raw/");
                        break;
                }
		case TEST :
                {
                        strcat( netcdf_pathfile, "radar-test/raw/");
                        break;
                }
		case TEST_SPECTRA :
                {
                        strcat( netcdf_pathfile, "radar-test/spectra/");
                        break;
                }
		case TEST_SPECTRA_RAPID :
                {
                        strcat( netcdf_pathfile, "radar-test/spectra-rapid/");
                        break;
                }
		case ACROBAT_CODED_SPECTRA :
                {
                        strcat( netcdf_pathfile, "radar-acrobat/spectra/");
                        break;
                }
		case COPERNICUS_SPECTRA :
		{
			strcat( netcdf_pathfile, "radar-copernicus/spectra/");
			break;
		}
		case COPERNICUS_SPECTRA_RAPID :
                {
                        strcat( netcdf_pathfile, "radar-copernicus/spectra-rapid/");
                        break;
                }
		case GALILEO_SPECTRA :
                {
                        strcat( netcdf_pathfile, "radar-galileo/spectra/");
                        break;
                }
		case GALILEO_SPECTRA_RAPID :
                {
                        strcat( netcdf_pathfile, "radar-galileo/spectra-rapid/");
                        break;
                }
	}

	strncat ( netcdf_pathfile, args->date, 8);

	if ( mkdir( netcdf_pathfile,  ( S_IRWXU | S_IRWXG | (S_IROTH | S_IXOTH)))  == 0) {
		printf("directory created : %s \n", netcdf_pathfile);
	} else {
		if ( errno != EEXIST ) {
			printf(" mkdir ERROR, errno : %d, netcdf_pathfile : %s\n", errno, netcdf_pathfile);
		} 
	}

        switch(radar)
        {
                case ACROBAT :
		case ACROBAT_CODED_SPECTRA :
		{
                        strcat( netcdf_pathfile, "/radar-acrobat_");
			break;
		}
                case COPERNICUS :
		case COPERNICUS_SPECTRA :
		case COPERNICUS_SPECTRA_RAPID :
		{
                        strcat( netcdf_pathfile, "/radar-copernicus_");
			break;
		}
		case GALILEO :
                case GALILEO_SPECTRA :
		case GALILEO_SPECTRA_RAPID :
                {
                        strcat( netcdf_pathfile, "/radar-galileo_");
                        break;
                }
		case TEST :
		case TEST_SPECTRA :
		case TEST_SPECTRA_RAPID :
		{
			strcat( netcdf_pathfile, "/radar-test_");
			break;
		}
		case CAMRA :
                {
                        strcat( netcdf_pathfile, "/radar-camra_");
                }

        }

	strcat( netcdf_pathfile, args->date ); 
	strcat( netcdf_pathfile, "_" );

	if(args->scanType==SCAN_PPI) { sprintf(netcdf_pathfile + strlen(netcdf_pathfile), "ppi"); }
	if(args->scanType==SCAN_RHI) { sprintf(netcdf_pathfile + strlen(netcdf_pathfile), "rhi"); }
	if(args->scanType==SCAN_CSP) { sprintf(netcdf_pathfile + strlen(netcdf_pathfile), "s-p"); }
	if(args->scanType==SCAN_FIX) { sprintf(netcdf_pathfile + strlen(netcdf_pathfile), "fix"); }
        if(args->scanType==SCAN_SGL) { sprintf(netcdf_pathfile + strlen(netcdf_pathfile), "ray"); }

	switch(radar)
	{
		case ACROBAT_CODED_SPECTRA :
		case COPERNICUS_SPECTRA :
		case GALILEO_SPECTRA :
		case TEST_SPECTRA :
		{
			strcat( netcdf_pathfile, "-fft" );
			break;
		}
		case COPERNICUS_SPECTRA_RAPID :
		case GALILEO_SPECTRA_RAPID :
		case TEST_SPECTRA_RAPID :
                {
                        strcat( netcdf_pathfile, "-fft-rapid" );
                        break;
                }
	}

        sprintf(netcdf_pathfile + strlen(netcdf_pathfile), "-raw.nc");

	printf("netCDF creating : %s\n", netcdf_pathfile);
	
	status = nc_create( netcdf_pathfile, NC_NOCLOBBER | NC_SHARE, ncid);
	if (status != NC_NOERR) check_netcdf_handle_error(status);

}

void RNC_SetupGlobalAttributes( int ncid, int radar, URC_ScanStruct *scan, RSP_ParamStruct *param, int argc, char *argv[] )
{

	char		buffer[1024];
	int		temp;
	float		temp_float;
	int		status;
	int 		n;
	time_t 		now;
	char		*char_time;

        printf("netCDF : setup global attributes\n");


        /* radar */
	switch (radar)
	{
		case ACROBAT :
			strcpy ( buffer, "ACROBAT" ); 
			break;
        	case COPERNICUS :
			strcpy ( buffer, "COPERNICUS" );
			break;
		case GALILEO :
			strcpy ( buffer, "GALILEO" );
                        break;
		case TEST :
			strcpy ( buffer, "TEST-GALILEO" );
			break;
		case CAMRA :
			strcpy ( buffer, "CAMRA" );
                        break;
	}
	status = nc_put_att_text( ncid, NC_GLOBAL, "radar", strlen(buffer), buffer);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }

        /* source */
        switch (radar)
	{
		case ACROBAT :
                	strcpy ( buffer, "Advanced Clear-air Radar for Observing the Boundary layer And Troposphere (ACROBAT)");
			break;
		case COPERNICUS :
			strcpy ( buffer, "35 GHz radar (COPERNICUS)");
			break;
	        case GALILEO :
                        strcpy ( buffer, "94 GHz radar (GALILEO)");
                        break;
	        case TEST :
                        strcpy ( buffer, "94 GHz radar (TEST-GALILEO)");
                        break;
	        case CAMRA :
		        strcpy ( buffer, "3 GHz Advanced Meteorological Radar (CAMRa)");
		        break;
        }
        status = nc_put_att_text( ncid, NC_GLOBAL, "source", strlen(buffer), buffer);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }

	/* history */
        time(&now);
        char_time = asctime(gmtime(&now));
        strncpy(buffer, char_time, strlen(char_time) - 1);
	buffer[strlen(char_time) - 1] = '\0';
        strcat(buffer, " - " );
	for (n = 0; n < argc; n++ )
	{
		strcat(buffer, argv[n]);
		strcat(buffer, " ");
	}
        status = nc_put_att_text( ncid, NC_GLOBAL, "history", strlen(buffer), buffer);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }

	/* British_National_Grid_Reference */
	/* using the standard location for Chilbolton */ 
	strcpy (buffer, "SU394386");
        status = nc_put_att_text( ncid, NC_GLOBAL, "British_National_Grid_Reference", strlen(buffer), buffer);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }

	/* year */
 	/* extract year from date field in scan structure */
	strncpy( buffer, scan->date, 4);
	buffer[4] = '\0';
	temp = atoi(buffer);
	status = nc_put_att_int( ncid, NC_GLOBAL, "year", NC_INT, 1, &temp );
	if (status != NC_NOERR) {check_netcdf_handle_error(status); }

	/* month */
	/* extract month from date field in args structure */
	strncpy( buffer, &scan->date[4], 2);
	buffer[2] = '\0';
	temp = atoi(buffer);
        status = nc_put_att_int( ncid, NC_GLOBAL, "month", NC_INT, 1, &temp );
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }

 	/* day */
	/* extract day from date field in args structure */
	strncpy( buffer, &scan->date[6], 2);
        buffer[2] = '\0';
        temp = atoi(buffer);
        status = nc_put_att_int( ncid, NC_GLOBAL, "day", NC_INT, 1, &temp );
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }

	/* pulse_compression */
	temp = 0;
	if ( radar == ACROBAT ) { temp = 1; }
	status = nc_put_att_int( ncid, NC_GLOBAL, "pulse_compression", NC_INT, 1, &temp );
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }

	/* pulses_per_ray */
	if ( radar == CAMRA ) {
	  temp = param->pulses_per_daq_cycle; }
	else {
	  temp = param->spectra_averaged * param->pulses_per_daq_cycle;
	}
	status = nc_put_att_int( ncid, NC_GLOBAL, "pulses_per_ray", NC_INT, 1, &temp );
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }
	
	if ( radar != CAMRA ) {
	  /* pulses_coherently_averaged */
	  temp = param->pulses_coherently_averaged * param->number_of_codes;
	  status = nc_put_att_int( ncid, NC_GLOBAL, "pulses_coherently_averaged", NC_INT, 1, &temp);
	  if (status != NC_NOERR) { check_netcdf_handle_error(status); }

	  /* moments_averaged */
	  status = nc_put_att_int( ncid, NC_GLOBAL, "moments_averaged", NC_INT, 1, &(param->moments_averaged));
	  if (status != NC_NOERR) { check_netcdf_handle_error(status); }

	  /* spectra_averaged */
	  status = nc_put_att_int( ncid, NC_GLOBAL, "spectra_averaged", NC_INT, 1, &param->spectra_averaged);
	  if (status != NC_NOERR) { check_netcdf_handle_error(status); }

	  /* fft_bins_interpolated */
	  status = nc_put_att_int( ncid, NC_GLOBAL, "fft_bins_interpolated", NC_INT, 1, &param->fft_bins_interpolated);
	  if (status != NC_NOERR) { check_netcdf_handle_error(status); }
	}

	/* delay_clocks */
	status = nc_put_att_int( ncid, NC_GLOBAL, "delay_clocks", NC_INT, 1, &param->delay_clocks);
	if (status != NC_NOERR) { check_netcdf_handle_error(status); }

	/* ADC_channels */
	status = nc_put_att_int( ncid, NC_GLOBAL, "ADC_channels", NC_INT, 1, &param->ADC_channels);
	if (status != NC_NOERR) { check_netcdf_handle_error(status); }

	/* pulses_per_daq_cycle */
	status = nc_put_att_int( ncid, NC_GLOBAL, "pulses_per_daq_cycle", NC_INT, 1, &param->pulses_per_daq_cycle);
	if (status != NC_NOERR) { check_netcdf_handle_error(status); }

	/* samples_per_pulse */
	status = nc_put_att_int( ncid, NC_GLOBAL, "samples_per_pulse", NC_INT, 1, &param->samples_per_pulse);
	if (status != NC_NOERR) { check_netcdf_handle_error(status); }

	/* ADC_bits_per_sample */
	temp = 12; 
	status = nc_put_att_int( ncid, NC_GLOBAL, "ADC_bits_per_sample", NC_INT, 1, &temp);
	if (status != NC_NOERR) { check_netcdf_handle_error(status); }

	if (radar == CAMRA) {
	  /* radar_constant */
	  temp_float = 64.7f;
	  status = nc_put_att_float( ncid, NC_GLOBAL, "radar_constant", NC_FLOAT, 1, &temp_float);
	  /* receiver_gain */
	  temp_float = 45.5f;
	  status = nc_put_att_float( ncid, NC_GLOBAL, "receiver_gain", NC_FLOAT, 1, &temp_float);
	  /* cable_losses */
	  temp_float = 4.8f;
	  status = nc_put_att_float( ncid, NC_GLOBAL, "cable_losses", NC_FLOAT, 1, &temp_float);
	}

	/* scan_datetime */
	strcpy(buffer, scan->date);
	status = nc_put_att_text( ncid, NC_GLOBAL, "scan_datetime", strlen(buffer), buffer);
	if (status != NC_NOERR) { check_netcdf_handle_error(status); }
	
	/* scan_angle */	
	status = nc_put_att_float( ncid, NC_GLOBAL, "scan_angle", NC_FLOAT, 1, &scan->scan_angle);
	if (status != NC_NOERR) { check_netcdf_handle_error(status); }

	/* min_angle */
	status = nc_put_att_float( ncid, NC_GLOBAL, "min_angle", NC_FLOAT, 1, &scan->min_angle);
	if (status != NC_NOERR) { check_netcdf_handle_error(status); }

	/* max_angle */
	status = nc_put_att_float( ncid, NC_GLOBAL, "max_angle", NC_FLOAT, 1, &scan->max_angle);
	if (status != NC_NOERR) { check_netcdf_handle_error(status); }

	/* min_range */
        temp_float = param->range[0]; 
        status = nc_put_att_float( ncid, NC_GLOBAL, "min_range", NC_FLOAT, 1, &temp_float);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }

	/* max_range */
	temp_float = param->range[param->samples_per_pulse - 1];
	status = nc_put_att_float( ncid, NC_GLOBAL, "max_range", NC_FLOAT, 1, &temp_float);
	if (status != NC_NOERR) { check_netcdf_handle_error(status); }

	/* scan_velocity */
	status = nc_put_att_float( ncid, NC_GLOBAL, "scan_velocity", NC_FLOAT, 1, &scan->scan_velocity);
	if (status != NC_NOERR) { check_netcdf_handle_error(status); }	

	/* operator */
        strcpy ( buffer, scan->operator);
        status = nc_put_att_text( ncid, NC_GLOBAL, "operator", strlen(buffer), buffer);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }

	/* experiment_id */
	status = nc_put_att_int( ncid, NC_GLOBAL, "experiment_id", NC_INT, 1, &scan->experiment_id);
	if (status != NC_NOERR) { check_netcdf_handle_error(status); }

	/* scantype */
	if(scan->scanType==SCAN_PPI) strcpy( buffer, "PPI" );
	if(scan->scanType==SCAN_RHI) strcpy( buffer, "RHI" );
	if(scan->scanType==SCAN_CSP) strcpy( buffer, "S-P" );
	if(scan->scanType==SCAN_FIX) strcpy( buffer, "Fixed");
	if(scan->scanType==SCAN_SGL) strcpy( buffer, "Fixed");
	status = nc_put_att_text( ncid, NC_GLOBAL, "scantype", strlen(buffer), buffer);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }

	/* file_number */
	status = nc_put_att_int( ncid, NC_GLOBAL, "file_number", NC_INT, 1, &scan->file_number);
	if (status != NC_NOERR) { check_netcdf_handle_error(status); }

	/* scan_number */
	status = nc_put_att_int( ncid, NC_GLOBAL, "scan_number", NC_INT, 1, &scan->scan_number);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }

	if ( radar != CAMRA ) {
	  /* dump spectra */
	  status = nc_put_att_int( ncid, NC_GLOBAL, "dump_spectra", NC_INT, 1, &param->dump_spectra );
	  if (status != NC_NOERR) { check_netcdf_handle_error(status); }
	  
	  /* dump spectra rapid */
	  status = nc_put_att_int( ncid, NC_GLOBAL, "dump_spectra_rapid", NC_INT, 1, &param->dump_spectra_rapid );
	  if (status != NC_NOERR) { check_netcdf_handle_error(status); }
	}
}

void RNC_SetupPulse_Compression_Code ( int ncid, RSP_ParamStruct *param ) 
{

	int	variable_shape[3];
	int	variable_count[] = { 0, 0, 0 };
	int	variable_start[] = { 0, 0, 0 }; 
	
	int	code_channel_dim, code_dim, code_length_dim;
	int	pccid;
	char	buffer[255];

	int 	status;
	int 	n, q;
	float	temp;	

	/* dimension definitions */
	status = nc_def_dim( ncid, "code_channel", 2, &code_channel_dim);
	if (status != NC_NOERR) { check_netcdf_handle_error(status); }
	status = nc_def_dim( ncid, "code", param->number_of_codes, &code_dim);
	if (status != NC_NOERR) { check_netcdf_handle_error(status); }
	status = nc_def_dim( ncid, "code_length", param->code_length, &code_length_dim);
	if (status != NC_NOERR) { check_netcdf_handle_error(status); }

	/* variable definition */
	variable_shape[0] = code_channel_dim;
	variable_shape[1] = code_dim;
	variable_shape[2] = code_length_dim;
	status = nc_def_var( ncid, "pulse_compression_code", NC_FLOAT, 3, variable_shape, &pccid);
	if (status != NC_NOERR) { check_netcdf_handle_error(status); }
	strcpy( buffer, "pulse compression code");
	status = nc_put_att_text( ncid, pccid, "long_name", strlen(buffer), buffer);
	if (status != NC_NOERR) { check_netcdf_handle_error(status); }
	status = nc_put_att_text( ncid, pccid, "code_name", strlen(param->code_name), param->code_name);
	if (status != NC_NOERR) { check_netcdf_handle_error(status); }
	status = nc_put_att_float( ncid, pccid, "code_bit_period", NC_FLOAT, 1, &param->pulse_period);
	if (status != NC_NOERR) { check_netcdf_handle_error(status); }

	/* variable data */

	/* go from define to data */
 	status = nc_enddef(ncid);
	if (status != NC_NOERR) { check_netcdf_handle_error(status); }	

	/* the code will be translated in to the netCDF variable code by code */
        /* this is to avoid any confusion */
	variable_count[0] = 1;
        variable_count[1] = 1;
        variable_count[2] = param->code_length;
	for (n = 0; n < param->number_of_codes; n++)
	{
		status = nc_put_vara_short( ncid, pccid, variable_start, variable_count, &param->codes[n][0]);
		if (status != NC_NOERR) { check_netcdf_handle_error(status); }
		variable_start[1] = variable_start[1] + 1;			
	}

	temp = 0;
	variable_start[0] = 1;
	for (n = 0; n < param->number_of_codes; n++)
	{
		variable_start[1] = n;
		for (q = 0; q < param->code_length; q++)
		{	
			variable_start[2] = q;
			status = nc_put_var1_float( ncid, pccid, variable_start, &temp);		
		}
	}
	
	/* go from data to define */
	status = nc_redef(ncid);
	if (status != NC_NOERR) { check_netcdf_handle_error(status); }

}

int RNC_SetupFile_State( int ncid )
{
	int 	file_stateid;
        int     variable_shape[] = {1};
	int 	status;
	char 	buffer[255];
	int	temp;

	/* define variable file state */	
        status = nc_def_var( ncid, "file_state", NC_INT, 0, variable_shape, &file_stateid);
	if (status != NC_NOERR) { check_netcdf_handle_error(status); }	
	strcpy( buffer, "current_state_of_file");
	status = nc_put_att_text( ncid, file_stateid, "chilbolton_standard_name", strlen(buffer), buffer);
	if (status != NC_NOERR) { check_netcdf_handle_error(status); }
	strcpy( buffer, "current state of file");
	status = nc_put_att_text( ncid, file_stateid, "long_name", strlen(buffer), buffer);
	if (status != NC_NOERR) { check_netcdf_handle_error(status); }

        /* go from define to data */
        status = nc_enddef(ncid);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }

	/* set to 0 : scan in progress */
	temp = 0;	
	status = nc_put_var_int(ncid, file_stateid, &temp);
        if (status != NC_NOERR) check_netcdf_handle_error(status);

        /* go from data to define */
        status = nc_redef(ncid);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }

	return (file_stateid);
}

void RNC_SetupStaticVariables( int ncid, RSP_ParamStruct *param)
{
	
	int     variable_shape[] = {1};
	int	clockid, transmit_powerid, pulse_periodid, antenna_diameterid;
	int	beamwidthVid, beamwidthHid, prfid, frequencyid;
	int	heightid, longitudeid, latitudeid;
	int	status;
	float	temp_float;
	char 	buffer[255];

	/* define variables */
	/* clock */
        status = nc_def_var( ncid, "clock", NC_FLOAT, 0, variable_shape, &clockid);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }
        strcpy( buffer, "clock");
        status = nc_put_att_text( ncid, clockid, "chilbolton_standard_name", strlen(buffer), buffer);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }
        strcpy( buffer, "clock input to ISACTRL");
        status = nc_put_att_text( ncid, clockid, "long_name", strlen(buffer), buffer);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }
	strcpy( buffer, "Hz");
        status = nc_put_att_text( ncid, clockid, "units", strlen(buffer), buffer);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }
	/* special attribute: clock_divfactor */
	status = nc_put_att_int( ncid, clockid, "clock_divfactor", NC_INT, 1, &param->clock_divfactor  );
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }	

	/* transmit_power */	
	status = nc_def_var( ncid, "transmit_power", NC_FLOAT, 0, variable_shape, &transmit_powerid);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }
        strcpy( buffer, "peak_transmitted_power");
        status = nc_put_att_text( ncid, transmit_powerid, "chilbolton_standard_name", strlen(buffer), buffer);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }
        strcpy( buffer, "peak transmitted power");
        status = nc_put_att_text( ncid, transmit_powerid, "long_name", strlen(buffer), buffer);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }
	strcpy( buffer, "W");
        status = nc_put_att_text( ncid, transmit_powerid, "units", strlen(buffer), buffer);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }

	/* pulse_period */
	status = nc_def_var( ncid, "pulse_period", NC_FLOAT, 0, variable_shape, &pulse_periodid);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }
        strcpy( buffer, "pulse_period");
        status = nc_put_att_text( ncid, pulse_periodid, "chilbolton_standard_name", strlen(buffer), buffer);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }
        strcpy( buffer, "pulse period");
        status = nc_put_att_text( ncid, pulse_periodid, "long_name", strlen(buffer), buffer);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }
        strcpy( buffer, "us");
        status = nc_put_att_text( ncid, pulse_periodid, "units", strlen(buffer), buffer);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }

	/* antenna_diameter */
        status = nc_def_var( ncid, "antenna_diameter", NC_FLOAT, 0, variable_shape, &antenna_diameterid);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }
        strcpy( buffer, "antenna_diameter");
        status = nc_put_att_text( ncid, antenna_diameterid, "chilbolton_standard_name", strlen(buffer), buffer);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }
        strcpy( buffer, "antenna diameter");
        status = nc_put_att_text( ncid, antenna_diameterid, "long_name", strlen(buffer), buffer);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }
        strcpy( buffer, "m");
        status = nc_put_att_text( ncid, antenna_diameterid, "units", strlen(buffer), buffer);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }
        /* special attribute: antennae_separation */
        temp_float = 0;
        status = nc_put_att_float( ncid, antenna_diameterid, "antennae_separation", NC_FLOAT, 1, &temp_float);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }

	/* beamwidthV */
	status = nc_def_var( ncid, "beamwidthV", NC_FLOAT, 0, variable_shape, &beamwidthVid);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }
        strcpy( buffer, "vertical angular beamwidth");
        status = nc_put_att_text( ncid, beamwidthVid, "chilbolton_standard_name", strlen(buffer), buffer);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }
        strcpy( buffer, "vertical angular beamwidth");
        status = nc_put_att_text( ncid, beamwidthVid, "long_name", strlen(buffer), buffer);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }
        strcpy( buffer, "degree");
        status = nc_put_att_text( ncid, beamwidthVid, "units", strlen(buffer), buffer);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }

	/* beamwidthH */
	status = nc_def_var( ncid, "beamwidthH", NC_FLOAT, 0, variable_shape, &beamwidthHid);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }
        strcpy( buffer, "horizontal_angular_beamwidth");
        status = nc_put_att_text( ncid, beamwidthHid, "chilbolton_standard_name", strlen(buffer), buffer);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }
        strcpy( buffer, "horizontal angular beamwidth");
        status = nc_put_att_text( ncid, beamwidthHid, "long_name", strlen(buffer), buffer);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }
        strcpy( buffer, "degree");
        status = nc_put_att_text( ncid, beamwidthHid, "units", strlen(buffer), buffer);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }

	/* prf */
	status = nc_def_var( ncid, "prf", NC_FLOAT, 0, variable_shape, &prfid);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }
        strcpy( buffer, "pulse_repetition_frequency");
        status = nc_put_att_text( ncid, prfid, "chilbolton_standard_name", strlen(buffer), buffer);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }
        strcpy( buffer, "pulse repetition frequency");
        status = nc_put_att_text( ncid, prfid, "long_name", strlen(buffer), buffer);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }
        strcpy( buffer, "Hz");
        status = nc_put_att_text( ncid, prfid, "units", strlen(buffer), buffer);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }

	/* frequency */
	status = nc_def_var( ncid, "frequency", NC_FLOAT, 0, variable_shape, &frequencyid);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }
        strcpy( buffer, "frequency");
        status = nc_put_att_text( ncid, frequencyid, "chilbolton_standard_name", strlen(buffer), buffer);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }
        strcpy( buffer, "frequency of transmitted radiation");
        status = nc_put_att_text( ncid, frequencyid, "long_name", strlen(buffer), buffer);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }
        strcpy( buffer, "GHz");
        status = nc_put_att_text( ncid, frequencyid, "units", strlen(buffer), buffer);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }

	/* height */
	status = nc_def_var( ncid, "height", NC_FLOAT, 0, variable_shape, &heightid);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }
        strcpy( buffer, "height");
        status = nc_put_att_text( ncid, heightid, "chilbolton_standard_name", strlen(buffer), buffer);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }
        strcpy( buffer, "height of the elevation axis above mean sea level (Ordnance Survey Great Britain)");
        status = nc_put_att_text( ncid, heightid, "long_name", strlen(buffer), buffer);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }
        strcpy( buffer, "m");
        status = nc_put_att_text( ncid, heightid, "units", strlen(buffer), buffer);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }

	/* longitude */
	status = nc_def_var( ncid, "longitude", NC_FLOAT, 0, variable_shape, &longitudeid);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }
        strcpy( buffer, "longitude");
        status = nc_put_att_text( ncid, longitudeid, "chilbolton_standard_name", strlen(buffer), buffer);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }
        strcpy( buffer, "longitude of the antenna");
        status = nc_put_att_text( ncid, longitudeid, "long_name", strlen(buffer), buffer);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }
        strcpy( buffer, "degree_north");
        status = nc_put_att_text( ncid, longitudeid, "units", strlen(buffer), buffer);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }

	/* latitude */
	status = nc_def_var( ncid, "latitude", NC_FLOAT, 0, variable_shape, &latitudeid);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }
        strcpy( buffer, "latitude");
        status = nc_put_att_text( ncid, latitudeid, "chilbolton_standard_name", strlen(buffer), buffer);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }
        strcpy( buffer, "latitude of the antenna");
        status = nc_put_att_text( ncid, latitudeid, "long_name", strlen(buffer), buffer);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }
        strcpy( buffer, "degree_east");
        status = nc_put_att_text( ncid, latitudeid, "units", strlen(buffer), buffer);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }

	/* variable data */

        /* go from define to data */
        status = nc_enddef(ncid);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }

	/* clock */
        status = nc_put_var_float(ncid, clockid, &param->clock);
        if (status != NC_NOERR) check_netcdf_handle_error(status);
	
	/* transmit power */
	status = nc_put_var_float(ncid, transmit_powerid, &param->transmit_power);
        if (status != NC_NOERR) check_netcdf_handle_error(status);

	
	/* pulse period */
	status = nc_put_var_float(ncid, pulse_periodid, &param->pulse_period);
	if (status != NC_NOERR) check_netcdf_handle_error(status);

	/* antenna_diameter */
	status = nc_put_var_float(ncid, antenna_diameterid, &param->antenna_diameter);
	if (status != NC_NOERR) check_netcdf_handle_error(status);

	/* beamwidthH */
        status = nc_put_var_float(ncid, beamwidthHid, &param->beamwidthH);
        if (status != NC_NOERR) check_netcdf_handle_error(status);

	/* beamwidthV */
	status = nc_put_var_float(ncid, beamwidthVid, &param->beamwidthV);
	if (status != NC_NOERR) check_netcdf_handle_error(status);

	/* prf */
	status = nc_put_var_float(ncid, prfid, &param->prf);
	if (status != NC_NOERR) check_netcdf_handle_error(status);

	/* frequency */
	status = nc_put_var_float(ncid, frequencyid, &param->frequency);
	if (status != NC_NOERR) check_netcdf_handle_error(status);

	/* height */
        status = nc_put_var_float(ncid, heightid, &param->height);
        if (status != NC_NOERR) check_netcdf_handle_error(status);

	/* longitude */
        temp_float = 358.5630;
        status = nc_put_var_float(ncid, longitudeid, &temp_float);
        if (status != NC_NOERR) check_netcdf_handle_error(status);

	/* latitude */
	temp_float = 51.1445;
        status = nc_put_var_float( ncid, latitudeid, &temp_float);
        if (status != NC_NOERR) check_netcdf_handle_error(status);

        /* go from data to define */
        status = nc_redef(ncid);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }

}

void RNC_SetupRange( int ncid, RSP_ParamStruct *param, RNC_DimensionStruct *dimensions)
{

        int     variable_shape[] = {1};
	int	variable_count[1];
	int	variable_start[1];
        int     rangeid;
        int     status;

	int	temp_int;
	float	temp_float;

        char    buffer[255];
	
	/* variable definition */
        variable_shape[0] = dimensions->range_dim;
        status = nc_def_var( ncid, "range", NC_FLOAT, 1, variable_shape, &rangeid);
        if (status != NC_NOERR) check_netcdf_handle_error(status);
        strcpy(buffer, "distance from the antenna to the middle of each range gate");
        status = nc_put_att_text( ncid, rangeid, "long_name", strlen(buffer), buffer );
        if (status != NC_NOERR) check_netcdf_handle_error(status);
        strcpy(buffer,"m");
        status = nc_put_att_text( ncid, rangeid, "units", strlen(buffer), buffer );
        if (status != NC_NOERR) check_netcdf_handle_error(status);
	status = nc_put_att_float( ncid, rangeid, "applied_range_offset", NC_FLOAT, 1, &param->range_offset);
	if (status != NC_NOERR) check_netcdf_handle_error(status);
	temp_int = 1;
	status = nc_put_att_int( ncid, rangeid, "gates_range_averaged", NC_INT, 1, &temp_int);
	if (status != NC_NOERR) check_netcdf_handle_error(status);
	temp_float = 66;
	status = nc_put_att_float( ncid, rangeid, "gate_width", NC_FLOAT, 1, &temp_float);
	if (status != NC_NOERR) check_netcdf_handle_error(status);
	
       	/* go from define to data */
        status = nc_enddef(ncid);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }

        /* writing out the range */
        variable_count[0] = param->samples_per_pulse;
        variable_start[0] = 0;
        status = nc_put_vara_float( ncid, rangeid, variable_start, variable_count, param->range);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }

        /* go from data to define */
        status = nc_redef(ncid);
        if (status != NC_NOERR) { check_netcdf_handle_error(status); }

}


void RNC_SetupDimensions ( int ncid, RSP_ParamStruct *param, RNC_DimensionStruct *dimensions)
{
	int 	status;

	/* define time dimension */
        status = nc_def_dim( ncid, "time", NC_UNLIMITED, &dimensions->time_dim);
        if (status != NC_NOERR) check_netcdf_handle_error(status);

	/* define range dimension */
        status = nc_def_dim( ncid, "range", param->samples_per_pulse, &dimensions->range_dim);
        if (status != NC_NOERR) check_netcdf_handle_error(status);

	/* define fft_bin_dim */
	status = nc_def_dim( ncid, "fft_bin_dim", param->npsd, &dimensions->fft_bin_dim);
	if (status != NC_NOERR) check_netcdf_handle_error(status);

	/* define coded_fft_bin_dim */
	/* this line is going to come back and haunt me */
	status = nc_def_dim( ncid, "coded_fft_bin_dim", param->npsd/param->num_interleave, &dimensions->coded_fft_bin_dim);
	if (status != NC_NOERR) check_netcdf_handle_error(status);
	
	/* define spectra_number_dim */
	status = nc_def_dim( ncid, "spectra_number_dim", param->spectra_averaged, &dimensions->spectra_number_dim);
        if (status != NC_NOERR) check_netcdf_handle_error(status);
}	

void RNC_SetupRapidLogPSDDimensions ( int ncid, int radar,RSP_ParamStruct *param, RNC_DimensionStruct *dimensions )
{
	int     status;

        /* define time dimension */
        status = nc_def_dim( ncid, "time_range", NC_UNLIMITED, &dimensions->time_dim);
        if (status != NC_NOERR) check_netcdf_handle_error(status);

        /* define range dimension */
        status = nc_def_dim( ncid, "range", param->samples_per_pulse, &dimensions->range_dim);
        if (status != NC_NOERR) check_netcdf_handle_error(status);

	/* define fft_bin_dim */
	if (radar == COPERNICUS_SPECTRA_RAPID) {
/*		status = nc_def_dim( ncid, "fft_bin_dim", param->npsd / 2, &dimensions->fft_bin_dim); */
		status = nc_def_dim( ncid, "fft_bin_dim", param->npsd, &dimensions->fft_bin_dim); 
	} else {
        	status = nc_def_dim( ncid, "fft_bin_dim", param->npsd, &dimensions->fft_bin_dim);
       	}
	if (status != NC_NOERR) check_netcdf_handle_error(status);
}


void RNC_SetupDynamicVariables( int ncid, int radar, URC_ScanStruct *args, RSP_ParamStruct *param, RNC_DimensionStruct *dimensions, RSP_ObservablesStruct *obs ) 
{
	int 	variable_shape[3];
	char 	buffer[255];
	int 	status;
	int 	n;

	float 	temp_float;

	variable_shape[0] = dimensions->time_dim;
	variable_shape[1] = dimensions->range_dim;

	/* time definition */
	status = nc_def_var( ncid, "time", NC_FLOAT, 1, variable_shape, &obs->tsid);
	if (status != NC_NOERR) check_netcdf_handle_error(status);
	strcpy( buffer, "time");
        status = nc_put_att_text( ncid, obs->tsid, "chilbolton_standard_name", strlen(buffer), buffer);
	if (status != NC_NOERR) check_netcdf_handle_error(status);
	strcpy( buffer, "time");
	status = nc_put_att_text( ncid, obs->tsid, "long_name", strlen(buffer), buffer);
	if (status != NC_NOERR) check_netcdf_handle_error(status);
       	sprintf( buffer, "seconds since %c%c%c%c-%c%c-%c%c 00:00:00 +00:00", args->date[0], args->date[1],
                args->date[2], args->date[3], args->date[4], args->date[5],
                args->date[6], args->date[7]);
	status = nc_put_att_text( ncid, obs->tsid, "units", strlen(buffer), buffer);
        if (status != NC_NOERR) check_netcdf_handle_error(status);

	/* dish_time definition */
        status = nc_def_var( ncid, "dish_time", NC_FLOAT, 1, variable_shape, &obs->dish_tsid);
        if (status != NC_NOERR) check_netcdf_handle_error(status);
        strcpy( buffer, "dish_time");
        status = nc_put_att_text( ncid, obs->dish_tsid, "chilbolton_standard_name", strlen(buffer), buffer);
        if (status != NC_NOERR) check_netcdf_handle_error(status);
        strcpy( buffer, "dish_time");
        status = nc_put_att_text( ncid, obs->dish_tsid, "long_name", strlen(buffer), buffer);
        if (status != NC_NOERR) check_netcdf_handle_error(status);
        sprintf( buffer, "seconds since %04d-%02d-%02d 00:00:00 +00:00",
                obs->dish_year, obs->dish_month, obs->dish_day );
        status = nc_put_att_text( ncid, obs->dish_tsid, "units", strlen(buffer), buffer);
        if (status != NC_NOERR) check_netcdf_handle_error(status);

	/* elevation definition */
	status = nc_def_var( ncid, "elevation", NC_FLOAT, 1, variable_shape, &obs->elevationid);
	if (status != NC_NOERR) check_netcdf_handle_error(status);
        strcpy( buffer, "elevation angle above the horizon at the start of the beamwidth");
        status = nc_put_att_text( ncid, obs->elevationid, "long_name", strlen(buffer), buffer);
        if (status != NC_NOERR) check_netcdf_handle_error(status);
	strcpy( buffer, "degree");
        status = nc_put_att_text( ncid, obs->elevationid, "units", strlen(buffer), buffer);
        if (status != NC_NOERR) check_netcdf_handle_error(status);

	/* azimuth definition */
        status = nc_def_var( ncid, "azimuth", NC_FLOAT, 1, variable_shape, &obs->azimuthid);
        if (status != NC_NOERR) check_netcdf_handle_error(status);
        strcpy( buffer, "azimuth angle clockwise from the grid north at the start of the beamwidth");
        status = nc_put_att_text( ncid, obs->azimuthid, "long_name", strlen(buffer), buffer);
        if (status != NC_NOERR) check_netcdf_handle_error(status);
        strcpy( buffer, "degree");
        status = nc_put_att_text( ncid, obs->azimuthid, "units", strlen(buffer), buffer);
        if (status != NC_NOERR) check_netcdf_handle_error(status);
	status = nc_put_att_float( ncid, obs->azimuthid, "azimuth_offset", NC_FLOAT, 1, &param->azimuth_offset);
	if (status != NC_NOERR) check_netcdf_handle_error(status);

	switch(radar)
	{
		case COPERNICUS_SPECTRA_RAPID :
	  	case GALILEO_SPECTRA_RAPID :
		case TEST_SPECTRA_RAPID :
	    	{
	      		/* bin_number */
	      		status = nc_def_var( ncid, "bin_number", NC_INT, 1, variable_shape, &obs->bin_numberid);
	      		if (status != NC_NOERR) check_netcdf_handle_error(status);
	      		strcpy( buffer, "bin_number");
	      		status = nc_put_att_text( ncid, obs->bin_numberid, "long_name", strlen(buffer), buffer);
	      		if (status != NC_NOERR) check_netcdf_handle_error(status);
	      		strcpy( buffer, "1");
	      		status = nc_put_att_text( ncid, obs->bin_numberid, "units", strlen(buffer), buffer);
	      		if (status != NC_NOERR) check_netcdf_handle_error(status);
	      
	      		/* ray_number */
	      		status = nc_def_var( ncid, "ray_number", NC_INT, 1, variable_shape, &obs->ray_numberid);
	      		if (status != NC_NOERR) check_netcdf_handle_error(status);
	      		strcpy( buffer, "ray_number");
	      		status = nc_put_att_text( ncid, obs->ray_numberid, "long_name", strlen(buffer), buffer);
	      		if (status != NC_NOERR) check_netcdf_handle_error(status);
	      		strcpy( buffer, "1");
	      		status = nc_put_att_text( ncid, obs->ray_numberid, "units", strlen(buffer), buffer);
	      		if (status != NC_NOERR) check_netcdf_handle_error(status);
			break;
	    	}
	  	case ACROBAT :
	  	case COPERNICUS :
	  	case GALILEO :
		case CAMRA :
		case TEST :
	    	{	
	      		printf("Number of observations %d \n", obs->n_obs);
	      		for (n = 0; n < obs->n_obs; n++ )
			{
		  		if ( obs->record_observable[n])
		    		{
		      			if (strcmp(obs->name[n], "SNR_HC") == 0)
					{
			  			status = nc_def_var( ncid, "SNR_HC", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "signal_to_noise_ratio_at_horizontal_polarisation_from_spectral_processing");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "signal to noise ratio at horizontal polarisation from spectral processing");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "dB");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
					}	
		      			if (strcmp(obs->name[n], "SNR_HCP") == 0)
					{
			  			status = nc_def_var( ncid, "SNR_HCP", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "signal_to_noise_ratio_at_horizontal_polarisation_from_spectral_processing_using_pulse_compression");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "signal to noise ratio at horizontal polarisation from spectral processing using pulse compression");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "dB");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
					}
		      			if (strcmp(obs->name[n], "SNR_VC") == 0)
					{
			  			status = nc_def_var( ncid, "SNR_VC", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "signal_to_noise_ratio_at_vertical_polarisation_from_spectral_processing");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "signal to noise ratio at vertical polarisation from spectral processing");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "dB");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
					}
		      			if (strcmp(obs->name[n], "SNR_VCP") == 0)
					{
			  			status = nc_def_var( ncid, "SNR_VCP", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "signal_to_noise_ratio_at_vertical_polarisation_from_spectral_processing_using_pulse_compression");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "signal to noise ratio at vertical polarisation from spectral processing using pulse compression");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "dB");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
					}
		      			if (strcmp(obs->name[n], "SNR_XHC") == 0)
					{
			  			status = nc_def_var( ncid, "SNR_XHC", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "crosspolar_signal_to_noise_ratio_at_horizontal_polarisation_from_spectral_processing");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "crosspolar signal to noise ratio at horizontal polarisation from spectral processing");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "dB");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
					}
		      			if (strcmp(obs->name[n], "SNR_XHCP") == 0)
					{
			  			status = nc_def_var( ncid, "SNR_XHCP", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "crosspolar_signal_to_noise_ratio_at_horizontal_polarisation_from_spectral_processing_using_pulse_compression");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "crosspolar signal to noise ratio at horizontal polarisation from spectral processing using pulse compression");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "dB");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
					}
		      			if (strcmp(obs->name[n], "SNR_XVC") == 0)
					{
			  			status = nc_def_var( ncid, "SNR_XVC", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "crosspolar_signal_to_noise_ratio_at_vertical_polarisation_from_spectral_processing");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "crosspolar signal to noise ratio at vertical polarisation from spectral processing");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "dB");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
					}
		      			if (strcmp(obs->name[n], "SNR_XVCP") == 0)
					{
			  			status = nc_def_var( ncid, "SNR_XVCP", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "crosspolar_signal_to_noise_ratio_at_vertical_polarisation_from_spectral_processing_using_pulse_compression");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "crosspolar signal to noise ratio at vertical polarisation from spectral processing using pulse compression");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "dB");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
					}
		      			if (strcmp(obs->name[n], "ZED_H") == 0)
					{
			  			status = nc_def_var( ncid, "ZED_H", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "radar_reflectivity_factor_at_horizontal_polarisation");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "radar reflectivity factor at horizontal polarisation");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "dBZ");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			status = nc_put_att_float( ncid, obs->varid[n], "applied_calibration_offset", NC_FLOAT, 1, &param->ZED_incoherent_calibration_offset);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
					}
					if (strcmp(obs->name[n], "ZED_V") == 0)
        	                  	{
                            			status = nc_def_var( ncid, "ZED_V", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
                            			if (status != NC_NOERR) check_netcdf_handle_error(status);
                            			strcpy( buffer, "radar_reflectivity_factor_at_vertical_polarisation");
                            			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
                            			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                            			strcpy( buffer, "radar reflectivity factor at vertical polarisation");
                            			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
                            			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                            			strcpy( buffer, "dBZ");
                            			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
                            			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                            			status = nc_put_att_float( ncid, obs->varid[n], "applied_calibration_offset", NC_FLOAT, 1, &param->ZED_incoherent_calibration_offset);
                            			if (status != NC_NOERR) check_netcdf_handle_error(status);
                          		}
                        		if (strcmp(obs->name[n], "ZED_XV") == 0)
                          		{
                            			status = nc_def_var( ncid, "ZED_XV", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
                            			if (status != NC_NOERR) check_netcdf_handle_error(status);
                            			strcpy( buffer, "crosspolar_radar_reflectivity_factor_at_vertical_polarisation");
                            			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
                            			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                            			strcpy( buffer, "crosspolar radar reflectivity factor at vertical polarisation");
                            			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
                            			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                            			strcpy( buffer, "dBZ");
                            			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
                            			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                            			status = nc_put_att_float( ncid, obs->varid[n], "applied_calibration_offset", NC_FLOAT, 1, &param->ZED_crosspolar_incoherent_calibration_offset);
                            			if (status != NC_NOERR) check_netcdf_handle_error(status);
  		                        }
		      			if (strcmp(obs->name[n], "ZED_XH") == 0)
					{
			  			status = nc_def_var( ncid, "ZED_XH", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "crosspolar_radar_reflectivity_factor_at_horizontal_polarisation");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "crosspolar radar reflectivity factor at horizontal polarisation");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "dBZ");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			status = nc_put_att_float( ncid, obs->varid[n], "applied_calibration_offset", NC_FLOAT, 1, &param->ZED_crosspolar_incoherent_calibration_offset);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
					}
		      			if (strcmp(obs->name[n], "CXC") == 0)
					{
			  			status = nc_def_var( ncid, "CXC", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "copolar_cross_correlation_coefficient");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "copolar cross correlation coefficient");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
					}
		     			if (strcmp(obs->name[n], "ZED_HP") == 0)
					{
			  			status = nc_def_var( ncid, "ZED_HP", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "radar_reflectivity_factor_at_horizontal_polarisation_using_pulse_compression");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			 			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "radar reflectivity factor at horizontal polarisation using pulse compression");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "dBZ");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
					}
		      			if (strcmp(obs->name[n], "ZED_HC") == 0)
					{
			  			status = nc_def_var( ncid, "ZED_HC", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "radar_reflectivity_factor_at_horizontal_polarisation_from_spectral_processing");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
						strcpy( buffer, "radar reflectivity factor at horizontal polarisation from spectral processing");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "dBZ");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			status = nc_put_att_float( ncid, obs->varid[n], "applied_calibration_offset", NC_FLOAT, 1, &param->ZED_calibration_offset);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
					}
		      			if (strcmp(obs->name[n], "ZED_HCD") == 0)
					{
			  			status = nc_def_var( ncid, "ZED_HCD", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "fractional_standard_deviation_of_radar_reflectivity_factor_at_horizontal_polarisation_from_spectral_processing");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "fractional standard deviation of radar reflectivity factor at horizontal polarisation from spectral processing");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "dB");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			status = nc_put_att_float( ncid, obs->varid[n], "applied_calibration_offset", NC_FLOAT, 1, &param->ZED_calibration_offset);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
					}
					if (strcmp(obs->name[n], "ZED_VCD") == 0)
                                        {
                                                status = nc_def_var( ncid, "ZED_VCD", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
                                                if (status != NC_NOERR) check_netcdf_handle_error(status);
                                                strcpy( buffer, "fractional_standard_deviation_of_radar_reflectivity_factor_at_vertical_polarisation_from_spectral_processing");
                                                status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
                                                if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                                                strcpy( buffer, "fractional standard deviation of radar reflectivity factor at vertical polarisation from spectral processing");
                                                status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
                                                if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                                                strcpy( buffer, "dB");
                                                status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
                                                if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                                                status = nc_put_att_float( ncid, obs->varid[n], "applied_calibration_offset", NC_FLOAT, 1, &param->ZED_calibration_offset);
                                                if (status != NC_NOERR) check_netcdf_handle_error(status);
                                        }
		      			if (strcmp(obs->name[n], "ZED_HCP") == 0)
					{
			  			status = nc_def_var( ncid, "ZED_HCP", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "radar_reflectivity_factor_at_horizontal_polarisation_from_spectral_processing_using_pulse_compression");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "radar reflectivity factor at horizontal polarisation from spectral processing using pulse compression");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "dBZ");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			status = nc_put_att_float( ncid, obs->varid[n], "applied_calibration_offset", NC_FLOAT, 1, &param->ZED_calibration_offset);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
					}
		      			if (strcmp(obs->name[n], "ZED_HCDP") == 0)
					{
			  			status = nc_def_var( ncid, "ZED_HCDP", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "fractional_standard_deviation_of_radar_reflectivity_factor_at_horizontal_polarisation_from_spectral_processing_using_pulse_compression");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "fractional standard deviation of radar reflectivity factor at horizontal polarisation from spectral processing using pulse compression");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "dB");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			status = nc_put_att_float( ncid, obs->varid[n], "applied_calibration_offset", NC_FLOAT, 1, &param->ZED_calibration_offset);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
					}
					if (strcmp(obs->name[n], "ZED_VCDP") == 0)
                                        {
                                                status = nc_def_var( ncid, "ZED_VCDP", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
                                                if (status != NC_NOERR) check_netcdf_handle_error(status);
                                                strcpy( buffer, "fractional_standard_deviation_of_radar_reflectivity_factor_at_vertical_polarisation_from_spectral_processing_using_pulse_compression");
                                                status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
                                                if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                                                strcpy( buffer, "fractional standard deviation of radar reflectivity factor at vertical polarisation from spectral processing using pulse compression");
                                                status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
                                                if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                                                strcpy( buffer, "dB");
                                                status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
                                                if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                                                status = nc_put_att_float( ncid, obs->varid[n], "applied_calibration_offset", NC_FLOAT, 1, &param->ZED_calibration_offset);
                                                if (status != NC_NOERR) check_netcdf_handle_error(status);
                                        }
		      			if (strcmp(obs->name[n], "ZED_XHC") == 0)
					{
			  			status = nc_def_var( ncid, "ZED_XHC", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "crosspolar_radar_reflectivity_factor_at_horizontal_polarisation_from_spectral_processing");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "crosspolar radar reflectivity factor at horizontal polarisation from spectral processing");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "dBZ");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			status = nc_put_att_float( ncid, obs->varid[n], "applied_calibration_offset", NC_FLOAT, 1, &param->ZED_calibration_offset);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
					}
		      			if (strcmp(obs->name[n], "ZED_XHCP") == 0)
					{
			  			status = nc_def_var( ncid, "ZED_XHCP", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "crosspolar_radar_reflectivity_factor_at_horizontal_polarisation_from_spectral_processing_using_pulse_compression");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			 			strcpy( buffer, "crosspolar radar reflectivity factor at horizontal polarisation from spectral processing using pulse compression");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "dBZ");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			status = nc_put_att_float( ncid, obs->varid[n], "applied_calibration_offset", NC_FLOAT, 1, &param->ZED_calibration_offset);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
					}
		      			if (strcmp(obs->name[n], "ZED_VC") == 0)
					{
			  			status = nc_def_var( ncid, "ZED_VC", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "radar_reflectivity_factor_at_vertical_polarisation_from_spectral_processing");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "radar reflectivity factor at vertical polarisation from spectral processing");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "dBZ");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			status = nc_put_att_float( ncid, obs->varid[n], "applied_calibration_offset", NC_FLOAT, 1, &param->ZED_calibration_offset);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
					}
		      			if (strcmp(obs->name[n], "ZED_VCP") == 0)
					{
			  			status = nc_def_var( ncid, "ZED_VCP", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "radar_reflectivity_factor_at_vertical_polarisation_from_spectral_processing_using_pulse_compression");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "radar reflectivity factor at vertical polarisation from spectral processing using pulse compression");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "dBZ");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			status = nc_put_att_float( ncid, obs->varid[n], "applied_calibration_offset", NC_FLOAT, 1, &param->ZED_calibration_offset);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
					}
		      			if (strcmp(obs->name[n], "ZED_XVC") == 0)
					{
			  			status = nc_def_var( ncid, "ZED_XVC", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "crosspolar_radar_reflectivity_factor_at_vertical_polarisation_from_spectral_processing");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "crosspolar radar reflectivity factor at vertical polarisation from spectral processing");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "dBZ");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			status = nc_put_att_float( ncid, obs->varid[n], "applied_calibration_offset", NC_FLOAT, 1, &param->ZED_calibration_offset);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
					}
		      			if (strcmp(obs->name[n], "ZED_XVCP") == 0)
					{
			  			status = nc_def_var( ncid, "ZED_XVCP", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "crosspolar_radar_reflectivity_factor_at_vertical_polarisation_from_spectral_processing_using_pulse_compression");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "crosspolar radar reflectivity factor at vertical polarisation from spectral processing using pulse compression");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "dBZ");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			status = nc_put_att_float( ncid, obs->varid[n], "applied_calibration_offset", NC_FLOAT, 1, &param->ZED_calibration_offset);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
					}
		      			if (strcmp(obs->name[n], "SPW_H") == 0)
					{
			  			status = nc_def_var( ncid, "SPW_H", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "doppler_spectral_width_at_horizontal_polarisation");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "doppler spectral width at horizontal polarisation");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			 			strcpy( buffer, "m s-1");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
					}
		      			if (strcmp(obs->name[n], "SPW_V") == 0)
					{
			  			status = nc_def_var( ncid, "SPW_V", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "doppler_spectral_width_at_vertical_polarisation");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "doppler spectral width at vertical polarisation");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			 			strcpy( buffer, "m s-1");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
					}
		      			if (strcmp(obs->name[n], "SPW_HV") == 0)
					{
			  			status = nc_def_var( ncid, "SPW_HV", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "doppler_spectral_width_at_horizontal_and_vertical_polarisation");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "doppler spectral width at horizontal and vertical polarisation");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			 			strcpy( buffer, "m s-1");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
					}

		      			if (strcmp(obs->name[n], "SPW_HC") == 0)
					{
			  			status = nc_def_var( ncid, "SPW_HC", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "doppler_spectral_width_at_horizontal_polarisation_from_spectral_processing");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "doppler spectral width at horizontal polarisation from spectral processing");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			 			strcpy( buffer, "m s-1");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
					}
					if (strcmp(obs->name[n], "SPW_VC") == 0)
                                        {
                                                status = nc_def_var( ncid, "SPW_VC", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
                                                if (status != NC_NOERR) check_netcdf_handle_error(status);
                                                strcpy( buffer, "doppler_spectral_width_at_vertical_polarisation_from_spectral_processing");
                                                status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
                                                if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                                                strcpy( buffer, "doppler spectral width at vertical polarisation from spectral processing");
                                                status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
                                                if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                                                strcpy( buffer, "m s-1");
                                                status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
                                                if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                                        }
		      			if (strcmp(obs->name[n], "SPW_HCP") == 0)
					{
			  			status = nc_def_var( ncid, "SPW_HCP", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "doppler_spectral_width_at_horizontal_polarisation_from_spectral_processing_using_pulse_compression");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "doppler spectral width at horizontal polarisation from spectral processing using pulse compression");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "m s-1");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
					}
					if (strcmp(obs->name[n], "SPW_VCP") == 0)
                                        {
                                                status = nc_def_var( ncid, "SPW_VCP", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
                                                if (status != NC_NOERR) check_netcdf_handle_error(status);
                                                strcpy( buffer, "doppler_spectral_width_at_vertical_polarisation_from_spectral_processing_using_pulse_compression");
                                                status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
                                                if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                                                strcpy( buffer, "doppler spectral width at vertical polarisation from spectral processing using pulse compression");
                                                status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
                                                if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                                                strcpy( buffer, "m s-1");
                                                status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
                                                if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                                        }
		      			if (strcmp(obs->name[n], "VEL_H") == 0)
					{
			  			status = nc_def_var( ncid, "VEL_H", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "radial_velocity_of_scatterers_away_from_instrument_at_horizontal_polarisation");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "radial velocity of scatterers away from instrument at horizontal polarisation");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "m s-1");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			status = nc_put_att_float( ncid, obs->varid[n], "folding_velocity", NC_FLOAT, 1, &param->folding_velocity);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
					}
		      			if (strcmp(obs->name[n], "VEL_V") == 0)
					{
			  			status = nc_def_var( ncid, "VEL_V", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "radial_velocity_of_scatterers_away_from_instrument_at_vertical_polarisation");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "radial velocity of scatterers away from instrument at vertical polarisation");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "m s-1");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			status = nc_put_att_float( ncid, obs->varid[n], "folding_velocity", NC_FLOAT, 1, &param->folding_velocity);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
					}
		      			if (strcmp(obs->name[n], "VEL_HV") == 0)
					{
			  			status = nc_def_var( ncid, "VEL_HV", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "radial_velocity_of_scatterers_away_from_instrument_at_horizontal_and_vertical_polarisation");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "radial velocity of scatterers away from instrument at horizontal and vertical polarisation");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "m s-1");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			status = nc_put_att_float( ncid, obs->varid[n], "folding_velocity", NC_FLOAT, 1, &param->folding_velocity);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
					}

		      			if (strcmp(obs->name[n], "VEL_HC") == 0)
					{
			  			status = nc_def_var( ncid, "VEL_HC", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "radial_velocity_of_scatterers_away_from_instrument_at_horizontal_polarisation_from_spectral_processing");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "radial velocity of scatterers away from instrument at horizontal polarisation from spectral processing");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "m s-1");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			status = nc_put_att_float( ncid, obs->varid[n], "folding_velocity", NC_FLOAT, 1, &param->folding_velocity);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
					}
					if (strcmp(obs->name[n], "VEL_VC") == 0)
                                        {
                                                status = nc_def_var( ncid, "VEL_VC", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
                                                if (status != NC_NOERR) check_netcdf_handle_error(status);
                                                strcpy( buffer, "radial_velocity_of_scatterers_away_from_instrument_at_vertical_polarisation_from_spectral_processing");
                                                status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
                                                if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                                                strcpy( buffer, "radial velocity of scatterers away from instrument at vertical polarisation from spectral processing");
                                                status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
                                                if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                                                strcpy( buffer, "m s-1");
                                                status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
                                                if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                                                status = nc_put_att_float( ncid, obs->varid[n], "folding_velocity", NC_FLOAT, 1, &param->folding_velocity);
                                                if (status != NC_NOERR) check_netcdf_handle_error(status);
                                        }
		      			if (strcmp(obs->name[n], "VEL_HCP") == 0)
					{
			  			status = nc_def_var( ncid, "VEL_HCP", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "radial_velocity_of_scatterers_away_from_instrument_at_horizontal_polarisation_from_spectral_processing_using_pulse_compression");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			 			strcpy( buffer, "radial velocity of scatterers away from instrument at horizontal polarisation from spectral processing using pulse compression");
			 			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "m s-1");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			status = nc_put_att_float( ncid, obs->varid[n], "folding_velocity", NC_FLOAT, 1, &param->folding_velocity);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
					}
					if (strcmp(obs->name[n], "VEL_VCP") == 0)
                                        {
                                                status = nc_def_var( ncid, "VEL_VCP", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
                                                if (status != NC_NOERR) check_netcdf_handle_error(status);
                                                strcpy( buffer, "radial_velocity_of_scatterers_away_from_instrument_at_vertical_polarisation_from_spectral_processing_using_pulse_compression");
                                                status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
                                                if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                                                strcpy( buffer, "radial velocity of scatterers away from instrument at vertical polarisation from spectral processing using pulse compression");
                                                status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
                                                if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                                                strcpy( buffer, "m s-1");
                                                status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
                                                if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                                                status = nc_put_att_float( ncid, obs->varid[n], "folding_velocity", NC_FLOAT, 1, &param->folding_velocity);
                                                if (status != NC_NOERR) check_netcdf_handle_error(status);
                                        }
		      			if (strcmp(obs->name[n], "VEL_HCD") == 0)
					{
			  			status = nc_def_var( ncid, "VEL_HCD", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "standard_deviation_of_radial_velocity_of_scatterers_away_from_instrument_at_horizontal_polarisation_from_spectral_processing");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "standard deviation of radial velocity of scatterers away from instrument at horizontal polarisation from spectral processing");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "m s-1");
			 			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			status = nc_put_att_float( ncid, obs->varid[n], "folding_velocity", NC_FLOAT, 1, &param->folding_velocity);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
					}
					if (strcmp(obs->name[n], "VEL_VCD") == 0)
                                        {
                                                status = nc_def_var( ncid, "VEL_VCD", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
                                                if (status != NC_NOERR) check_netcdf_handle_error(status);
                                                strcpy( buffer, "standard_deviation_of_radial_velocity_of_scatterers_away_from_instrument_at_vertical_polarisation_from_spectral_processing");
                                                status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
                                                if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                                                strcpy( buffer, "standard deviation of radial velocity of scatterers away from instrument at vertical polarisation from spectral processing");
                                                status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
                                                if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                                                strcpy( buffer, "m s-1");
                                                status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
                                                if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                                                status = nc_put_att_float( ncid, obs->varid[n], "folding_velocity", NC_FLOAT, 1, &param->folding_velocity);
                                                if (status != NC_NOERR) check_netcdf_handle_error(status);
                                        }

		      			if (strcmp(obs->name[n], "VEL_HCDP") == 0)
					{
			  			status = nc_def_var( ncid, "VEL_HCDP", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "standard_deviation_of_radial_velocity_of_scatterers_away_from_instrument_at_horizontal_polarisation_from_spectral_processing_using_pulse_compression");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "standard deviation of radial velocity of scatterers away from instrument at horizontal polarisation from spectral processing using pulse compression");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "m s-1");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			status = nc_put_att_float( ncid, obs->varid[n], "folding_velocity", NC_FLOAT, 1, &param->folding_velocity);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
					}
					if (strcmp(obs->name[n], "VEL_VCDP") == 0)
                                        {
                                                status = nc_def_var( ncid, "VEL_VCDP", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
                                                if (status != NC_NOERR) check_netcdf_handle_error(status);
                                                strcpy( buffer, "standard_deviation_of_radial_velocity_of_scatterers_away_from_instrument_at_vertical_polarisation_from_spectral_processing_using_pulse_compression");
                                                status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
                                                if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                                                strcpy( buffer, "standard deviation of radial velocity of scatterers away from instrument at vertical polarisation from spectral processing using pulse compression");
                                                status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
                                                if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                                                strcpy( buffer, "m s-1");
                                                status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
                                                if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                                                status = nc_put_att_float( ncid, obs->varid[n], "folding_velocity", NC_FLOAT, 1, &param->folding_velocity);
                                                if (status != NC_NOERR) check_netcdf_handle_error(status);
                                        }

		      			if (strcmp(obs->name[n], "PHI_H") == 0)
					{
			  			status = nc_def_var( ncid, "PHI_H", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "absolute_phase_at_horizontal_polarisation");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "absolute phase at horizontal polarisation");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "degree");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
					}
		      			if (strcmp(obs->name[n], "PHI_V") == 0)
					{
			  			status = nc_def_var( ncid, "PHI_V", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "absolute_phase_at_vertical_polarisation");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "absolute phase at vertical polarisation");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "degree");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
					}
		      			if (strcmp(obs->name[n], "PHI_HV") == 0)
					{
			  			status = nc_def_var( ncid, "PHI_HV", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "absolute_phase_at_horizontal_and_vertical_polarisation");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "absolute phase at horizontal and vertical polarisation");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "degree");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
					}
		      			if (strcmp(obs->name[n], "PHI_HD") == 0)
					{
			  			status = nc_def_var( ncid, "PHI_HD", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "standard_deviation_of_absolute_phase_at_horizontal_polarisation");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "standard_deviation_of_absolute phase at horizontal polarisation");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "degree");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
					}
		      			if (strcmp(obs->name[n], "PHI_VD") == 0)
					{
			  			status = nc_def_var( ncid, "PHI_VD", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "standard_deviation_of_absolute_phase_at_vertical_polarisation");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "standard_deviation_of_absolute phase at vertical polarisation");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "degree");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
					}
		      			if (strcmp(obs->name[n], "PHI_HVD") == 0)
					{
			  			status = nc_def_var( ncid, "PHI_HVD", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "standard_deviation_of_absolute_phase_at_horizontal_and_vertical_polarisation");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "standard_deviation_of_absolute phase at horizontal and vertical polarisation");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "degree");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
					}
					if (strcmp(obs->name[n], "PDP") == 0)
					{
			  			status = nc_def_var( ncid, "PDP", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "differential_phase_shift");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			 			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "differential phase shift");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "degree");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
					}
					if (strcmp(obs->name[n], "ZDR") == 0)
                          		{
                            			status = nc_def_var( ncid, "ZDR", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
                            			if (status != NC_NOERR) check_netcdf_handle_error(status);
                            			strcpy( buffer, "differential_reflectivity");
                            			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
                            			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                            			strcpy( buffer, "differential reflectivity");
                            			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
                            			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                            			strcpy( buffer, "dB");
                            			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
                            			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                            			status = nc_put_att_float( ncid, obs->varid[n], "applied_calibration_offset", NC_FLOAT, 1, &param->ZDR_calibration_offset);
                            			if (status != NC_NOERR) check_netcdf_handle_error(status);
                          		}
		      			if (strcmp(obs->name[n], "ZDR_C") == 0)
					{
			  			status = nc_def_var( ncid, "ZDR_C", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "differential_reflectivity_from_spectral_processing");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "differential reflectivity from spectral processing");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "dB");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			status = nc_put_att_float( ncid, obs->varid[n], "applied_calibration_offset", NC_FLOAT, 1, &param->ZDR_C_calibration_offset);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
					}
		      			if (strcmp(obs->name[n], "ZDR_CP") == 0)
					{
			  			status = nc_def_var( ncid, "ZDR_CP", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "differential_reflectivity_from_spectral_processing_using_pulse_compression");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "differential reflectivity from spectral processing using pulse compression");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "dB");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			status = nc_put_att_float( ncid, obs->varid[n], "applied_calibration_offset", NC_FLOAT, 1, &param->ZDR_CP_calibration_offset);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
					}
					if (strcmp(obs->name[n], "LDR") == 0)
                        		{
                          			status = nc_def_var( ncid, "LDR", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
                          			if (status != NC_NOERR) check_netcdf_handle_error(status);
                          			strcpy( buffer, "linear_depolarisation_ratio");
                          			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
                          			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                          			strcpy( buffer, "linear depolarisation ratio");
                          			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
                          			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                          			strcpy( buffer, "dB");
                          			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
                          			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                        		}
		      			if (strcmp(obs->name[n], "LDR_C") == 0)
					{
			  			status = nc_def_var( ncid, "LDR_C", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "linear_depolarisation_ratio_from_spectral_processing");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "linear depolarisation ratio from spectral processing");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "dB");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
					}
					if (strcmp(obs->name[n], "LDR_HC") == 0)
                                        {
                                                status = nc_def_var( ncid, "LDR_HC", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
                                                if (status != NC_NOERR) check_netcdf_handle_error(status);
                                                strcpy( buffer, "linear_depolarisation_ratio_at_horizontal_polarisation_from_spectral_processing");
                                                status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
                                                if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                                                strcpy( buffer, "linear depolarisation ratio at horizontal polarisation from spectral processing");
                                                status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
                                                if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                                                strcpy( buffer, "dB");
                                                status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
                                                if (status != NC_NOERR) { check_netcdf_handle_error(status); }
						status = nc_put_att_float( ncid, obs->varid[n], "applied_calibration_offset", NC_FLOAT, 1, &param->LDR_HC_calibration_offset);
						if (status != NC_NOERR) check_netcdf_handle_error(status);
                                        }
					if (strcmp(obs->name[n], "LDR_VC") == 0)
                                        {
                                                status = nc_def_var( ncid, "LDR_VC", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
                                                if (status != NC_NOERR) check_netcdf_handle_error(status);
                                                strcpy( buffer, "linear_depolarisation_ratio_at_vertical_polarisation_from_spectral_processing");
                                                status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
                                                if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                                                strcpy( buffer, "linear depolarisation ratio at vertical polarisation from spectral processing");
                                                status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
                                                if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                                                strcpy( buffer, "dB");
                                                status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
                                                if (status != NC_NOERR) { check_netcdf_handle_error(status); }
						status = nc_put_att_float( ncid, obs->varid[n], "applied_calibration_offset", NC_FLOAT, 1, &param->LDR_VC_calibration_offset);
                                                if (status != NC_NOERR) check_netcdf_handle_error(status);
                                        }
		      			if (strcmp(obs->name[n], "LDR_CP") == 0)
					{
			  			status = nc_def_var( ncid, "LDR_CP", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "linear_depolarisation_ratio_from_spectral_processing_using_pulse_compression");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "linear depolarisation ratio from spectral processing using pulse compression");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "dB");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
					}
					if (strcmp(obs->name[n], "LDR_HCP") == 0)
                                        {
                                                status = nc_def_var( ncid, "LDR_HCP", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
                                                if (status != NC_NOERR) check_netcdf_handle_error(status);
                                                strcpy( buffer, "linear_depolarisation_ratio_at_horizontal_polarisation_from_spectral_processing_using_pulse_compression");
                                                status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
                                                if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                                                strcpy( buffer, "linear depolarisation ratio at horizontal polarisation from spectral processing using pulse compression");
                                                status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
                                                if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                                                strcpy( buffer, "dB");
                                                status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
                                                if (status != NC_NOERR) { check_netcdf_handle_error(status); }
						status = nc_put_att_float( ncid, obs->varid[n], "applied_calibration_offset", NC_FLOAT, 1, &param->LDR_HCP_calibration_offset);
                                                if (status != NC_NOERR) check_netcdf_handle_error(status);
                                        }
					if (strcmp(obs->name[n], "LDR_VCP") == 0)
                                        {
                                                status = nc_def_var( ncid, "LDR_VCP", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
                                                if (status != NC_NOERR) check_netcdf_handle_error(status);
                                                strcpy( buffer, "linear_depolarisation_ratio_at_vertical_polarisation_from_spectral_processing_using_pulse_compression");
                                                status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
                                                if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                                                strcpy( buffer, "linear depolarisation ratio at vertical polarisation from spectral processing using pulse compression");
                                                status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
                                                if (status != NC_NOERR) { check_netcdf_handle_error(status); }
                                                strcpy( buffer, "dB");
                                                status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
                                                if (status != NC_NOERR) { check_netcdf_handle_error(status); }
						status = nc_put_att_float( ncid, obs->varid[n], "applied_calibration_offset", NC_FLOAT, 1, &param->LDR_VCP_calibration_offset);
                                                if (status != NC_NOERR) check_netcdf_handle_error(status);
                                        }
					if (strcmp(obs->name[n], "PHIDP_C") == 0)
					{
			  			status = nc_def_var( ncid, "PHIDP_C", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "differential_phase_shift");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			 			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "differential phase shift");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "degree");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
					}
					if (strcmp(obs->name[n], "PHIDP_CP") == 0)
					{
			  			status = nc_def_var( ncid, "PHIDP_CP", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "differential_phase_shift_using_pulse_compression");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			 			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "differential phase shift using pulse compression");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "degree");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
					}
					if (strcmp(obs->name[n], "RHOHV_C") == 0)
					{
			  			status = nc_def_var( ncid, "RHOHV_C", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "copolar_cross_correlation_coefficient");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			 			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "copolar cross correlation coefficient");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
					}
					if (strcmp(obs->name[n], "RHOHV_CP") == 0)
					{
			  			status = nc_def_var( ncid, "RHOHV_CP", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "copolar_cross_correlation_coefficient_using_pulse_compression");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			 			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "copolar cross correlation coefficient using pulse compression");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
					}
					if (strcmp(obs->name[n], "NPC_H") == 0)
					{
			  			status = nc_def_var( ncid, "NPC_H", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "copolar_noise_power_counts");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			 			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "copolar noise power ADC counts");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "dB");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
					}
					if (strcmp(obs->name[n], "NPC_V") == 0)
					{
			  			status = nc_def_var( ncid, "NPC_V", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "cross_polar_noise_power_counts");
			  			status = nc_put_att_text( ncid, obs->varid[n], "chilbolton_standard_name", strlen(buffer), buffer);
			 			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "crosspolar noise power ADC counts");
			  			status = nc_put_att_text( ncid, obs->varid[n], "long_name", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
			  			strcpy( buffer, "dB");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
					}
					if (strcmp(obs->name[n], "I_UNCH") == 0)
					{
			  			status = nc_def_var( ncid, "I_UNCH", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
					}
					if (strcmp(obs->name[n], "Q_UNCH") == 0)
					{
			  			status = nc_def_var( ncid, "Q_UNCH", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
					}
					if (strcmp(obs->name[n], "I_UNCV") == 0)
					{
			  			status = nc_def_var( ncid, "I_UNCV", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
					}
					if (strcmp(obs->name[n], "Q_UNCV") == 0)
					{
			  			status = nc_def_var( ncid, "Q_UNCV", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
					}
					if (strcmp(obs->name[n], "I_CODH") == 0)
					{
			  			status = nc_def_var( ncid, "I_CODH", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
					}
					if (strcmp(obs->name[n], "Q_CODH") == 0)
					{
			  			status = nc_def_var( ncid, "Q_CODH", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
					}
					if (strcmp(obs->name[n], "I_CODV") == 0)
					{
			  			status = nc_def_var( ncid, "I_CODV", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
					}
					if (strcmp(obs->name[n], "Q_CODV") == 0)
					{
			  			status = nc_def_var( ncid, "Q_CODV", NC_FLOAT, 2, variable_shape, &obs->varid[n]);
			  			if (status != NC_NOERR) check_netcdf_handle_error(status);
			  			strcpy( buffer, "");
			  			status = nc_put_att_text( ncid, obs->varid[n], "units", strlen(buffer), buffer);
			  			if (status != NC_NOERR) { check_netcdf_handle_error(status); }
					}
					/* apply missing value */
					temp_float = -999;
                        		printf("applying missing value to: %s", buffer);
					status = nc_inq_varname( ncid, obs->varid[n], buffer);
					if (status != NC_NOERR) { check_netcdf_handle_error(status); }
					status = nc_put_att_float( ncid, obs->varid[n], "missing_value", NC_FLOAT, 1, &temp_float);
					if (status != NC_NOERR) { check_netcdf_handle_error(status); }
					status = nc_put_att_float( ncid, obs->varid[n], "_FillValue", NC_FLOAT, 1, &temp_float);
					if (status != NC_NOERR) { check_netcdf_handle_error(status); }
					printf("\n");
				} 
		     	} 
		break;
		}
	}
}

void RNC_SetupLogPSDVariables( int ncid, int radar, RSP_ParamStruct *param, RNC_DimensionStruct *dimensions, int PSD_varid[] )
{
        int     variable_shape[4];
        int     status;

        variable_shape[0] = dimensions->time_dim;
        variable_shape[1] = dimensions->range_dim;
        variable_shape[2] = dimensions->fft_bin_dim;

        switch(radar)
        {
                case COPERNICUS_CODED_SPECTRA :
                {
                        status = nc_def_var( ncid, "PSD_HH", NC_SHORT, 3, variable_shape, &PSD_varid[PSD_HH]);
                        if (status != NC_NOERR) check_netcdf_handle_error(status);
                        status = nc_def_var( ncid, "PSD_HV", NC_SHORT, 3, variable_shape, &PSD_varid[PSD_HV]);
			if (param->num_tx_pol == 2) {
                        	if (status != NC_NOERR) check_netcdf_handle_error(status);
				status = nc_def_var( ncid, "PSD_VV", NC_SHORT, 3, variable_shape, &PSD_varid[PSD_VV]);
                        	if (status != NC_NOERR) check_netcdf_handle_error(status);
                        	status = nc_def_var( ncid, "PSD_VH", NC_SHORT, 3, variable_shape, &PSD_varid[PSD_VH]);
			}
                        if (status != NC_NOERR) check_netcdf_handle_error(status);
                        status = nc_def_var( ncid, "PSD_HHP", NC_SHORT, 3, variable_shape, &PSD_varid[PSD_HHP]);
                        if (status != NC_NOERR) check_netcdf_handle_error(status);
                        status = nc_def_var( ncid, "PSD_HVP", NC_SHORT, 3, variable_shape, &PSD_varid[PSD_HVP]);
			if (param->num_tx_pol == 2) {
				if (status != NC_NOERR) check_netcdf_handle_error(status);
				status = nc_def_var( ncid, "PSD_VVP", NC_SHORT, 3, variable_shape, &PSD_varid[PSD_VVP]);
                        	if (status != NC_NOERR) check_netcdf_handle_error(status);
                        	status = nc_def_var( ncid, "PSD_VHP", NC_SHORT, 3, variable_shape, &PSD_varid[PSD_VHP]);
			}
			if (param->include_iq_in_spectra == 1)
			{
                        	if (status != NC_NOERR) check_netcdf_handle_error(status);
				variable_shape[1] = dimensions->spectra_number_dim;
				variable_shape[2] = dimensions->fft_bin_dim;
		        	variable_shape[3] = dimensions->range_dim;
                        	if (status != NC_NOERR) check_netcdf_handle_error(status);
				status = nc_def_var( ncid, "IPF_HH", NC_SHORT, 4, variable_shape, &PSD_varid[IPF_HH]);
                        	if (status != NC_NOERR) check_netcdf_handle_error(status);
                        	status = nc_def_var( ncid, "IPF_HV", NC_SHORT, 4, variable_shape, &PSD_varid[IPF_HV]);
                        	if (status != NC_NOERR) check_netcdf_handle_error(status);
				variable_shape[2] = dimensions->coded_fft_bin_dim;
                        	status = nc_def_var( ncid, "IPF_HHP", NC_LONG, 4, variable_shape, &PSD_varid[IPF_HHP]);
                        	if (status != NC_NOERR) check_netcdf_handle_error(status);
                        	status = nc_def_var( ncid, "IPF_HVP", NC_LONG, 4, variable_shape, &PSD_varid[IPF_HVP]);
                        	if (status != NC_NOERR) check_netcdf_handle_error(status);
				variable_shape[2] = dimensions->fft_bin_dim;	
                        	status = nc_def_var( ncid, "QPF_HH", NC_SHORT, 4, variable_shape, &PSD_varid[QPF_HH]);
                        	if (status != NC_NOERR) check_netcdf_handle_error(status);
                        	status = nc_def_var( ncid, "QPF_HV", NC_SHORT, 4, variable_shape, &PSD_varid[QPF_HV]);
                        	if (status != NC_NOERR) check_netcdf_handle_error(status);
				variable_shape[2] = dimensions->coded_fft_bin_dim;
                        	status = nc_def_var( ncid, "QPF_HHP", NC_LONG, 4, variable_shape, &PSD_varid[QPF_HHP]);
                        	if (status != NC_NOERR) check_netcdf_handle_error(status);
                        	status = nc_def_var( ncid, "QPF_HVP", NC_LONG, 4, variable_shape, &PSD_varid[QPF_HVP]);
                        	if (status != NC_NOERR) check_netcdf_handle_error(status);
				if (param->num_tx_pol == 2) {
					variable_shape[2] = dimensions->fft_bin_dim;
					status = nc_def_var( ncid, "IPF_VV", NC_SHORT, 4, variable_shape, &PSD_varid[IPF_VV]);
                                        if (status != NC_NOERR) check_netcdf_handle_error(status);
                                        status = nc_def_var( ncid, "IPF_VH", NC_SHORT, 4, variable_shape, &PSD_varid[IPF_VH]);
					variable_shape[2] = dimensions->coded_fft_bin_dim;
                                        if (status != NC_NOERR) check_netcdf_handle_error(status);
					status = nc_def_var( ncid, "IPF_VVP", NC_LONG, 4, variable_shape, &PSD_varid[IPF_VVP]);
                                        if (status != NC_NOERR) check_netcdf_handle_error(status);
                                        status = nc_def_var( ncid, "IPF_VHP", NC_LONG, 4, variable_shape, &PSD_varid[IPF_VHP]);
					variable_shape[2] = dimensions->fft_bin_dim;
                                        if (status != NC_NOERR) check_netcdf_handle_error(status);
					status = nc_def_var( ncid, "QPF_VV", NC_SHORT, 4, variable_shape, &PSD_varid[QPF_VV]);
                                        if (status != NC_NOERR) check_netcdf_handle_error(status);
                                        status = nc_def_var( ncid, "QPF_VH", NC_SHORT, 4, variable_shape, &PSD_varid[QPF_VH]);
					variable_shape[2] = dimensions->coded_fft_bin_dim;
                                        if (status != NC_NOERR) check_netcdf_handle_error(status);
					status = nc_def_var( ncid, "QPF_VVP", NC_LONG, 4, variable_shape, &PSD_varid[QPF_VVP]);
                        		if (status != NC_NOERR) check_netcdf_handle_error(status);
                        		status = nc_def_var( ncid, "QPF_VHP", NC_LONG, 4, variable_shape, &PSD_varid[QPF_VHP]);
                        		if (status != NC_NOERR) check_netcdf_handle_error(status);
				}
			}
                        break;
                }
		case COPERNICUS_SPECTRA_RAPID :
                {
                        variable_shape[0] = dimensions->time_dim;
                        variable_shape[1] = dimensions->fft_bin_dim;
                        status = nc_def_var( ncid, "PSD_RAPID_HHP", NC_SHORT, 2, variable_shape, &PSD_varid[0]);
                        if (status != NC_NOERR) check_netcdf_handle_error(status);
                        status = nc_def_var( ncid, "PSD_RAPID_HH", NC_SHORT, 2, variable_shape, &PSD_varid[1]);
                        if (status != NC_NOERR) check_netcdf_handle_error(status);
			break;
                }
                case ACROBAT_CODED_SPECTRA :
                {
                        status = nc_def_var( ncid, "PSD_HH", NC_SHORT, 3, variable_shape, &PSD_varid[0]);
                        if (status != NC_NOERR) check_netcdf_handle_error(status);
			break;
                }
                case GALILEO_SPECTRA :
		case TEST_SPECTRA :
                {
                        status = nc_def_var( ncid, "PSD_HH", NC_SHORT, 3, variable_shape, &PSD_varid[0]);
                        if (status != NC_NOERR) check_netcdf_handle_error(status);
			variable_shape[1] = dimensions->spectra_number_dim;
                        variable_shape[2] = dimensions->fft_bin_dim;
                        variable_shape[3] = dimensions->range_dim;
			status = nc_def_var( ncid, "IPF_HH", NC_SHORT, 4, variable_shape, &PSD_varid[1]);
                        if (status != NC_NOERR) check_netcdf_handle_error(status);
			status = nc_def_var( ncid, "QPF_HH", NC_SHORT, 4, variable_shape, &PSD_varid[2]);
                        if (status != NC_NOERR) check_netcdf_handle_error(status);
			break;
                }
		case GALILEO_SPECTRA_RAPID :
		case TEST_SPECTRA_RAPID :
		{
			variable_shape[0] = dimensions->time_dim;
		        variable_shape[1] = dimensions->fft_bin_dim;
                        status = nc_def_var( ncid, "PSD_RAPID_HH", NC_SHORT, 2, variable_shape, &PSD_varid[0]);
                        if (status != NC_NOERR) check_netcdf_handle_error(status);
			break;
                }	
        }
}

void RNC_WriteDynamicVariables( int ncid, RSP_ParamStruct *param, RSP_ObservablesStruct *obs )
{
        int     variable_count[2];
        int     variable_start[2];
        int     status;
	int	n;
	float	temp_float;

	/* write time */
	variable_start[0] = obs->ray_number;
	temp_float =  ((float) obs->hour * 3600.) + ((float) obs->minute * 60.) + ((float) obs->second) + ((float) obs->centisecond/100);
        status = nc_put_var1_float( ncid, obs->tsid, variable_start, &temp_float);
        if (status != NC_NOERR) check_netcdf_handle_error(status);

	/* write dish_time */
        variable_start[0] = obs->ray_number;
        temp_float =  ((float) obs->dish_hour * 3600.) + ((float) obs->dish_minute * 60.) + ((float) obs->dish_second) + ((float) obs->dish_centisecond/100);
        status = nc_put_var1_float( ncid, obs->dish_tsid, variable_start, &temp_float);
        if (status != NC_NOERR) check_netcdf_handle_error(status);


	/* write elevation */
        status = nc_put_var1_float( ncid, obs->elevationid, variable_start, &obs->elevation );
        if (status != NC_NOERR) check_netcdf_handle_error(status);
	
	/* write azimuth */
	temp_float = obs->azimuth + param->azimuth_offset;
        status = nc_put_var1_float( ncid, obs->azimuthid, variable_start, &temp_float );
        if (status != NC_NOERR) check_netcdf_handle_error(status);

	/* write radar observables */ 
	variable_start[0] = obs->ray_number ;
	variable_start[1] = 0;
	variable_count[0] = 1;
	variable_count[1] = param->samples_per_pulse;
       	for (n = 0; n < obs->n_obs; n++ )
       	{
		if ( obs->record_observable[n] )
		{
			status = nc_put_vara_float( ncid, obs->varid[n], variable_start, variable_count, obs->data[n]);
       			if (status != NC_NOERR) check_netcdf_handle_error(status);
		}
	}

        obs->ray_number = obs->ray_number + 1; 
}

void RNC_WriteLogPSDVariables( int ncid, int radar, RSP_ParamStruct *param, RSP_ObservablesStruct *obs, PolPSDStruct PSD[], IQStruct *IQStruct, int PSD_varid[] )
{
        int     variable_count[4];
        int     variable_start[4];
        int     status;
        int     n,j;
        float   temp_float;
	short int	*log_psd;


        variable_start[0] = obs->PSD_ray_number;
        variable_start[1] = 0;
        variable_start[2] = 0;
	variable_start[3] = 0;

        variable_count[0] = 1;
        variable_count[1] = 1;
        variable_count[2] = param->npsd;
	variable_count[3] = 0;

	/* create a short integer array to allow the results of log10 to be stored */
        log_psd = (short int *)calloc( param->npsd, sizeof(short int));
        if (log_psd == NULL) {
                printf("memory request for log_psd failed\n");
                exit(-1);
        }

        /* write time */
        temp_float =  ((float) obs->hour * 3600.) + ((float) obs->minute * 60.) + ((float) obs->second) + ((float) obs->centisecond/100);
        status = nc_put_var1_float( ncid, obs->tsid, variable_start, &temp_float);
        if (status != NC_NOERR) check_netcdf_handle_error(status);

        /* write elevation */
        status = nc_put_var1_float( ncid, obs->elevationid, variable_start, &obs->elevation );
        if (status != NC_NOERR) check_netcdf_handle_error(status);

        /* write azimuth */
        temp_float = obs->azimuth + param->azimuth_offset;
        status = nc_put_var1_float( ncid, obs->azimuthid, variable_start, &temp_float );
        if (status != NC_NOERR) check_netcdf_handle_error(status);

        switch(radar)
        {
                case COPERNICUS_CODED_SPECTRA :
                {
		if (param->include_iq_in_spectra == 1)
		{
			/* write out IQ data first for all gates in one go */
			variable_count[1] = param->spectra_averaged;
			variable_count[2] = param->npsd;
			variable_count[3] = param->samples_per_pulse;
			printf ("ipf_hh\n");
			/* write out IPF_HH */
			status = nc_put_vara_short( ncid, PSD_varid[IPF_HH], variable_start, variable_count, IQStruct->I_uncoded_copolar_H);
			if (status != NC_NOERR) check_netcdf_handle_error(status);	
			 printf ("ipf_hv\n");
			/* write out IPF_HV */
			status = nc_put_vara_short( ncid, PSD_varid[IPF_HV], variable_start, variable_count, IQStruct->I_uncoded_crosspolar_H);
               	 	if (status != NC_NOERR) check_netcdf_handle_error(status);
			variable_count[2] = param->npsd/param->num_interleave;
			 printf ("ipf_hhp\n");
			/* write out IPF_HHP */
	                status = nc_put_vara_long( ncid, PSD_varid[IPF_HHP], variable_start, variable_count, IQStruct->I_coded_copolar_H);
	                if (status != NC_NOERR) check_netcdf_handle_error(status);
			printf ("ipf_hvp\n");
			/* write out IPF_HVP */
	                status = nc_put_vara_long( ncid, PSD_varid[IPF_HVP], variable_start, variable_count, IQStruct->I_coded_crosspolar_H);
	                if (status != NC_NOERR) check_netcdf_handle_error(status);		
			variable_count[2] = param->npsd;
			printf ("qpf_hh\n");
			/* write out QPF_HH */
	                status = nc_put_vara_short( ncid, PSD_varid[QPF_HH], variable_start, variable_count, IQStruct->Q_uncoded_copolar_H);
	                if (status != NC_NOERR) check_netcdf_handle_error(status);
			printf ("qpf_hv\n");
	                /* write out QPF_HV */
	                status = nc_put_vara_short( ncid, PSD_varid[QPF_HV], variable_start, variable_count, IQStruct->Q_uncoded_crosspolar_H);
	                if (status != NC_NOERR) check_netcdf_handle_error(status);
			variable_count[2] = param->npsd/param->num_interleave;
			printf ("qpf_hhp\n");
	                /* write out QPF_HHP */
	                status = nc_put_vara_long( ncid, PSD_varid[QPF_HHP], variable_start, variable_count, IQStruct->Q_coded_copolar_H);
	                if (status != NC_NOERR) check_netcdf_handle_error(status);
			printf ("ipf_hvp\n");
	                /* write out QPF_HVP */
	                status = nc_put_vara_long( ncid, PSD_varid[QPF_HVP], variable_start, variable_count, IQStruct->Q_coded_crosspolar_H);
	                if (status != NC_NOERR) check_netcdf_handle_error(status);
			printf("we are done with i and q dump\n");
			if (param->num_tx_pol == 2) {
				variable_count[2] = param->npsd;
				/* write out IPF_VV */
                                status = nc_put_vara_short( ncid, PSD_varid[IPF_VV], variable_start, variable_count, IQStruct->I_uncoded_copolar_V);
                                if (status != NC_NOERR) check_netcdf_handle_error(status);
                                /* write out IPF_VH */
                                status = nc_put_vara_short( ncid, PSD_varid[IPF_VH], variable_start, variable_count, IQStruct->I_uncoded_crosspolar_V);
                                if (status != NC_NOERR) check_netcdf_handle_error(status);
				variable_count[2] = param->npsd/param->num_interleave;
				/* write out IPF_VVP */
                                status = nc_put_vara_long( ncid, PSD_varid[IPF_VVP], variable_start, variable_count, IQStruct->I_coded_copolar_V);
                                if (status != NC_NOERR) check_netcdf_handle_error(status);
                                /* write out IPF_VHP */
                                status = nc_put_vara_long( ncid, PSD_varid[IPF_VHP], variable_start, variable_count, IQStruct->I_coded_crosspolar_V);
                                if (status != NC_NOERR) check_netcdf_handle_error(status);
				variable_count[2] = param->npsd;
				/* write out QPF_VV */
                                status = nc_put_vara_short( ncid, PSD_varid[QPF_VV], variable_start, variable_count, IQStruct->Q_uncoded_copolar_V);
                                if (status != NC_NOERR) check_netcdf_handle_error(status);
                                /* write out QPF_VH */
                                status = nc_put_vara_short( ncid, PSD_varid[QPF_VH], variable_start, variable_count, IQStruct->Q_uncoded_crosspolar_V);
                                if (status != NC_NOERR) check_netcdf_handle_error(status);
				variable_count[2] = param->npsd/param->num_interleave;
				/* write out QPF_VVP */
	                	status = nc_put_vara_long( ncid, PSD_varid[QPF_VVP], variable_start, variable_count, IQStruct->Q_coded_copolar_V);
	                	if (status != NC_NOERR) check_netcdf_handle_error(status);
	                	/* write out QPF_VHP */
	                	status = nc_put_vara_long( ncid, PSD_varid[QPF_VHP], variable_start, variable_count, IQStruct->Q_coded_crosspolar_V);
	                	if (status != NC_NOERR) check_netcdf_handle_error(status);
			}
		}
		
		/* one gate at a time */	
		variable_count[1] = 1;
		variable_count[2] = param->npsd;
		/* PSD_HH */
                for(n = 0; n < param->samples_per_pulse; n++)
                {
			/* calculate the log10 of the PSD */
			for( j = 0; j < param->npsd; j++) {
				log_psd[j] = (short int) 1000 * log10( PSD[n].HH[j] );
			}
                        variable_start[1] = n;
                        status = nc_put_vara_short( ncid, PSD_varid[PSD_HH], variable_start, variable_count, log_psd);
                        if (status != NC_NOERR) check_netcdf_handle_error(status);
                }
		/* PSD_HV */
                for(n = 0; n < param->samples_per_pulse; n++)
                {
			/* calculate the log10 of the PSD */
                        for( j = 0; j < param->npsd; j++) {
                                log_psd[j] = (short int) 1000 * log10( PSD[n].HV[j] );
                        }

                        variable_start[1] = n;
                        status = nc_put_vara_short( ncid, PSD_varid[PSD_HV], variable_start, variable_count, log_psd);
                        if (status != NC_NOERR) check_netcdf_handle_error(status);
                }
		if (param->num_tx_pol == 2) {
			/* PSD_VV */
			for(n = 0; n < param->samples_per_pulse; n++)
                	{
                	        /* calculate the log10 of the PSD */
                	        for( j = 0; j < param->npsd; j++) {
                	                log_psd[j] = (short int) 1000 * log10( PSD[n].VV[j] );
                	        }
                	        variable_start[1] = n;
                	        status = nc_put_vara_short( ncid, PSD_varid[2], variable_start, variable_count, log_psd);
                	        if (status != NC_NOERR) check_netcdf_handle_error(status);
                	}
			/* PSD_VH */
                	for(n = 0; n < param->samples_per_pulse; n++)
                	{
                	        /* calculate the log10 of the PSD */
                	        for( j = 0; j < param->npsd; j++) {
                	                log_psd[j] = (short int) 1000 * log10( PSD[n].VH[j] );
                	        }
                	        variable_start[1] = n;
                	        status = nc_put_vara_short( ncid, PSD_varid[PSD_VV], variable_start, variable_count, log_psd);
                	        if (status != NC_NOERR) check_netcdf_handle_error(status);
                	}
		}
		/* PSD_HHP */
                for(n = 0; n < param->samples_per_pulse; n++)
                {
			/* calculate the log10 of the PSD */
                        for( j = 0; j < param->npsd; j++) {
                                log_psd[j] = (short int) 1000 * log10( PSD[n].HHP[j] );
                        }
                        variable_start[1] = n;
                        status = nc_put_vara_short( ncid, PSD_varid[PSD_HHP], variable_start, variable_count, log_psd);
                        if (status != NC_NOERR) check_netcdf_handle_error(status);
                }
		/* PSD_HVP */
		for(n = 0; n < param->samples_per_pulse; n++)
                {
			/* calculate the log10 of the PSD */
                        for( j = 0; j < param->npsd; j++) {
                                log_psd[j] = (short int) 1000 * log10( PSD[n].HVP[j] );
                        }
                        variable_start[1] = n;
                        status = nc_put_vara_short( ncid, PSD_varid[PSD_HVP], variable_start, variable_count, log_psd);
                        if (status != NC_NOERR) check_netcdf_handle_error(status);
                }
		if (param->num_tx_pol == 2) {
			/* PSD_VVP */
			for(n = 0; n < param->samples_per_pulse; n++)
                	{
                        	/* calculate the log10 of the PSD */
                        	for( j = 0; j < param->npsd; j++) {
                        	        log_psd[j] = (short int) 1000 * log10( PSD[n].VVP[j] );
                        	}
                        	variable_start[1] = n;
                        	status = nc_put_vara_short( ncid, PSD_varid[PSD_VVP], variable_start, variable_count, log_psd);
                        	if (status != NC_NOERR) check_netcdf_handle_error(status);
                	}	
			/* PSD_VHP */
                	for(n = 0; n < param->samples_per_pulse; n++)
                	{
                	        /* calculate the log10 of the PSD */
                	        for( j = 0; j < param->npsd; j++) {
                	                log_psd[j] = (short int) 1000 * log10( PSD[n].VHP[j] );
                	        }
                	        variable_start[1] = n;
                	        status = nc_put_vara_short( ncid, PSD_varid[PSD_VHP], variable_start, variable_count, log_psd);
                	        if (status != NC_NOERR) check_netcdf_handle_error(status);
                	}
		}		
               break;
                }
                case ACROBAT_CODED_SPECTRA :
                {
                for(n = 0; n < param->samples_per_pulse; n++)
                {
			/* calculate the log10 of the PSD */
                        for( j = 0; j < param->npsd; j++) {
                                log_psd[j] = (short int) 1000 * log10( PSD[n].HH[j] );
                        }
                        variable_start[1] = n;
                        status = nc_put_vara_short( ncid, PSD_varid[0], variable_start, variable_count, log_psd);
                        if (status != NC_NOERR) check_netcdf_handle_error(status);
                }
                break;
		}
		case GALILEO_SPECTRA :
		case TEST_SPECTRA :
                {
		/* write out IQ data first for all gates in one go */
		variable_start[0] = obs->PSD_ray_number;
        	variable_start[1] = 0;
        	variable_start[2] = 0;
        	variable_start[3] = 0;
		variable_count[0] = 1;
                variable_count[1] = param->spectra_averaged;
                variable_count[2] = param->npsd;
                variable_count[3] = param->samples_per_pulse;
		/* write out IPF_HH */
                status = nc_put_vara_short( ncid, PSD_varid[1], (const size_t *) variable_start, (const size_t *) variable_count, (IQStruct->I_uncoded_copolar_H));
                if (status != NC_NOERR) check_netcdf_handle_error(status);
		/* write out QPF_HH */
                status = nc_put_vara_short( ncid, PSD_varid[2], variable_start, variable_count, (IQStruct->Q_uncoded_copolar_H));	
		if (status != NC_NOERR) check_netcdf_handle_error(status);
		/* one gate at a time */
		variable_count[0] = 1;
                variable_count[1] = 1;
                variable_count[2] = param->npsd;
                for(n = 0; n < param->samples_per_pulse; n++)
                {
			/* calculate the log10 of the PSD */
                        for( j = 0; j < param->npsd; j++) {
                                log_psd[j] = (short int) 1000 * log10( PSD[n].HH[j] );
                        }
                        variable_start[1] = n;
                        status = nc_put_vara_short( ncid, PSD_varid[0], variable_start, variable_count, log_psd);
                        if (status != NC_NOERR) check_netcdf_handle_error(status);
                }
		status = nc_sync(ncid);
                if (status != NC_NOERR) check_netcdf_handle_error(status);
		break;
                }


        }

	free(log_psd);

        obs->PSD_ray_number = obs->PSD_ray_number + 1;
	status = nc_sync(ncid);
	if (status != NC_NOERR) check_netcdf_handle_error(status);
}

void RNC_WriteRapidLogPSDVariables( int ncid, int radar, RSP_ParamStruct *param, RSP_ObservablesStruct *obs, PolPSDStruct PSD[], int PSD_varid[] )
{
        int     variable_count[3];
        int     variable_start[3];
        int     status;
        int     n,j;
        float   timestamp;
	float 	azimuth;
        short int       *log_psd;
	short int	*log_psd_coded;
	int	temp_int = 0;
	int	save_data = 0;
	int	saved_ray = 0;
	int	start_bin = 0;
	int	noise_bin = 0;

        variable_count[0] = 1;
        variable_count[1] = param->npsd;

        /* create a short integer array to allow the results of log10 to be stored */
        log_psd = (short int *)calloc( param->npsd, sizeof(short int));
        if (log_psd == NULL) {
                printf("memory request for log_psd failed\n");
                exit(-1);
        }
	/* create a short integer array to allow the results of log10 to be stored */
        log_psd_coded = (short int *)calloc( param->npsd, sizeof(short int));
        if (log_psd_coded == NULL) {
                printf("memory request for log_psd_coded failed\n");
                exit(-1);
        }


        /* calculate time */
        timestamp =  ((float) obs->hour * 3600.) + ((float) obs->minute * 60.) + ((float) obs->second) + ((float) obs->centisecond/100);

        /* obtain azimuth */
        azimuth = obs->azimuth + param->azimuth_offset;

	/* start point of copernicus fft */
	if ( radar == GALILEO_SPECTRA_RAPID || radar == TEST_SPECTRA_RAPID ) {
		temp_int = 0;
		variable_count[1] = param->npsd;
		start_bin = 6;
		noise_bin = 0;
	} else if ( radar == COPERNICUS_SPECTRA_RAPID ) {
		temp_int = 0;
		/* since I am dumping out the spectra for a range bin */
		/* can I make the */
		variable_count[1] = param->npsd;
		start_bin = 26;
		noise_bin = 469;
	}

        switch(radar)
        {
		case COPERNICUS_SPECTRA_RAPID :
		case GALILEO_SPECTRA_RAPID :
		case TEST_SPECTRA_RAPID :
                {
	                for(n = 0; n < param->samples_per_pulse; n++)
        	        {
				save_data = 0;
				if ( radar == GALILEO_SPECTRA_RAPID || radar == TEST_SPECTRA_RAPID ) {
					for( j = 0; j < param->npsd; j++) {
                                                log_psd[j] = (short int) 1000 * log10( PSD[n].HH[j] );
                                               	if (log_psd[j] > 5000) {
							if ( n >= start_bin ) {
								/* this is an attempt to jump over the central bin */
								if (param->fft_bins_interpolated > 0) {
									if ( (j < (param->npsd/2 - (param->fft_bins_interpolated - 1))) || (j > (param->npsd/2 + (param->fft_bins_interpolated - 1))) ) {
										save_data = 1;
									}
								} else {
                                                 			save_data = 1;
								}
							}
                                               	}
                                        }
                                        if ( (n == noise_bin) || (n == noise_bin + 1) ||
                                                (n == noise_bin + 2) || (n == noise_bin + 3) ) {
                                                save_data = 1;
                                        }
				} else if (radar == COPERNICUS_SPECTRA_RAPID ) {
                                       	for( j = 0; j < param->npsd; j++) {
                                               	log_psd_coded[j] = (short int) 1000 * log10( PSD[n].HHP[j] );
                                               	log_psd[j] = (short int) 1000 * log10( PSD[n].HH[j] );
						if (log_psd_coded[j] > 4700) {
							if ( n >= start_bin ) {
								save_data = 1;
							}
						}
                                        }
					if ( (n == noise_bin) || (n == noise_bin + 1) ||
						(n == noise_bin + 2) || (n == noise_bin + 3) ) {
						save_data = 1;
					}
                                }
				if (save_data == 1) {
					variable_start[0] = obs->bin_ray_number;
        	              		variable_start[1] = 0;

					/* write out variables */
					/* time */
					status = nc_put_var1_float( ncid, obs->tsid, variable_start, &timestamp);
					if (status != NC_NOERR) check_netcdf_handle_error(status);
					/* elevation */
					status = nc_put_var1_float( ncid, obs->elevationid, variable_start, &obs->elevation );
					if (status != NC_NOERR) check_netcdf_handle_error(status);
					/* azimuth */
					status = nc_put_var1_float( ncid, obs->azimuthid, variable_start, &azimuth );
					if (status != NC_NOERR) check_netcdf_handle_error(status);
					/* ray_number */
                        	        status = nc_put_var1_int( ncid, obs->ray_numberid, variable_start, &obs->ray_number );
                                	if (status != NC_NOERR) check_netcdf_handle_error(status);
					/* bin_number */
                               	 	status = nc_put_var1_int( ncid, obs->bin_numberid, variable_start, &n );
                                	if (status != NC_NOERR) check_netcdf_handle_error(status);
        	                	/* write out fft for this ray */
				        if ( radar == GALILEO_SPECTRA_RAPID || radar == TEST_SPECTRA_RAPID ) {
				        	status = nc_put_vara_short( ncid, PSD_varid[0], variable_start, variable_count, &log_psd[temp_int]);
                	        	        if (status != NC_NOERR) check_netcdf_handle_error(status);
				        } else if (radar == COPERNICUS_SPECTRA_RAPID ) {
					        status = nc_put_vara_short( ncid, PSD_varid[0], variable_start, variable_count, &log_psd_coded[temp_int]);
                	        	        if (status != NC_NOERR) check_netcdf_handle_error(status);
					        status = nc_put_vara_short( ncid, PSD_varid[1], variable_start, variable_count, &log_psd[temp_int]);
                	        	        if (status != NC_NOERR) check_netcdf_handle_error(status);
					}
                                        obs->bin_ray_number = obs->bin_ray_number + 1;
					saved_ray = 1;
				}
                	}
			break;
                }
        }

	/* if we have stored some PSD of a ray let us inc. the ray_number */
	if (saved_ray == 1) {
		obs->ray_number = obs->ray_number + 1;
	}
        free(log_psd);
	free(log_psd_coded);

        status = nc_sync(ncid);
        if (status != NC_NOERR) check_netcdf_handle_error(status);
}




void check_netcdf_handle_error(int status)
{
   if (status != NC_NOERR)
   {
      fprintf(stderr, "netCDF ERROR : %s\n", nc_strerror(status));
      exit(-1);
   }
}







