// RSP_Initialise.c
// --------------
// Part of the Chilbolton Radar Signal Processing Package
//
// Purpose: To initialise operating parameters and allocate
//          memory required by all modules.
//
// Created on: 17/11/03
// Created by: E. Pavelin (EGP)
// -------------------------------------------------------

#include "RSP.h"
#include <malloc.h>
#include <math.h>

void RSP_InitialiseParams(RSP_ParamStruct *param)
{
  int i;
  int tot_avg;

  tot_avg = param->pulses_coherently_averaged * param->number_of_codes;

  // Calculate radar operating parameters
  // See RSP.h for explanations
  param->prt = 1/param->prf;
  param->sample_frequency = param->clock / param->clock_divfactor;
  param->sample_period = 1.0 / param->sample_frequency;
  param->daq_time = param->pulses_per_daq_cycle * param->prt;
  param->dwell_time = param->daq_time * param->spectra_averaged;
  param->range_gate_width = param->sample_period * SPEED_LIGHT / 2;
  param->range_resolution = param->pulse_period * 1e-9 * SPEED_LIGHT / 2;
  param->nfft = param->pulses_per_daq_cycle / tot_avg;
  param->npsd = param->nfft + 1;
  param->frequency_bin_width = 1.0 / ((float)param->nfft * param->prt * (float)tot_avg);
  param->hz_per_mps = 2 * param->frequency*1e9 / SPEED_LIGHT;
  param->folding_frequency = param->prf / tot_avg / 2;
  param->folding_velocity = param->folding_frequency / param->hz_per_mps;
  param->oversample_ratio = (int)(param->pulse_period * 1e-9 * param->sample_frequency);

  // Allocate memory blocks
  param->range = (float *)malloc(param->samples_per_pulse * sizeof(float));
  param->range_correction = (float *)malloc(param->samples_per_pulse * sizeof(float));
  param->frequency_axis = (float *)malloc(param->npsd * sizeof(float));
  param->velocity_axis = (float *)malloc(param->npsd * sizeof(float));
  param->window = (float *)malloc(param->nfft * sizeof(float));

  // Fill array variables
  for(i=0; i<param->npsd; i++) {
    param->frequency_axis[i] = param->frequency_bin_width * (-1*(param->nfft/2+1)+i);
    param->velocity_axis[i] = param->frequency_axis[i] / param->hz_per_mps;
  }
  
  for(i=0; i<param->samples_per_pulse; i++) {
    param->range[i] = i * param->range_gate_width + param->range_offset;
    param->range_correction[i]=(param->range[i]/1000)*(param->range[i]/1000);
  }

  param->Wss=0;
  for(i=0; i<param->nfft; i++) {
    param->window[i] = BLACKMAN_WINDOW(i,param->nfft);
    param->Wss += param->window[i]*param->window[i];
  }
  param->Wss = param->Wss / param->nfft;

}

