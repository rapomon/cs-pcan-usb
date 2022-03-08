/* PCAN-Basic helper functions

   Collection of functions that print TPCAN structures to stdout, look up 
   string representations of constants, and decode DLC values.

   Copyright (c) 2020 Control Solutions LLC. All rights reserved.
*/

#if defined _WIN32
#include <windows.h>     // provide Win32 types, constants, and functions
#include <PCANBasic.h>   // provide PCAN-Basic types, constants, and functions
#elif defined __APPLE__
#include <PCBUSB.h>      // provide PCAN-Basic types, constants, and functions
#endif

#include <stdint.h>        // provide uintX_t
#include <stdio.h>         // provide printf

#include "common_helper.h" // provide lookupString
#include "pcan_helper.h"
#if defined _WIN32
#include "pcan_event_win32.h"    // provide threadParameters_t
#endif


// ----------------------------------- // -----------------------------------
// Definitions




// ----------------------------------- // -----------------------------------
// Global variables




// ----------------------------------- // -----------------------------------
// Local variables


// String lookup table for PCAN statuses
const lookup_t pcanStatuses[] = {
    { PCAN_ERROR_OK,           "PCAN_ERROR_OK" },
    { PCAN_ERROR_XMTFULL,      "PCAN_ERROR_XMTFULL" },
    { PCAN_ERROR_OVERRUN,      "PCAN_ERROR_OVERRUN" },
    { PCAN_ERROR_BUSLIGHT,     "PCAN_ERROR_BUSLIGHT" },
    { PCAN_ERROR_BUSHEAVY,     "PCAN_ERROR_BUSHEAVY" },
    { PCAN_ERROR_BUSWARNING,   "PCAN_ERROR_BUSWARNING" },
    { PCAN_ERROR_BUSPASSIVE,   "PCAN_ERROR_BUSPASSIVE" },
    { PCAN_ERROR_BUSOFF,       "PCAN_ERROR_BUSOFF" },
    { PCAN_ERROR_ANYBUSERR,    "PCAN_ERROR_ANYBUSERR" },
    { PCAN_ERROR_QRCVEMPTY,    "PCAN_ERROR_QRCVEMPTY" },
    { PCAN_ERROR_QOVERRUN,     "PCAN_ERROR_QOVERRUN" },
    { PCAN_ERROR_QXMTFULL,     "PCAN_ERROR_QXMTFULL" },
    { PCAN_ERROR_REGTEST,      "PCAN_ERROR_REGTEST" },
    { PCAN_ERROR_NODRIVER,     "PCAN_ERROR_NODRIVER" },
    { PCAN_ERROR_HWINUSE,      "PCAN_ERROR_HWINUSE" },
    { PCAN_ERROR_NETINUSE,     "PCAN_ERROR_NETINUSE" },
    { PCAN_ERROR_ILLHW,        "PCAN_ERROR_ILLHW" },
    { PCAN_ERROR_ILLNET,       "PCAN_ERROR_ILLNET" },
    { PCAN_ERROR_ILLCLIENT,    "PCAN_ERROR_ILLCLIENT" },
    { PCAN_ERROR_ILLHANDLE,    "PCAN_ERROR_ILLHANDLE" },
    { PCAN_ERROR_RESOURCE,     "PCAN_ERROR_RESOURCE" },
    { PCAN_ERROR_ILLPARAMTYPE, "PCAN_ERROR_ILLPARAMTYPE" },
    { PCAN_ERROR_ILLPARAMVAL,  "PCAN_ERROR_ILLPARAMVAL" },
    { PCAN_ERROR_UNKNOWN,      "PCAN_ERROR_UNKNOWN" },
    { PCAN_ERROR_ILLDATA,      "PCAN_ERROR_ILLDATA" },
    { PCAN_ERROR_ILLMODE,      "PCAN_ERROR_ILLMODE" },
    { PCAN_ERROR_CAUTION,      "PCAN_ERROR_CAUTION" },
    { PCAN_ERROR_INITIALIZE,   "PCAN_ERROR_INITIALIZE" },
    { PCAN_ERROR_ILLOPERATION, "PCAN_ERROR_ILLOPERATION" },
    { 0, 0 }
};


