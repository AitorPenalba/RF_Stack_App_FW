/******************************************************************************
 *
 * Filename: MessageQ.c
 *
 * Global Designator: OS_MSGQ_
 *
 * Contents:
 *
 ******************************************************************************
 * Copyright (c) 2022 ACLARA.  All rights reserved.
 * This program may not be reproduced, in whole or in part, in any form or by
 * any means whatsoever without the written permission of:
 *    ACLARA, ST. LOUIS, MISSOURI USA
 *****************************************************************************/

/* INCLUDE FILES */
#include "project.h"
#include "DBG_SerialDebug.h"
#include "buffer.h"

/* #DEFINE DEFINITIONS */
#define DEFAULT_NUM_ITEMS_MSGQ 10
/* MACRO DEFINITIONS */

/* TYPE DEFINITIONS */

/* CONSTANTS */

/* FILE VARIABLE DEFINITIONS */

/* FUNCTION PROTOTYPES */

/* FUNCTION DEFINITIONS */


/*******************************************************************************

  Function name: OS_MSGQ_Create

  Purpose: This function is used to create a new Message Q Object

  Arguments: MsgqHandle - pointer to the MessageQ object

  Returns: RetStatus - True if MessageQ created successfully, False if error

  Notes: Calling function must pass in an allocated OS_MSGQ_Obj

*******************************************************************************/

#if ( ( BM_USE_KERNEL_AWARE_DEBUGGING == 1 ) && ( RTOS_SELECTION == FREE_RTOS ) )
bool OS_MSGQ_CREATE ( OS_MSGQ_Handle MsgqHandle,  uint32_t NumMessages, const char *name )
#else
bool OS_MSGQ_CREATE ( OS_MSGQ_Handle MsgqHandle,  uint32_t NumMessages )
#endif
{
   bool RetStatus = (bool)true;

   if ( false == OS_QUEUE_Create(&(MsgqHandle->MSGQ_QueueObj), NumMessages, name) )
   {
      RetStatus = (bool)false;
   } /* end if() */
   if ( false == OS_SEM_Create(&(MsgqHandle->MSGQ_SemObj), NumMessages ) )
   {
      RetStatus = (bool)false;
   } /* end if() */

   return ( RetStatus );
} /* end OS_MSGQ_Create () */
/*******************************************************************************

  Function name: OS_MSGQ_POST

  Purpose: This function is used to Post a message into a MessageQ structure

  Arguments: MsgqHandle - pointer to the MessageQ object
             MessageData - pointer to the pointer of the Message data to post (see Notes below)
             ErrorCheck - flag to check some errors or not. This is need when BM post a free buffer.

  Returns: None

  Notes: The memory for MessageData must be allocated by the calling function and
         and is retained in the original allocated memory location.
         Allocation and De-allocation must be handled by the application calling
         these OS_MSGQ_xxx functions This OS_MSGQ module does not allocate,
         nor free any memory, and just deals
         with a pointer to the memory location

         Function will not return if it fails

*******************************************************************************/
void OS_MSGQ_POST ( OS_MSGQ_Handle MsgqHandle, void *MessageData, bool ErrorCheck, char *file, int line )
{
  OS_QUEUE_Element *ptr = MessageData;

  // Sanity check
  if (ErrorCheck && ptr->flag.isFree) {
    // The buffer was freed
    DBG_LW_printf("\nERROR: OS_MSGQ_POST got a buffer marked as free. Size: %u, pool = %u, addr=0x%p\n",
                  ptr->dataLen, ptr->bufPool, MessageData);
    DBG_LW_printf("ERROR: OS_MSGQ_POST called from %s:%d\n", file, line);
  }
  if (ptr->flag.inQueue) {
    // The buffer is already in use.
    DBG_LW_printf("\nERROR: OS_MSGQ_POST got a buffer marked as in used by another queue. Size: %u, pool = %u, addr=0x%p\n",
                  ptr->dataLen, ptr->bufPool, MessageData);
    DBG_LW_printf("ERROR: OS_MSGQ_POST called from %s:%d\n", file, line);
  }

  // Mark as on queue
  ptr->flag.inQueue++;

  OS_QUEUE_ENQUEUE(&(MsgqHandle->MSGQ_QueueObj), MessageData, file, line); // Function will not return if it fails

  OS_SEM_POST(&(MsgqHandle->MSGQ_SemObj), file, line); // Function will not return if it fails


} /* end OS_MSGQ_Post () */

