#ifndef INC_USARTBUFFERS_H_
#define INC_USARTBUFFERS_H_
#include "main.h"

typedef uint16_t bufferType;
typedef uint8_t commandType;
#define sizeOfCommand 30
#define sizeOfBuffer 1024
typedef struct CircularBuffer_ {
	volatile bufferType busy;
	volatile bufferType empty;
	volatile uint8_t array[sizeOfBuffer];
} CircularBuffer;

#define startByte 0x24
#define endByte 0x25
#define helpByte 0x40
#define startByteEncoded 0x41
#define endByteEncoded 0x42
#define helpByteEncoded 0x43

#define commandLED_on 0x00
#define commandLED_off 0x01
#define commandRTC_startReading 0x02
#define commandRTC_stopReading 0x03
#define commandRTC_printDate 0x04
#define commandRTC_sendByte 0x05
#define commandRTC_readByte 0x06
#define commandRTC_printBurstClock 0x07
#define commandRTC_printBurstRAM 0x08
#define commandRTC_sendBurstRAM 0x09
#define commandRTC_setTime 0x0A
#define commandRTC_setDate 0x0B
#define commandRTC_setFullDate 0x0C
#define commandRTC_enableWrite 0x0D
#define commandRTC_disableWrite 0x0E
#define commandRTC_readClockHaltFlag 0x0F
#define commandRTC_haltClock 0x10
#define commandRTC_poweronClock 0x11

void USART_Begin();
void USART_Send(uint8_t message[], bufferType lengthOfMsg);

void RxBufferLoop();
void executeCommand(uint8_t command[], uint8_t parametersAmmount);

#endif /* INC_USARTBUFFERS_H_ */
