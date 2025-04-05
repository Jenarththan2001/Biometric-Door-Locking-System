#include "as608.h"
#include <string.h>

// Static packet buffer
static uint8_t packet[32] = {0};

// --------------------
// Compute checksum
// --------------------
static uint16_t AS608_Checksum(uint8_t *data, uint16_t len)
{
    uint16_t sum = 0;
    for (int i = 6; i < len - 2; i++) {
        sum += data[i];
    }
    return sum;
}

// --------------------
// Send Packet
// --------------------
static void AS608_SendPacket(uart_port_t uart_num, uint8_t *data, size_t len)
{
    uint16_t sum = AS608_Checksum(data, len);
    data[len - 2] = (sum >> 8) & 0xFF;
    data[len - 1] = sum & 0xFF;
    uart_write_bytes(uart_num, (const char *)data, len);
}

// --------------------
// Read Packet
// --------------------
static int AS608_ReadPacket(uart_port_t uart_num, uint8_t *buffer, size_t len, uint32_t timeout_ms)
{
    return uart_read_bytes(uart_num, buffer, len, timeout_ms / portTICK_PERIOD_MS);
}

// --------------------
// Verify Password
// --------------------
bool AS608_VerifyPassword(uart_port_t uart_num)
{
    uint8_t cmd[16] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x07, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    AS608_SendPacket(uart_num, cmd, sizeof(cmd));
    memset(packet, 0, sizeof(packet));
    int len = AS608_ReadPacket(uart_num, packet, 12, 1000);
    if (len != 12) return false;
    return (packet[9] == FINGERPRINT_OK);
}

// --------------------
// Get Image
// --------------------
uint8_t AS608_GetImage(uart_port_t uart_num)
{
    uint8_t cmd[12] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x01, 0x00, 0x00};
    AS608_SendPacket(uart_num, cmd, sizeof(cmd));
    memset(packet, 0, sizeof(packet));
    int len = AS608_ReadPacket(uart_num, packet, 12, 1000);
    if (len != 12) return 0xFF;
    return packet[9];
}

// --------------------
// Generate Character File
// --------------------
uint8_t AS608_GenChar(uart_port_t uart_num, uint8_t bufferID)
{
    uint8_t cmd[13] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x04, 0x02, bufferID, 0x00, 0x00};
    AS608_SendPacket(uart_num, cmd, sizeof(cmd));
    memset(packet, 0, sizeof(packet));
    int len = AS608_ReadPacket(uart_num, packet, 12, 1000);
    if (len != 12) return 0xFF;
    return packet[9];
}

// --------------------
// Register Model (merge CharBuffer1 & CharBuffer2)
// --------------------
uint8_t AS608_RegModel(uart_port_t uart_num)
{
    uint8_t cmd[12] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x05, 0x00, 0x00};
    AS608_SendPacket(uart_num, cmd, sizeof(cmd));
    memset(packet, 0, sizeof(packet));
    int len = AS608_ReadPacket(uart_num, packet, 12, 1000);
    if (len != 12) return 0xFF;
    return packet[9];
}

// --------------------
// Store Template
// --------------------
uint8_t AS608_Store(uart_port_t uart_num, uint8_t bufferID, uint16_t pageID)
{
    uint8_t cmd[15] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x06, 0x06, bufferID, (uint8_t)(pageID >> 8), (uint8_t)(pageID & 0xFF), 0x00, 0x00};
    AS608_SendPacket(uart_num, cmd, sizeof(cmd));
    memset(packet, 0, sizeof(packet));
    int len = AS608_ReadPacket(uart_num, packet, 12, 1000);
    if (len != 12) return 0xFF;
    return packet[9];
}

// --------------------
// Search Finger
// --------------------
uint8_t AS608_Search(uart_port_t uart_num, uint8_t bufferID, uint16_t startPage, uint16_t pageNum, uint16_t *pageID, uint16_t *matchScore)
{
    uint8_t cmd[17] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x08, 0x04, bufferID, (uint8_t)(startPage >> 8), (uint8_t)(startPage & 0xFF), (uint8_t)(pageNum >> 8), (uint8_t)(pageNum & 0xFF), 0x00, 0x00};
    AS608_SendPacket(uart_num, cmd, sizeof(cmd));
    memset(packet, 0, sizeof(packet));
    int len = AS608_ReadPacket(uart_num, packet, 16, 1000);
    if (len != 16) return 0xFF;
    *pageID = ((uint16_t)packet[10] << 8) | packet[11];
    *matchScore = ((uint16_t)packet[12] << 8) | packet[13];
    return packet[9];
}

// --------------------
// Delete Template
// --------------------
uint8_t AS608_Delete(uart_port_t uart_num, uint16_t pageID, uint16_t count)
{
    uint8_t cmd[16] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x07, 0x0C, (uint8_t)(pageID >> 8), (uint8_t)(pageID & 0xFF), (uint8_t)(count >> 8), (uint8_t)(count & 0xFF), 0x00, 0x00};
    AS608_SendPacket(uart_num, cmd, sizeof(cmd));
    memset(packet, 0, sizeof(packet));
    int len = AS608_ReadPacket(uart_num, packet, 12, 1000);
    if (len != 12) return 0xFF;
    return packet[9];
}

// --------------------
// Get Template Count
// --------------------
uint8_t AS608_GetTemplateCount(uart_port_t uart_num)
{
    uint8_t cmd[12] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x1D, 0x00, 0x00};
    AS608_SendPacket(uart_num, cmd, sizeof(cmd));
    memset(packet, 0, sizeof(packet));
    int len = AS608_ReadPacket(uart_num, packet, 14, 1000);
    if (len != 14) return 0xFF;
    return packet[9] == FINGERPRINT_OK ? packet[11] : 0xFF;
}

// --------------------
// Status to String
// --------------------
const char* AS608_StatusString(uint8_t status)
{
    switch (status) {
        case FINGERPRINT_OK: return "OK";
        case FINGERPRINT_NOFINGER: return "No Finger Detected";
        case FINGERPRINT_IMAGEFAIL: return "Imaging Error";
        case FINGERPRINT_IMAGEMESS: return "Image Too Messy";
        case FINGERPRINT_FEATUREFAIL: return "Feature Generation Failed";
        case FINGERPRINT_NOMATCH: return "No Match Found";
        case FINGERPRINT_NOTFOUND: return "Template Not Found";
        case FINGERPRINT_PACKETRECIEVEERR: return "Packet Receive Error";
        case FINGERPRINT_FLASHERR: return "Flash Write Error";
        case FINGERPRINT_BADLOCATION: return "Bad PageID Location";
        case FINGERPRINT_DELETEFAIL: return "Delete Failed";
        default: return "Unknown Error";
    }
}