/*******************************************************************************

  Function name: OS_MSGQ_POST_RetStatus

  Purpose: This function is used to Post a message into a MessageQ structure giving the caller
           the opportunity to ignore failures due to a full queue

  Arguments: MsgqHandle - pointer to the MessageQ object
             MessageData - pointer to the pointer of the Message data to post (see Notes below)
             ErrorCheck - flag to check some errors or not. This is need when BM post a free buffer.

  Returns: eSUCCESS if the element was successfully queued
           eFAILURE if the element is either free or already in a queue
           eOS_QUE_FULL_ERR if the queue is full

  Notes: See notes for OS_MSGQ_POST

         Function will not return if it fails

*******************************************************************************/
returnStatus_t OS_MSGQ_POST_RetStatus ( OS_MSGQ_Handle MsgqHandle, void *MessageData, bool ErrorCheck, char *file, int line )
{
   OS_QUEUE_Element *ptr = MessageData;
   returnStatus_t eRetVal = eFAILURE;
   // Sanity check
   if (ErrorCheck && ptr->flag.isFree) {
      // The buffer was freed
      return ( eRetVal );
   }
   if (ptr->flag.inQueue) {
      // The buffer is already in use.
      return ( eRetVal );
   }

   // Mark as on queue
   ptr->flag.inQueue++;

   returnStatus_t err = OS_QUEUE_ENQUEUE_RetStatus( &(MsgqHandle->MSGQ_QueueObj), MessageData, file, line );
   if ( eSUCCESS == err )
   {
      eRetVal = OS_SEM_POST_RetStatus ( &(MsgqHandle->MSGQ_SemObj), file, line );
   }
   else
   {
      eRetVal = err;
   }

   return ( eRetVal );

} /* end OS_MSGQ_POST_RetStatus () */

/*******************************************************************************

  Function name: OS_MSGQ_PEND

  Purpose: This function is used to Pend on a message from a MessageQ structure

  Arguments: MsgqHandle - pointer to the MessageQ object
             MessageData - pointer to the location of a pointer where the data resides
             TimeoutMs - Timeout in Milliseconds to wait for a message.  0 = poll to see if a message is present
             ErrorCheck - flag to check some errors or not. This is needed when BM pends for a buffer.

  Returns: RetStatus - True if MessageQ Pended Message successfully, False if error, or Timeout

  Notes: The memory for MessageData has been allocated by an application module outside
         of this OS_MSGQ_xxx functions.  The application is also responsible for
         de-allocating this memory after it has been consumed.
         This OS_MSGQ module does not allocate, nor free any memory, and just deals
         with a pointer to the memory location

*******************************************************************************/
bool OS_MSGQ_PEND ( OS_MSGQ_Handle MsgqHandle, void **MessageData, uint32_t TimeoutMs, bool ErrorCheck, char *file, int line )
{
   bool RetStatus = true;
   OS_QUEUE_Element_Handle ptr;

   if ( true == OS_SEM_Pend(&(MsgqHandle->MSGQ_SemObj), TimeoutMs) )
   {
      ptr = OS_QUEUE_Dequeue(&(MsgqHandle->MSGQ_QueueObj));
      *MessageData = ptr;

      // Sanity check
      if (ErrorCheck && ptr->flag.isFree) {
         // The buffer was freed
         DBG_LW_printf("\nERROR: OS_MSGQ_PEND got a buffer marked as free. Size: %u, pool = %u, addr=0x%p\n",
                       ptr->dataLen, ptr->bufPool, *MessageData);
         DBG_LW_printf("ERROR: OS_MSGQ_PEND called from %s:%d\n", file, line);
      }
      if (ptr->flag.inQueue == 0 ) {
         // The buffer marked as not in queue.
         DBG_LW_printf("\nERROR: OS_MSGQ_PEND got a buffer marked as not on queue. Size: %u, pool = %u, addr=0x%p\n",
                       ptr->dataLen, ptr->bufPool, *MessageData);
         DBG_LW_printf("ERROR: OS_MSGQ_PEND called from %s:%d\n", file, line);
      } else {
         if (ptr->flag.inQueue > 1 ) {
            // The buffer is on more than one queue.
            DBG_LW_printf("\nERROR: OS_MSGQ_PEND got a buffer marked as in used by another queue. Size: %u, pool = %u, addr=0x%p\n",
                          ptr->dataLen, ptr->bufPool, *MessageData);
            DBG_LW_printf("ERROR: OS_MSGQ_PEND called from %s:%d\n", file, line);
         }
         // Mark as dequeued
         ptr->flag.inQueue--;
      }
   } /* end if() */
   else
   {
      RetStatus = false;
   } /* end else() */

   return ( RetStatus );
} /* end OS_MSGQ_Pend () */

