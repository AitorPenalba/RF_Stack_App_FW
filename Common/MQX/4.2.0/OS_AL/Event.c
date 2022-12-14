/******************************************************************************
 *
 * Filename: Event.c
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
#include <stdbool.h>
#include <mqx.h>
#include <lwevent.h>
#include "EVL_event_log.h"

/* #DEFINE DEFINITIONS */

/* MACRO DEFINITIONS */

/* TYPE DEFINITIONS */

/* CONSTANTS */

/* FILE VARIABLE DEFINITIONS */

/* FUNCTION PROTOTYPES */

/* FUNCTION DEFINITIONS */

/*******************************************************************************

  Function name: OS_EVNT_Create

  Purpose: This function will create a new Event object

  Arguments: EventHandle - pointer to the Handle structure of the Event

  Returns: FuncStatus - True if Event created successfully, False if error

  Notes:

*******************************************************************************/
bool OS_EVNT_Create ( OS_EVNT_Handle EventHandle )
{
   uint32_t RetStatus;
   bool FuncStatus = true;

   RetStatus = _lwevent_create ( EventHandle, 0 );
   if ( RetStatus != MQX_OK )
   {
      FuncStatus = false;
   } /* end if() */

   return ( FuncStatus );
} /* end OS_EVNT_Create () */

/*******************************************************************************

  Function name: OS_EVNT_Set

  Purpose: This function is used to set an event that can be waited on by another function

  Arguments: EventHandle - pointer to the Handle structure of the Event
             EventMask - Bit Mask of the Events that should be set

  Returns: None

  Notes: Although MQX can return false for the _lwevent_set function, it should
         never happen since the ewvent should always be valid.
         If this happens, we consider this a catastrophic failure.

         Function will not return if it fails

*******************************************************************************/
void OS_EVNT_SET ( OS_EVNT_Handle EventHandle, uint32_t EventMask, char *file, int line )
{
   if ( _lwevent_set ( EventHandle, EventMask ) ) {
      EVL_FirmwareError( "OS_EVNT_Set" , file, line );
   }
} /* end OS_EVNT_Set () */

/*******************************************************************************

  Function name: OS_EVNT_Wait

  Purpose: This function is used to wait for an Event Mask for the specified time

  Arguments: EventHandle - pointer to the Handle structure of the Event
             EventMask - Bit Mask of the Events to check
             WaitForAll - True if waiting for all EventMask bits to be set, False will wait for any one bit
             Timeout - Time in milliseconds to wait for any Events to be set (Limited to 96 hours)

  Returns: SetMask - Bit Mask of the events that are currently set

  Notes: When Timeout is =0, this function will return immediately.
         When Timeout is =OS_WAIT_FOREVER, this function will wait (forever) until
         the event is set by another process
         The exact number of Milliseconds passed in may not be the exact number
         of Milliseconds that this task will wait for - it is because there may
         not be a direct correlation between milliseconds and how long a tick is
         This will be rounded up to the next largest tick time.

*******************************************************************************/
uint32_t OS_EVNT_WAIT ( OS_EVNT_Handle EventHandle, uint32_t EventMask, bool WaitForAll, uint32_t Timeout_msec, char *file, int line )
{
   uint32_t SetMask = 0;
   uint32_t RetStatus;

   uint32_t timeout_ticks;  // Timeout in ticks

   if ( Timeout_msec > 0 )
   {
      if ( Timeout_msec == OS_WAIT_FOREVER )
      {
         timeout_ticks = 0; /* In MQX, 0 represents a wait forever value */
      } /* end if() */
      else
      {
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
   } /* end if() */
   else
   {
      /* just use the minimum value cause we don't want to wait */
      timeout_ticks = 1;
   } /* end else() */

   RetStatus = _lwevent_wait_ticks ( EventHandle, EventMask, (bool)WaitForAll, timeout_ticks );
   if ( RetStatus == MQX_LWEVENT_INVALID ) {
      EVL_FirmwareError( "OS_EVNT_Wait" , file, line );
   }

   if ( RetStatus == MQX_OK )
   {
      /* Get the events that are currently set (as of the wait command) */
      SetMask = _lwevent_get_signalled();
      /* Clear only the events that were set */
      (void)_lwevent_clear ( EventHandle, SetMask );
   } /* end if() */

   return ( SetMask );
} /* end OS_EVNT_Wait () */

/*******************************************************************************

  Function name: OS_EVNT_CLEAR

  Purpose: This function is used to wait for an Event Mask for the specified time

  Arguments: EventHandle - pointer to the Handle structure of the Event
             EventMask - Bit Mask of the Events to check

  Returns:   None

*******************************************************************************/
void OS_EVNT_CLEAR ( OS_EVNT_Handle EventHandle, uint32_t EventMask, char *file, int line )
{
   ( void )_lwevent_clear(LWEVENT_STRUCT_PTR, _mqx_uint);
}