// Default string for unknown statuses
const char pcanStatusUnknown[] = "Status unknown";


// String lookup table for message types
const lookup_t pcanMsgtypes [] = {
    { PCAN_MESSAGE_STANDARD, "PCAN_MESSAGE_STANDARD" },
    { PCAN_MESSAGE_RTR,      "PCAN_MESSAGE_RTR" },
    { PCAN_MESSAGE_EXTENDED, "PCAN_MESSAGE_EXTENDED" },
    { PCAN_MESSAGE_FD,       "PCAN_MESSAGE_FD" },
    { PCAN_MESSAGE_BRS,      "PCAN_MESSAGE_BRS" },
    { PCAN_MESSAGE_ESI,      "PCAN_MESSAGE_ESI" },
    { PCAN_MESSAGE_ERRFRAME, "PCAN_MESSAGE_ERRFRAME" },
    { PCAN_MESSAGE_STATUS,   "PCAN_MESSAGE_STATUS" },
    { 0, 0 }
};


// Default string for unknown message types
const char pcanMsgtypeUnknown[] = "Message type unknown";


// String lookup table for parameters
const lookup_t pcanParameters [] = {
    { PCAN_DEVICE_ID,                "PCAN_DEVICE_ID" },
    { PCAN_5VOLTS_POWER,             "PCAN_5VOLTS_POWER" },
    { PCAN_RECEIVE_EVENT,            "PCAN_RECEIVE_EVENT" },
    { PCAN_MESSAGE_FILTER,           "PCAN_MESSAGE_FILTER" },
    { PCAN_API_VERSION,              "PCAN_API_VERSION" },
    { PCAN_CHANNEL_VERSION,          "PCAN_CHANNEL_VERSION" },
    { PCAN_BUSOFF_AUTORESET,         "PCAN_BUSOFF_AUTORESET" },
    { PCAN_LISTEN_ONLY,              "PCAN_LISTEN_ONLY" },
    { PCAN_LOG_LOCATION,             "PCAN_LOG_LOCATION" },
    { PCAN_LOG_STATUS,               "PCAN_LOG_STATUS" },
    { PCAN_LOG_CONFIGURE,            "PCAN_LOG_CONFIGURE" },
    { PCAN_LOG_TEXT,                 "PCAN_LOG_TEXT" },
    { PCAN_CHANNEL_CONDITION,        "PCAN_CHANNEL_CONDITION" },
    { PCAN_HARDWARE_NAME,            "PCAN_HARDWARE_NAME" },
    { PCAN_RECEIVE_STATUS,           "PCAN_RECEIVE_STATUS" },
    { PCAN_CONTROLLER_NUMBER,        "PCAN_CONTROLLER_NUMBER" },
    { PCAN_TRACE_LOCATION,           "PCAN_TRACE_LOCATION" },
    { PCAN_TRACE_STATUS,             "PCAN_TRACE_STATUS" },
    { PCAN_TRACE_SIZE,               "PCAN_TRACE_SIZE" },
    { PCAN_TRACE_CONFIGURE,          "PCAN_TRACE_CONFIGURE" },
    { PCAN_CHANNEL_IDENTIFYING,      "PCAN_CHANNEL_IDENTIFYING" },
    { PCAN_CHANNEL_FEATURES,         "PCAN_CHANNEL_FEATURES" },
    { PCAN_BITRATE_ADAPTING,         "PCAN_BITRATE_ADAPTING" },
    { PCAN_BITRATE_INFO,             "PCAN_BITRATE_INFO" },
    { PCAN_BITRATE_INFO_FD,          "PCAN_BITRATE_INFO_FD" },
    { PCAN_BUSSPEED_NOMINAL,         "PCAN_BUSSPEED_NOMINAL" },
    { PCAN_BUSSPEED_DATA,            "PCAN_BUSSPEED_DATA" },
    { PCAN_IP_ADDRESS,               "PCAN_IP_ADDRESS" },
    { PCAN_LAN_SERVICE_STATUS,       "PCAN_LAN_SERVICE_STATUS" },
    { PCAN_ALLOW_STATUS_FRAMES,      "PCAN_ALLOW_STATUS_FRAMES" },
    { PCAN_ALLOW_RTR_FRAMES,         "PCAN_ALLOW_RTR_FRAMES" },
    { PCAN_ALLOW_ERROR_FRAMES,       "PCAN_ALLOW_ERROR_FRAMES" },
    { PCAN_INTERFRAME_DELAY,         "PCAN_INTERFRAME_DELAY" },
    { PCAN_ACCEPTANCE_FILTER_11BIT,  "PCAN_ACCEPTANCE_FILTER_11BIT" },
    { PCAN_ACCEPTANCE_FILTER_29BIT,  "PCAN_ACCEPTANCE_FILTER_29BIT" },
    { PCAN_IO_DIGITAL_CONFIGURATION, "PCAN_IO_DIGITAL_CONFIGURATION" },
    { PCAN_IO_DIGITAL_VALUE,         "PCAN_IO_DIGITAL_VALUE" },
    { PCAN_IO_DIGITAL_SET,           "PCAN_IO_DIGITAL_SET" },
    { PCAN_IO_DIGITAL_CLEAR,         "PCAN_IO_DIGITAL_CLEAR" },
    { PCAN_IO_ANALOG_VALUE,          "PCAN_IO_ANALOG_VALUE" },
    { PCAN_FIRMWARE_VERSION,         "PCAN_FIRMWARE_VERSION" },
    { PCAN_ATTACHED_CHANNELS_COUNT,  "PCAN_ATTACHED_CHANNELS_COUNT" },
    { PCAN_ATTACHED_CHANNELS,        "PCAN_ATTACHED_CHANNELS" },
    { 0, 0 }
};


