// RSP_CalcSpecMom.c
// -----------------
// Part of the Chilbolton Radar Signal Processing Package
//
// Purpose: To estimate spectral moments from a radar power
//          sepctrum.
//
// Created on: 05/11/03
// Created by: E. Pavelin (EGP)
// --------------------------------------------------------
//
// Revision History
// --------------------------------------------------------
// When     : Who : What
// --------------------------------------------------------
// 05-11-03 : EGP : Created
// 03-03-04 : EGP : Algorithm streamlined

#include <stdio.h>
#include "RSP.h"
#include <math.h>

void RSP_CalcSpecMom(float *psd, int nBins, RSP_PeakStruct peak, float noiseLevel, float *moments)
{

  register int k,j;
  int b2;
  float pxx;

  if(peak.leftBin > peak.rightBin) { // Folded spectrum
    b2=peak.rightBin+nBins;
  }
  else b2=peak.rightBin;

  moments[0]=0;
  moments[1]=0;
  moments[2]=0;

  for(k=peak.leftBin; k<=b2; k++)
    {
      j = k % nBins;
      pxx = psd[j]-noiseLevel;
      moments[0] += pxx;
      moments[1] += k*pxx;
      moments[2] += k*k*pxx;
    }

  moments[1] = moments[1]/moments[0];
  moments[2] = sqrt(moments[2]/moments[0] - moments[1]*moments[1]);

  // Deal with case of folding from -ve to +ve frequency
  if(peak.leftBin>peak.rightBin && peak.peakBin<peak.leftBin)
    moments[1] = moments[1] - nBins;
}


//----------------------------------------------------------
// Converts bin number (floating point) to Doppler velocity
//----------------------------------------------------------
float RSP_BinToVelocity(float bin,RSP_ParamStruct param)
{
  float mid;

  mid=param.nfft/2-1;
  return(param.frequency_bin_width*(bin-mid)/param.hz_per_mps);
}


// Calculate total noise power under a peak
float RSP_CalcNoisePower(float noiseLevel, RSP_PeakStruct peak, RSP_ParamStruct param)
{
  float noise_power;

  noise_power=(peak.rightBin-peak.leftBin+1);
  if(peak.leftBin>peak.rightBin) // If folded
    noise_power=noise_power+param.npsd;
  noise_power=noise_power*param.frequency_bin_width*noiseLevel;

  return noise_power;
}
