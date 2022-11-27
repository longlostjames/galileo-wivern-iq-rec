// Part of the Chilbolton Radar Signal Processing Package
//
// Purpose: To find multiple peaks in the radar power spectrum.
//          
// Created on: 05/11/03
// Created by: E. Pavelin (EGP)
// --------------------------------------------------------
//
// Revision History
// --------------------------------------------------------
// When    : Who : What
// --------------------------------------------------------
// 05-11-03: EGP : Created
// 03-03-04: EGP : left and right bins are now in signal
//         :     : instead of noise.
// 27-09-04: EGP : When PSD doesn't reach noise level, algorithm now
//                 places edges at minimum in spectrum.
//
// Notes
// --------------------------------------------------------

#include <stdio.h>
#include <malloc.h>

#include "RSP.h"

void RSP_FindPeaks(float *psd, int nBins, int nPeaks, float noiseLevel,
                   RSP_PeakStruct *peaks)
{
  float PSDmax=0;
  int bin;

  if( nPeaks > 1 ) {
    printf("* RSP_FindPeaks: Warning: Multiple-peak detection not implemented!\n");
    printf("               : Finding only one peak.\n");
    printf("               : (Use RSP_FindPeaksMulti instead!)\n");
  }

  // Scan PSD to find biggest peak
  for( bin=0; bin<=nBins; bin++ ) {
    if( psd[bin] > PSDmax ) {
      PSDmax = psd[bin];
      peaks[0].peakBin = bin;
      peaks[0].peakPSD = PSDmax;
    }
  }

  RSP_FindEdges(psd,nBins,noiseLevel,&peaks[0]);

}


void RSP_FindPeaksMulti(float *psd, int nBins, int nPeaks, float noiseLevel,
                   RSP_PeakStruct *peaks)
{
  float PSDmax;
  int bin,peak;
  float *PSDcopy;

  PSDcopy=(float *)malloc(nBins*sizeof(float));
  for(bin=0; bin<nBins; bin++) PSDcopy[bin]=psd[bin];

  // Loop through peaks
  for(peak=0; peak<nPeaks; peak++)
    {
      PSDmax=0;
      // Scan PSD to find biggest peak
      for( bin=0; bin<nBins; bin++ ) {
	if( PSDcopy[bin] > PSDmax) {
	  PSDmax = PSDcopy[bin];
	  peaks[peak].peakBin = bin;
	  peaks[peak].peakPSD = PSDmax;
	}
      }      
      RSP_FindEdges(PSDcopy,nBins,noiseLevel,&peaks[peak]);
      for(bin=peaks[peak].leftBin;bin<=peaks[peak].rightBin;bin++)
	PSDcopy[bin%nBins]=noiseLevel;
    }

  free(PSDcopy);
}

void RSP_FindPeaksMulti_Destructive(float *PSD, int nBins, int nPeaks, float noiseLevel,
                   RSP_PeakStruct *peaks)
// Same as FindPeaksMulti, but destroys the input PSD
{
  float PSDmax;
  int bin,peak;

  // Loop through peaks
  for(peak=0; peak<nPeaks; peak++)
    {
      PSDmax=0;
      // Scan PSD to find biggest peak
      for( bin=0; bin<nBins; bin++ ) {
        if( PSD[bin] > PSDmax) {
          PSDmax = PSD[bin];
          peaks[peak].peakBin = bin;
          peaks[peak].peakPSD = PSDmax;
        }
      }
      RSP_FindEdges(PSD,nBins,noiseLevel,&peaks[peak]);
      if(peak<(nPeaks-1)){
        for(bin=peaks[peak].leftBin;bin<=peaks[peak].rightBin;bin++)
          PSD[bin%nBins]=noiseLevel;
        }
    }
}


void RSP_FindEdges(float *psd, int nBins, float noiseLevel, RSP_PeakStruct *peak)
{
  register int k;
  int *leftbin, *rightbin;
  int bin;
  int noiseflag; // Flag to indicate whether noise has been reached
  int minBin;
  float minVal;

  leftbin  = &peak->leftBin;
  rightbin = &peak->rightBin;
  bin = peak->peakBin;
 
  k = 0;
  noiseflag=0;
  minVal=psd[bin];
  minBin=bin;
  do {
    k--;
    *leftbin=bin+k;
    if(*leftbin < 0) *leftbin = nBins + *leftbin; // Deal with folding
    if(psd[*leftbin]<=noiseLevel) noiseflag=1;
    if(psd[*leftbin]<=minVal){  // minBin is used if noise floor not reached
      minBin=*leftbin;
      minVal=psd[minBin];
    }
  } while( noiseflag==0 && (k>(-1*nBins)) );

  if(noiseflag==0){  // If we never reached the noise floor...
    *leftbin=(minBin+1)%nBins;
    *rightbin=minBin;
    return;
  } else {           // If we did reach the noise floor
    *leftbin = (*leftbin+1)%nBins; // Step back up out of noise
  }

  k = 0;
  noiseflag=0;
  do {
    k++;
    *rightbin=(bin+k) % nBins;
    if(psd[*rightbin]<=noiseLevel) noiseflag=1;
  } while( noiseflag==0 && (k<nBins-1) );

  *rightbin = (*rightbin-1); // Step back up out of noise
  if (*rightbin<0) *rightbin=nBins-1;

}