// Default string for unknown parameters
const char pcanParameterUnknown[] = "Parameter unknown";


// String lookup table for device types
const lookup_t pcanDevices [] = {
    { PCAN_NONE,    "PCAN_NONE" },
    { PCAN_PEAKCAN, "PCAN_PEAKCAN" },
    { PCAN_ISA,     "PCAN_ISA" },
    { PCAN_DNG,     "PCAN_DNG" },
    { PCAN_PCI,     "PCAN_PCI" },
    { PCAN_USB,     "PCAN_USB" },
    { PCAN_PCC,     "PCAN_PCC" },
    { PCAN_VIRTUAL, "PCAN_VIRTUAL" },
    { PCAN_LAN,     "PCAN_LAN" },
    { 0, 0 },
};


// Default string for unknown device types
const char pcanDeviceUnknown[] = "Device type unknown";




// ----------------------------------- // -----------------------------------
// Public functions


const char *pcanStatusLookup(TPCANStatus status)
{
    const char *statusString = 0;

    statusString = lookupString(pcanStatuses, pcanStatusUnknown, status);

    return statusString;
}




void pcanDumpMsg(TPCANMsg *msg)
{
    printf("msg { \n"
           "  ID = 0x%02X\n"
           "  MSGTYPE = 0x%02X (%s)\n"
           "  LEN = 0x%02X\n"
           "  DATA = ",
           msg->ID,
           msg->MSGTYPE,
           pcanMsgtypeLookup(msg->MSGTYPE),
           msg->LEN);

    pcanDumpBuffer(msg->DATA, msg->LEN);
    
    printf("\n"
           "}\n");

    return;
}




void pcanDumpMsgFD(TPCANMsgFD *msg)
{
    int msgLength;

    msgLength = pcanDLCDecode(msg->DLC);

    printf("msg_fd {\n"
           "  ID = 0x%02X\n"
           "  MSGTYPE = 0x%02X (%s)\n"
           "  DLC = 0x%02X (0x%02X bytes)\n"
           "  DATA = ",
           msg->ID,
           msg->MSGTYPE,
           pcanMsgtypeLookup(msg->MSGTYPE),
           msg->DLC,
           msgLength);

    pcanDumpBuffer(msg->DATA, msgLength);

    printf("\n"
           "}\n");

    return;
}




