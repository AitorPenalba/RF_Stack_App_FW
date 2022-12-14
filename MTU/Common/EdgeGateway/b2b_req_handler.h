/** ****************************************************************************
@file b2b_req_handler.h
 
API for the Board to Board handling code.
 
A product of Aclara Technologies LLC
Confidential and Proprietary
Copyright 2018 Aclara.  All Rights Reserved.
*******************************************************************************/

#ifndef B2B_REQ_HANDLER_H
#define B2B_REQ_HANDLER_H

/*******************************************************************************
#INCLUDES
*******************************************************************************/

#include <stdint.h>
#include "b2b.h"
#include "hdlc_frame.h"

/*******************************************************************************
Public #defines (Object-like macros)
*******************************************************************************/

/*******************************************************************************
Public #defines (Function-like macros)
*******************************************************************************/

/*******************************************************************************
Public Struct, Typedef, and Enum Definitions
*******************************************************************************/

/*******************************************************************************
Global Variable Extern Statements
*******************************************************************************/

/*******************************************************************************
Public Function Prototypes
*******************************************************************************/

void HandleB2BRequest(B2BPacketHeader_t header, const uint8_t *params);
void HandleNonB2BRequest(hdlc_addr_t addr, const uint8_t *data, uint16_t len);

#endif
