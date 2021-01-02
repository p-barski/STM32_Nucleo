#ifndef INC_RTC_DS1302_FUNCTIONS_H_
#define INC_RTC_DS1302_FUNCTIONS_H_
#include "main.h"

typedef struct Date_ {
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hours;
	uint8_t dayOfTheMonth;
	uint8_t month;
	uint8_t dayOfTheWeek;
	uint8_t years;
} Date;

#define RTC_read 0x01
#define seconds_address 0x80
#define minutes_address 0x82
#define hours_address 0x84
#define dayOfTheMonth_address 0x86
#define month_address 0x88
#define dayOfTheWeek_address 0x8A
#define years_address 0x8C
#define writeProtectBit_address 0x8E
#define clockBurst_address 0xBE
#define RAMBurst_address 0xFE

//READ FUNCTIONS

//Sends command byte to RTC and then reads one byte from RTC
//Returns 0 if command byte is wrong
uint8_t RTCreadByte(uint8_t commandByte);

//Reads RTC clock in burst mode
//Returned pointer has to be freed
uint8_t* RTCreadBurstClock();

//Reads RTC RAM in burst mode
//Returned pointer has to be freed
uint8_t* RTCreadBurstRAM();

//Returns true or false depending on clock halt flag
bool RTCreadClockHaltFlag();

//Reads date from RTC and prints it
void RTCprintDate();

//The same as RTCprintDate but uses burst read
void RTCprintDateBurst();

//Reads RTC RAM in burst mode and prints it
void RTCprintRAM();

//SEND FUNCTIONS

//Sends command byte to RTC and then sends data byte to RTC
void RTCsendByte(uint8_t commandByte, uint8_t dataByte);

//Sends data to RTC in burst mode
void RTCsendBurstClock(uint8_t data[]);

//Sends data to RTC RAM in burst mode
void RTCsendBurstRAM(uint8_t data[]);

//Sets clock halt flag to 0 if argument is false
//Sets clock halt flag to 1 if argument is true
void RTCsetClockHaltFlag(bool haltFlag);

//Sets write protect bit to 0 if argument is false
//Sets write protect bit to 1 if argument is true
void RTCsetWriteProtectBit(bool bit);

//Sets time on RTC
void RTCsetTime(Date date);

//Sets date on RTC
void RTCsetDate(Date date);

//Sets date on RTC based on argument
void RTCsetFullDate(Date date);

//OTHER

//Changes BCD uint8 to decimal
uint8_t bcdToDec(uint8_t bcd);

//Changes decimal uint8 to BC
uint8_t decToBcd(uint8_t dec);

//Constructor for Date struct
Date constructor(uint8_t seconds, uint8_t minutes, uint8_t hours,
		uint8_t dayOfTheMonth, uint8_t month, uint8_t dayOfTheWeek,
		uint8_t years);

#endif /* INC_RTC_DS1302_FUNCTIONS_H_ */
