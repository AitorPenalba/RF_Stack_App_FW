/******************************************************************************

   Filename: processDerotatedSamples.h

   Global Designator:

   Contents:

 ******************************************************************************
   A product of
   Aclara Technologies LLC
   Confidential and Proprietary
   Copyright 2012-2019 Aclara.  All Rights Reserved.

   PROPRIETARY NOTICE
   The information contained in this document is private to Aclara Technologies LLC an Ohio limited liability company
   (Aclara).  This information may not be published, reproduced, or otherwise disseminated without the express written
   authorization of Aclara.  Any software or firmware described in this document is furnished under a license and may
   be used or copied only in accordance with the terms of such license.
 *****************************************************************************/

#ifndef PROCESSDEROTATEDSAMPLES_H
#define PROCESSDEROTATEDSAMPLES_H

/* Include files */
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include "rtwtypes.h"

/* Function Declarations */
uint32_t processDerotatedSamples(const float newSamples_data[],
                                 uint32_t startingIndex,
                                 uint32_t samplesInBuffer,
                                 uint32_t LoP, //Length_of_Preamble
                                 float    b_SlicIntg2[200],
                                 uint32_t *c_SlicOutIdx,
                                 float    b_samplesUnProcessedBuffer[13],
                                 uint32_t *samplesUnProcessed,
                                 int      *c_ZCerrAcc);

#endif