void pcanDumpBuffer(BYTE *buffer, size_t bufferLength)
{
    unsigned int i;

    printf("{");

    for (i = 0; i < bufferLength; i++)
    {
        printf("0x%02X", buffer[i]);
        if (i != (bufferLength - 1))
        {
            printf(", ");
        }
        // Print comma following data byte only if not second-to-last
        // byte in byte
    }

    printf("}");

    return;
}




const char *pcanMsgtypeLookup(TPCANMessageType msgtype)
{
    const char *msgtypeString = 0;

    msgtypeString = lookupString(pcanMsgtypes, pcanMsgtypeUnknown, msgtype);
    
    return msgtypeString;
}




void pcanDumpTimestamp(TPCANTimestamp *timestamp)
{
    printf("timestamp {\n"
           "  millis = %i\n"
           "  millis_overflow = %i\n"
           "  micros = %i\n"
           "}\n",
           timestamp->millis,
           timestamp->millis_overflow,
           timestamp->micros);

    return;
}




void pcanDumpTimestampFD(TPCANTimestampFD timestamp)
{
    printf("timestampFD { %lli }\n", timestamp);

    return;
}




const char *pcanParameterLookup(TPCANParameter parameter)
{
    const char *parameterString;

    parameterString = lookupString(pcanParameters, pcanParameterUnknown, parameter);

    return parameterString;
}




uint8_t pcanDLCDecode(uint8_t dlc)
{
    int len;

    if (dlc <= 8)
    {
        len = dlc;
    }
    else
    {
        switch (dlc) {
        case 9:
            len = 12;
            break;
        case 10:
            len = 16;
            break;
        case 11:
            len = 20;
            break;
        case 12:
            len = 24;
            break;
        case 13:
            len = 32;
            break;
        case 14:
            len = 48;
            break;
        case 15:
            len = 64;
            break;
        }
    }

    return len;
}




void pcanDumpChannelInfo(TPCANChannelInformation *info)
{
    printf("TPCANChannelInfo {\n"
           "  channel_handle    = 0x%02X\n"
           "  device_type       = 0x%02X (%s)\n"
           "  controller_number = 0x%02X\n"
           "  device_features   = 0x%02X\n"
           "  device_name       = \"%s\"\n"
           "  device_id         = 0x%02X\n"
           "  channel_condition = 0x%02X\n"
           "}\n",
           info->channel_handle,
           info->device_type,
           pcanDeviceTypeLookup(info->device_type),
           info->controller_number,
           info->device_features,
           info->device_name,
           info->device_id,
           info->channel_condition);

    return;
}




const char *pcanDeviceTypeLookup(TPCANDevice device)
{
    const char *deviceString = 0;

    deviceString = lookupString(pcanDevices, pcanDeviceUnknown, device);

    return deviceString;
}




int pcanTranslateBaud(uint32_t baudInt)
{
    int translated = 0;

    switch (baudInt) {
    case 1000000: // 1M
        translated = PCAN_BAUD_1M;
        break;
    case 800000: // 800k
        translated = PCAN_BAUD_800K;
        break;
    case 500000: // 500k
        translated = PCAN_BAUD_500K;
        break;
    case 250000: // 250k
        translated = PCAN_BAUD_250K;
        break;
    case 125000: // 125k
        translated = PCAN_BAUD_125K;
        break;
    case 100000: // 100k
        translated = PCAN_BAUD_100K;
        break;
    case 95000: // 95k
        translated = PCAN_BAUD_95K;
        break;
    case 83000: // 83k
        translated = PCAN_BAUD_83K;
        break;
    case 50000: // 50k
        translated = PCAN_BAUD_50K;
        break;
    case 47000: // 47k
        translated = PCAN_BAUD_47K;
        break;
    case 33000: // 33k
        translated = PCAN_BAUD_33K;
        break;
    case 20000: // 20k
        translated = PCAN_BAUD_20K;
        break;
    case 10000: // 10k
        translated = PCAN_BAUD_10K;
        break;
    case 5000: // 5k
        translated = PCAN_BAUD_5K;
        break;
    default:
        translated = 0; // Unknown
        break;
    }

    return translated;
}

