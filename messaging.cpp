// Copyright (C) 2013 Project Hashbang
// Created by Sultan Qasim Khan

#include <stdint.h>
#include <stellarino.h>
#include <crc.h>
#include <messaging.h>

#define UART_PORT 0 //5 //2
#define MESSAGE_TIMEOUT 500 // ms

#define NIBBLE2ASCII(x) ((x) < 0x0A ? '0' + (x) : 'A' + (x) - 0x0A)
#define ASCII2NIBBLE(x) ((x) >= 'A' ? (x) - 'A' + 0x0A : (x) - '0')

// The output buffer must be at least double the length of the input buffer
static inline uint8_t *toHex(uint8_t input)
{
    static uint8_t buff[3] = {0};

    buff[0] = NIBBLE2ASCII(input >> 4);
    buff[1] = NIBBLE2ASCII(input & 0x0F);

    return buff;
}


// The output buffer must be at least half the size of the input buffer
static inline uint8_t fromHex(const uint8_t input[2])
{
    uint8_t i;

    i = ASCII2NIBBLE(input[0]) << 4;
    i |= ASCII2NIBBLE(input[1]);

    return i;
}


static void writeHex(const uint8_t *buff, int buffSize)
{
    int i;
    uint8_t *hexByte;

    for (i = 0; i < buffSize; i++)
    {
        hexByte = toHex(buff[i]);
        UARTputc(UART_PORT, hexByte[0]);
        UARTputc(UART_PORT, hexByte[1]);
    }
}


// Waits timeout milliseconds for a char
// Returns the char if it succeeds, else returns -255
static int getCharWait(int timeout)
{
    unsigned long sTime = millis();
    int c;

    do
    {
        c = UARTpeek(UART_PORT);
    } while (c == -255 && millis() - sTime < timeout);

    if (c > -255) c = getc();   // Take the character off the buffer

    return c;
}

// Returns number of (binary) bytes read
// i.e. Returns two if it reads four bytes of hex data
static int readHex(uint8_t *buff, int buffSize)
{
    uint8_t hexByte[2];
    int i, c;

    for (i = 0; i < buffSize; i++)
    {
        c = getCharWait(MESSAGE_TIMEOUT);
        if (c == -255) break;
        hexByte[0] = c;

        c = getCharWait(MESSAGE_TIMEOUT);
        if (c == -255) break;
        hexByte[1] = c;

        buff[i] = fromHex(hexByte);
    }

    return i;
}


void enableMessaging(void)
{
    enableUART(UART_PORT, 115200);
}


void putMessage(MessageType msgType, uint16_t msgLength, uint16_t flags, const uint8_t *msg)
{
    static uint32_t curId = 0;  // Nasty way

    MsgHeader header;
    header.type = (uint16_t)msgType;
    header.flags = flags;
    header.length = msgLength;
    header.id = curId;
    header.crc = calcCrc16((uint8_t *)&header + 2, sizeof(MsgHeader) - 2);
    header.crc = calcCrc16(msg, msgLength, header.crc);

    curId++;

    // Start the message
    UARTputc(UART_PORT, '#');

    // Send the message header
    writeHex((uint8_t *)&header, sizeof(header));

    // Send the message
    writeHex(msg, msgLength);

    // End the message
    UARTputc(UART_PORT, '!');
}


// Returns 0 for success, 1 for timeout, 2 for CRC failure
int getMessage(MessageType *msgType, uint16_t *msgLength, uint16_t *flags, uint8_t *msg)
{
    int c, n;

    // Take in data until the start of a message
    do
    {
        c = getCharWait(MESSAGE_TIMEOUT);
    } while (c > -255 && c != '#');

    if (c == -255) return 1;

    MsgHeader header;

    // Read in the hex-encoded header
    n = readHex((uint8_t *)&header, sizeof(header));
    if (n < sizeof(header)) return 1;

    *msgType = (MessageType)header.type;
    *msgLength = header.length;
    *flags = header.flags;

    // Read in the message
    n = readHex(msg, header.length);
    if (n < header.length) return 1;

    // Perform a CRC16 check on the message
    uint16_t crc = calcCrc16((uint8_t *)&header + 2, sizeof(header) - 2);
    crc = calcCrc16(msg, header.length, crc);
    if (crc != header.crc) return 2;

    return 0;
}
