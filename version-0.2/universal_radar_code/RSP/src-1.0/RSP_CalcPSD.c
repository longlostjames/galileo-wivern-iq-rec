// RSP_CalcPSD.c
// --------------
// Part of the Chilbolton Radar Signal Processing Package
//
// Purpose: To calculate the power spectral density (PSD) from
//          a complex video time series (I and Q).
//
// NB: RSP_CalcPSD acts destructively on the input data IQ.
//
// Created on: 03/11/03
// Created by: E. Pavelin (EGP)
// --------------------------------------------------------

#include "RSP.h"
#include <stdio.h>
#include <math.h>
#include <malloc.h>

void RSP_CalcPSD(RSP_ComplexType *IQ, int nfft, float *window, float *psd, float norm)
{
  int i;

  // Multiply by window
  for(i=0; i<nfft; i++) {
    IQ[i].real = IQ[i].real * window[i];
    IQ[i].imag = IQ[i].imag * window[i];
  }

  // Do FFT
  RSP_FFT((float *)IQ,nfft,1);

  // Calculate PSD
  RSP_FFT2PowerSpec((float *)IQ,psd,nfft,norm);
}

void RSP_SubtractOffset(RSP_ComplexType *IQ, int nfft)
{
  int i;
  float Imean=0,Qmean=0;

  // Calculate DC offsets
  for(i=0; i<nfft; i++) {
    Imean += IQ[i].real;
    Qmean += IQ[i].imag;
  }
  Imean = Imean / nfft;
  Qmean = Qmean / nfft;

  // Subtract offsets and multiply by window
  for(i=0; i<nfft; i++) {
    IQ[i].real = (IQ[i].real - Imean);
    IQ[i].imag = (IQ[i].imag - Qmean);
  }
}

//-------------------------------------------------------------------
/* FFT routine adapted from Numerical Recipes in C */
#define SWAP(a,b) tempr=(a);(a)=(b);(b)=tempr
void RSP_FFT(float data_in[], unsigned long nn, int isign)
{
    unsigned long n,mmax,m,j,istep,i;
    double wtemp,wr,wpr,wpi,wi,theta;
    float tempr,tempi;
    float *data;

    data=data_in-1; // Shift to zero-based array

    n=nn << 1;
    j=1;
    for (i=1;i<n;i+=2) {
        if (j > i) {
            SWAP(data[j],data[i]);
            SWAP(data[j+1],data[i+1]);
        }
        m=n >> 1;
        while (m >= 2 && j > m) {
            j -= m;
            m >>= 1;
        }
        j += m;
    }
    mmax=2;
    while (n > mmax) {
        istep=mmax << 1;
        theta=isign*(6.28318530717959/mmax);
        wtemp=sin(0.5*theta);
        wpr = -2.0*wtemp*wtemp;
        wpi=sin(theta);
        wr=1.0;
        wi=0.0;
        for (m=1;m<mmax;m+=2) {
            for (i=m;i<=n;i+=istep) {
                j=i+mmax;
                tempr=wr*data[j]-wi*data[j+1];
                tempi=wr*data[j+1]+wi*data[j];
                data[j]=data[i]-tempr;
                data[j+1]=data[i+1]-tempi;
                data[i] += tempr;
                data[i+1] += tempi;
            }
            wr=(wtemp=wr)*wpr-wi*wpi+wr;
            wi=wi*wpr+wtemp*wpi+wi;
        }
        mmax=istep;
    }
}
#undef SWAP

//----------------------------------------------------------------------
void RSP_FFT2PowerSpec(float *data, float *PSD, int nfft, float norm) 
/* Calculates power spectrum from complex FFT output */
{
    int i,j,nn,i2;
    float *PP;

    PP=(float *)malloc(sizeof(float)*nfft);

    for(i=0;i<nfft;i++){
        i2=i*2;
        PP[i]=data[i2]*data[i2] + data[i2+1]*data[i2+1];
    }

    j=0;
    //nn=nfft/2;   // could replace this with a bit shift?
    nn=nfft>>1;
    for(i=nn; i<nfft; i++){
        PSD[j]=PP[i] * norm;
        j++;
    }
    for(i=0; i<=nn; i++){
        PSD[j]=PP[i] * norm;
        j++;
    }
    free(PP);
}

