
/*{

#include <IOKit/IOTypes.h>

#define OP_NONE               0
#define OP_LIST               1 
#define OP_READ               2
#define OP_READ_FAN           3
#define OP_WRITE              4

#define KERNEL_INDEX_SMC      2

#define SMC_CMD_READ_BYTES    5
#define SMC_CMD_WRITE_BYTES   6
#define SMC_CMD_READ_INDEX    8
#define SMC_CMD_READ_KEYINFO  9
#define SMC_CMD_READ_PLIMIT   11
#define SMC_CMD_READ_VERS     12

#define DATATYPE_FPE2         "fpe2"
#define DATATYPE_UINT8        "ui8 "
#define DATATYPE_UINT16       "ui16"
#define DATATYPE_UINT32       "ui32"
#define DATATYPE_SP78         "sp78"

// key values
#define SMC_KEY_FAN0_RPM_MIN  "F0Mn"
#define SMC_KEY_FAN1_RPM_MIN  "F1Mn"
#define SMC_KEY_FAN0_RPM_CUR  "F0Ac"
#define SMC_KEY_FAN1_RPM_CUR  "F1Ac"
enum {
 kSMCSuccess = 0,
 kSMCError = 1
};
 
enum {
 kSMCUserClientOpen  = 0,
 kSMCUserClientClose = 1,
 kSMCHandleYPCEvent  = 2, 
      kSMCReadKey         = 5,
 kSMCWriteKey        = 6,
 kSMCGetKeyCount     = 7,
 kSMCGetKeyFromIndex = 8,
 kSMCGetKeyInfo      = 9
};
 
typedef struct SMCVersion 
{
    unsigned char    major;
    unsigned char    minor;
    unsigned char    build;
    unsigned char    reserved;
    unsigned short   release;
     
} SMCVersion;
 
typedef struct SMCPLimitData 
{
    uint16_t    version;
    uint16_t    length;
    uint32_t    cpuPLimit;
    uint32_t    gpuPLimit;
    uint32_t    memPLimit;
     
} SMCPLimitData;
 
typedef struct SMCKeyInfoData 
{
    IOByteCount         dataSize;
    uint32_t            dataType;
    uint8_t             dataAttributes;
     
} SMCKeyInfoData;
 
typedef struct {
    uint32_t            key;
    SMCVersion          vers;
    SMCPLimitData       pLimitData;
    SMCKeyInfoData      keyInfo;
    uint8_t             result;
    uint8_t             status;
    uint8_t             data8;
    uint32_t            data32;    
    uint8_t             bytes[32];
}  SMCParamStruct;

}*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <IOKit/IOKitLib.h>


#include "smc.h"

static io_connect_t conn = 0;

uint32_t
key_to_int(const char *str, int size)
{
    uint32_t total = 0;
    int c;
     
    for (c = 0; c < size; c++) {
        total += str[c] << (size - 1 - c) * 8;
    }
    return total;
}

float _strtof(uint8_t *str, int size, int e)
{
    float total = 0;
    int i;

    for (i = 0; i < size; i++)
    {
        if (i == (size - 1))
           total += (str[i] & 0xff) >> e;
        else
           total += str[i] << (size - 1 - i) * (8 - e);
    }

    return total;
}

int SMCOpen(void)
{
    IOReturn       result;
    io_service_t   device;

    device = IOServiceGetMatchingService( kIOMasterPortDefault, IOServiceMatching("AppleSMC"));
    if (IO_OBJECT_NULL == device)
        return -1;
     
    result = IOServiceOpen(device, mach_task_self(), 1, &conn);
    IOObjectRelease(device);
    return 0;
}

io_connect_t SMCClose(void)
{
    io_connect_t result;

    if (conn != 0) {
        result = IOServiceClose(conn);
        if (result == 0) 
            conn = 0;
        return result;
    }
    return -1;
}

IOReturn
SMCCall(int index,   SMCParamStruct* input,  SMCParamStruct* output)
{
    IOReturn result;

    if (conn == 0 && (result = SMCOpen()) != 0)
        return result;

    result = IOConnectCallMethod(
                                          conn, 
                                          kSMCUserClientOpen, 
                                          NULL, 
                                          0, 
                                          NULL, 
                                          0, 
                                          NULL, 
                                          NULL, 
                                          NULL, 
                                          NULL);
    if (kIOReturnSuccess != result)
        return result;
     
    size_t outSize = sizeof(SMCParamStruct);
    result = IOConnectCallStructMethod(
                                       conn, 
                                       index, 
                                       input, 
                                       sizeof(SMCParamStruct), 
                                       output, 
                                       &outSize);
    IOConnectCallMethod(
                        conn, 
                        kSMCUserClientClose, 
                        NULL, 
                        0, 
                        NULL, 
                        0, 
                        NULL, 
                        NULL, 
                        NULL, 
                        NULL);
    return result;
}

IOReturn
SMCReadKey(uint32_t     key, 
         uint8_t*     out, 
         uint8_t*     size)
{
    SMCParamStruct input;
    SMCParamStruct output;
     
    bzero(out,     *size);
    bzero(&input,  sizeof(SMCParamStruct));
    bzero(&output, sizeof(SMCParamStruct));
     
    // Advise the SMC to get meta-info about a key
    input.data8 = kSMCGetKeyInfo;
    input.key   = key;

    IOReturn result = SMCCall( kSMCHandleYPCEvent, 
                               &input, 
                               &output);
    if (kSMCSuccess != result)
        return result;
     
    // Get the value for the key
    input.data8 = kSMCReadKey;
    input.key   = key;
    input.keyInfo.dataSize = output.keyInfo.dataSize;
     
    bzero(&output, sizeof(SMCParamStruct));
     
    result = SMCCall(kSMCHandleYPCEvent, &input, &output);
    if (kSMCSuccess != result)
        return result;
     
    // Copy the data to out
    if (*size > input.keyInfo.dataSize)
        *size = input.keyInfo.dataSize;
     
    memcpy(out, output.bytes, *size);
    return kSMCSuccess;
}

float
SMCGetFanRPM(const char *key_text) {
    uint8_t buffer[256];
    uint8_t size=255;
    float result;
    uint32_t key;

    key = key_to_int(key_text, 4);
    result = SMCReadKey(key, buffer, &size);
    if (result != 0) 
        return -1;
    
    result = _strtof(buffer, (int)size, 2);
    SMCClose();
    return result;
}

double
SMCGetTemperature(const char *key_text) {
    uint8_t buffer[256];
    uint8_t size=255;
    double result;
    uint32_t key;

    key = key_to_int(key_text, 4);
    result = SMCReadKey(key, buffer, &size);
    if (result != 0) 
        return -10000.0;

    result = ((uint16_t)buffer[1] + ((uint16_t)buffer[0] << 8)) / 256.0;
    SMCClose();
    return result;
}

