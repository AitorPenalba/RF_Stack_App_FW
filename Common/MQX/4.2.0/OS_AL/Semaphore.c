/******************************************************************************
 *
 * Filename: Semaphore.c
 *
 * Global Designator:
 *
 * Contents:
 *
 ******************************************************************************
 * Copyright (c) 2012 ACLARA.  All rights reserved.
 * This program may not be reproduced, in whole or in part, in any form or by
 * any means whatsoever without the written permission of:
 *    ACLARA, ST. LOUIS, MISSOURI USA
 *****************************************************************************/

/* INCLUDE FILES */
#include "project.h"
#include <stdint.h>
#include <stdbool.h>
#include <mqx.h>
#include "OS_aclara.h"
#include "EVL_event_log.h"

/* #DEFINE DEFINITIONS */

/* MACRO DEFINITIONS */

/* TYPE DEFINITIONS */

/* CONSTANTS */

/* FILE VARIABLE DEFINITIONS */

/* FUNCTION PROTOTYPES */

/* FUNCTION DEFINITIONS */

/*******************************************************************************

  Function name: OS_SEM_Create

  Purpose: This function will create a new Semaphore

  Arguments: SemHandle - pointer to the Handle structure of the Semaphore
              maxCount - not used

  Returns: FuncStatus - True if Semaphore created successfully, False if error

  Notes:

*******************************************************************************/
bool OS_SEM_Create ( OS_SEM_Handle SemHandle, uint32_t maxCount)
{
   uint32_t RetStatus;
   bool FuncStatus = true;
   (void) maxCount; //Paramter not used

   RetStatus = _lwsem_create ( SemHandle, 0 ); /* Always create with initial count of 0 */
   if ( RetStatus != MQX_OK )
   {
      FuncStatus = false;
   } /* end if() */

   return ( FuncStatus );
} /* end OS_SEM_Create () */

/*******************************************************************************

  Function name: OS_SEM_Post

  Purpose: This function will post to the passed in Semaphore

  Arguments: SemHandle - pointer to the Handle structure of the Semaphore

  Returns: None

  Notes: Although MQX can return false for the _lwsem_post function, it should
         never happen since the semepaphore should always be valid.
         If this happens, we consider this a catastrophic failure.

         Function will not return if it fails

*******************************************************************************/
void OS_SEM_POST ( OS_SEM_Handle SemHandle, char *file, int line )
{
  if ( _lwsem_post ( SemHandle ) ) {
      EVL_FirmwareError( "OS_SEM_Post" , file, line );
   }
} /* end OS_SEM_Post () */

/*******************************************************************************

  Function name: OS_SEM_Pend

  Purpose: This function will wait for the passed in Semaphore until it has been
           posted by another process (Semaphore count must be greater than 0)

  Arguments: SemHandle - pointer to the Handle structure of the Semaphore
             TimeoutMs - time in Milliseconds that this function should wait for
                         the semaphore before timing out and returning to the calling process.

  Returns: FuncStatus - True if Semaphore pended successfully, False if error or if timed out.

  Notes: When TimeoutMs is =0, this function will return immediately.
         When TimeoutMs is =OS_WAIT_FOREVER, this function will wait (forever) until
         the semaphore is posted by another process
         The exact number of Milliseconds passed in may not be the exact number
         of Milliseconds that this task will wait for - it is because there may
         not be a direct correlation between milliseconds and how long a tick is
         This will be rounded up to the next largest tick time.

*******************************************************************************/
bool OS_SEM_PEND ( OS_SEM_Handle SemHandle, uint32_t Timeout_msec, char *file, int line )
{
   uint32_t RetStatus;
   bool FuncStatus = true;

   uint32_t timeout_ticks;  // Timeout in ticks

   if ( Timeout_msec > 0 )
   {
      if ( Timeout_msec == OS_WAIT_FOREVER )
      {
         timeout_ticks = 0; /* In MQX, 0 represents a wait forever value */
      } /* end if() */
      else
      {
         /* Convert the passed in Timeout from Milliseconds into Ticks */
         /* This is limited to a maximum of 96 hours, to prevent an overflow!
          * This is OK for 200 ticks per second.
          */
         if(Timeout_msec > (ONE_MIN * 60 * 24 * 4))
         {
            Timeout_msec = (ONE_MIN * 60 * 24 * 4);
         }

         /* Convert the Timeout from Milliseconds into Ticks */
         timeout_ticks = (uint32_t)  ((uint64_t) ((uint64_t) Timeout_msec * (uint64_t) _time_get_ticks_per_sec()) / 1000);
         if( (uint32_t) ((uint64_t) ((uint64_t) Timeout_msec * (uint64_t) _time_get_ticks_per_sec()) % 1000) > 0)
         {   /* Round the value up to ensure the time is >= the time requested */
            timeout_ticks = timeout_ticks + 1;
         } /* end if() */
      } /* end else() */

      RetStatus = _lwsem_wait_ticks ( SemHandle, timeout_ticks  );

      if ( RetStatus == MQX_INVALID_LWSEM ) {
         EVL_FirmwareError( "OS_SEM_Pend" , file, line );
      }

      if ( RetStatus != MQX_OK )
      {
         FuncStatus = false;
      } /* end if() */
   } /* end if() */
   else
   {
      RetStatus = _lwsem_poll ( SemHandle );
      if ( RetStatus == false )
      {
         FuncStatus = false;
      } /* end if() */
   } /* end else() */

   return ( FuncStatus );
} /* end OS_SEM_Pend () */

/*******************************************************************************

  Function name: OS_SEM_Reset

  Purpose: Reset the semaphore count back to 0

  Arguments: SemHandle - pointer to the Handle structure of the Semaphore

  Returns:

  Notes: This basically performs a Pend opperation until all of them have been
         consumed

*******************************************************************************/
void OS_SEM_Reset ( OS_SEM_Handle SemHandle )
{
   uint32_t RetStatus;

   do
   {
      RetStatus = _lwsem_poll ( SemHandle );
   } while ( RetStatus != false );
} /* end OS_SEM_Reset () */

