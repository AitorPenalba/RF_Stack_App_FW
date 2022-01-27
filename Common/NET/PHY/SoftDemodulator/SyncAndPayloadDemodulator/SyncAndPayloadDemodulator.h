/******************************************************************************

   Filename: SyncAndPayloadDemodulator.h

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

#ifndef SYNCANDPAYLOADDEMODULATOR_H
#define SYNCANDPAYLOADDEMODULATOR_H

/* Include files */
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include "rtwtypes.h"
#include "SyncAndPayloadDemodulator_types.h"

/* Type Definitions */
#include "SoftDemodulator.h"

/* Function Declarations */
extern void SyncAndPayloadDemodulator(c_SyncAndPayloadDemodulatorPers *Pers,
                                      const float filteredInput[150],
                                      eSyncAndPayloadDemodFailCodes_t *FailCode);

#endif