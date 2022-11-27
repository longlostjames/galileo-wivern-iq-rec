// RSP_FreeMemory.c
// --------------
// Part of the Chilbolton Radar Signal Processing Package
//
// Purpose: To free the memory blocks previously allocated
//          by the RSP_InitialiseParams module.
//
// Created on: 19/01/04
// Created by: E. Pavelin (EGP)
// -------------------------------------------------------

#include "RSP.h"
#include <malloc.h>

void RSP_FreeMemory(RSP_ParamStruct param)
{
  // Free memory blocks
  free(param.range);
  free(param.range_correction);
  free(param.frequency_axis);
  free(param.velocity_axis);
  free(param.window);
}

