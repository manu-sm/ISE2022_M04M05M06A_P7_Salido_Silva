/*------------------------------------------------------------------------------
 * MDK Middleware - Component ::Network:Service
 * Copyright (c) 2004-2019 Arm Limited (or its affiliates). All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    Net_Config_SNTP_Client.h
 * Purpose: Network Configuration for SNTP Client
 * Rev.:    V5.0.0
 *----------------------------------------------------------------------------*/

//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------

// <h>SNTP Client
#define SNTP_CLIENT_ENABLE       1

//   <q>Broadcast Mode
//   =================
//   <i>Enable this option, if you have NTP/SNTP server
//   <i>on LAN, which is broadcasting NTP time messages.
//   <i>Disable this option to access public NTP server.
//   <i>Default: disabled
#define SNTP_CLIENT_BCAST_MODE   0

//   <h>NTP Server
//   <i>NTP Server IP Address
//     <o>Address byte 1 <0-255>
//     <i>Default: 217
#define SNTP_CLIENT_SERVER_IP1   130

//     <o>Address byte 2 <0-255>
//     <i>Default: 79
#define SNTP_CLIENT_SERVER_IP2   206

//     <o>Address byte 3 <0-255>
//     <i>Default: 179
#define SNTP_CLIENT_SERVER_IP3   0

//     <o>Address byte 4 <0-255>
//     <i>Default: 106
#define SNTP_CLIENT_SERVER_IP4   1
//   </h>

// </h>
