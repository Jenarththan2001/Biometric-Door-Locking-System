#ifndef __AS608_H__
#define __AS608_H__

#include "driver/uart.h"
#include "esp_log.h"
#include <stdint.h>
#include <stdbool.h>

// ----------------------
// AS608 Status Codes
// ----------------------
#define FINGERPRINT_OK                    0x00
#define FINGERPRINT_PACKETRECIEVEERR      0x01
#define FINGERPRINT_NOFINGER              0x02
#define FINGERPRINT_IMAGEFAIL             0x03
#define FINGERPRINT_IMAGEMESS             0x06
#define FINGERPRINT_FEATUREFAIL           0x07
#define FINGERPRINT_NOMATCH               0x08
#define FINGERPRINT_NOTFOUND              0x09
#define FINGERPRINT_ENROLLMISMATCH        0x0A
#define FINGERPRINT_BADLOCATION           0x0B
#define FINGERPRINT_DBRANGEFAIL           0x0C
#define FINGERPRINT_UPLOADFEATUREFAIL     0x0D
#define FINGERPRINT_PACKETRESPONSEFAIL    0x0E
#define FINGERPRINT_UPLOADFAIL            0x0F
#define FINGERPRINT_DELETEFAIL            0x10
#define FINGERPRINT_DBCLEARFAIL           0x11
#define FINGERPRINT_PASSFAIL              0x13
#define FINGERPRINT_INVALIDIMAGE          0x15
#define FINGERPRINT_FLASHERR              0x18
#define FINGERPRINT_INVALIDREG            0x1A
#define FINGERPRINT_ADDRCODE              0x20
#define FINGERPRINT_PASSVERIFY            0x21

// ----------------------
// AS608 Packet Types
// ----------------------
#define AS608_COMMANDPACKET               0x01
#define AS608_DATAPACKET                  0x02
#define AS608_ACKPACKET                   0x07
#define AS608_ENDDATAPACKET               0x08

#ifdef __cplusplus
extern "C" {
#endif

// ----------------------
// AS608 API Functions
// ----------------------
bool     AS608_VerifyPassword(uart_port_t uart_num);
uint8_t  AS608_GetImage(uart_port_t uart_num);
uint8_t  AS608_GenChar(uart_port_t uart_num, uint8_t bufferID);
uint8_t  AS608_RegModel(uart_port_t uart_num);
uint8_t  AS608_Store(uart_port_t uart_num, uint8_t bufferID, uint16_t pageID);
uint8_t  AS608_Search(uart_port_t uart_num, uint8_t bufferID, uint16_t startPage, uint16_t pageNum, uint16_t *pageID, uint16_t *matchScore);
uint8_t  AS608_Delete(uart_port_t uart_num, uint16_t pageID, uint16_t count);
uint8_t  AS608_Empty(uart_port_t uart_num);
uint8_t  AS608_GetTemplateCount(uart_port_t uart_num);
const char* AS608_StatusString(uint8_t status);

#ifdef __cplusplus
}
#endif

#endif // __AS608_H__
