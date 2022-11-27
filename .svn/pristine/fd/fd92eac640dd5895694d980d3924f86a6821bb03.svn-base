// RSP_CalcPhase.c
// --------------
// Part of the Chilbolton Radar Signal Processing Package
//
// Purpose: To calculate phase parameters from I and Q time
//          series.
//
// Created on: 12/02/04
// Created by: E. Pavelin (EGP)
// --------------------------------------------------------

#include "RSP.h"
#include <stdio.h>
#include <math.h>


// Calculates phase and std. dev. of phase in degrees

void RSP_CalcPhase(RSP_ComplexType *IQ, float *phi, float *sdphi, int nfft)
{
  int j;
  float sum_I=0,sum_Q=0;
  float phase,diff;

  // Calculate mean phase
  for(j=0;j<nfft;j++){
    sum_I += IQ[j].real;
    sum_Q += IQ[j].imag;
  }

  *phi = atan2(sum_I,sum_Q);
  
  // CALC STD DEVIATION
  *sdphi = 0;
  for(j=0;j<nfft;j++){
    phase = atan2( IQ[j].real, IQ[j].imag );
    diff = phase - *phi;
    if( diff < (-1.0*PI) ) diff += 2.0*PI;
    else if( diff > PI )   diff -= 2.0*PI;
    *sdphi += (diff * diff);
  }

  *sdphi = sqrt( *sdphi / (float)nfft ) * RAD2DEG;
  *phi   = *phi * RAD2DEG;
}


