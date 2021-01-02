#include "rtc_ds1302_functions.h"
#include "main.h"
#include "usartBuffers.h"

//READ FUNCTIONS

//Sends command byte to RTC and then reads one byte from RTC
//Returns 0 if command byte is wrong
uint8_t RTCreadByte(uint8_t commandByte) {
	if (commandByte < seconds_address || (commandByte & RTC_read)
			|| commandByte == clockBurst_address
			|| commandByte == RAMBurst_address) {
		uint8_t msg[50];
		sprintf(msg, "Wrong command byte (%d) when reading", commandByte);
		USART_Send(msg, strlen(msg));
		return 0;
	}
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
	uint8_t byteFromRTC = 0;
	commandByte = commandByte | RTC_read;
	HAL_SPI_Transmit(&hspi2, &commandByte, 1, 10);
	HAL_SPI_Receive(&hspi2, &byteFromRTC, 1, 10);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	return byteFromRTC;
}

//Reads RTC clock in burst mode
//Returned pointer has to be freed
uint8_t* RTCreadBurstClock() {
	uint8_t *clockData = (uint8_t*) malloc(9);
	uint8_t burstAddress = clockBurst_address | RTC_read;
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
	HAL_SPI_Transmit(&hspi2, &burstAddress, 1, 10);
	HAL_SPI_Receive(&hspi2, clockData, 9, 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	return clockData;
}

//Reads RTC RAM in burst mode
//Returned pointer has to be freed
uint8_t* RTCreadBurstRAM() {
	uint8_t *clockData = (uint8_t*) malloc(31);
	uint8_t burstAddress = RAMBurst_address | RTC_read;
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
	HAL_SPI_Transmit(&hspi2, &burstAddress, 1, 10);
	HAL_SPI_Receive(&hspi2, clockData, 31, 300);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
	return clockData;
}

//Returns true or false depending on clock halt flag
bool RTCreadClockHaltFlag() {
	if (RTCreadByte(seconds_address) & 0x80)
		return true;
	return false;
}

//Reads date from RTC and prints it
void RTCprintDate() {
	Date date;
	date.seconds = bcdToDec(RTCreadByte(seconds_address) & 0x7F);
	date.minutes = bcdToDec(RTCreadByte(minutes_address) & 0x7F);
	date.hours = bcdToDec(RTCreadByte(hours_address) & 0x7F);
	date.dayOfTheMonth = bcdToDec(RTCreadByte(dayOfTheMonth_address) & 0x3F);
	date.month = bcdToDec(RTCreadByte(month_address) & 0x1F);
	date.dayOfTheWeek = RTCreadByte(dayOfTheWeek_address) & 0x07;
	date.years = bcdToDec(RTCreadByte(years_address));
	int8_t komunikat[50];
	sprintf(komunikat,
			"Godzina:%02d:%02d:%02d,Data:%02d.%02d.%02dDzień tygodnia: %d",
			date.hours, date.minutes, date.seconds, date.dayOfTheMonth,
			date.month, date.years, date.dayOfTheWeek);
	USART_Send(komunikat, strlen(komunikat));
}

//The same as RTCprintDate but uses burst read
void RTCprintDateBurst() {
	uint8_t *clock = RTCreadBurstClock();
	Date date = constructor(bcdToDec(clock[2] & 0x7F),
			bcdToDec(clock[1] & 0x7F), bcdToDec(clock[0] & 0x7F),
			clock[5] & 0x07, bcdToDec(clock[3] & 0x3F),
			bcdToDec(clock[4] & 0x1F), bcdToDec(clock[6]));
	int8_t komunikat[50];
	sprintf(komunikat,
			"Godzina:%02d:%02d:%02d,Data:%02d.%02d.%02dDzień tygodnia: %d",
			date.hours, date.minutes, date.seconds, date.dayOfTheMonth,
			date.month, date.years, date.dayOfTheWeek);
	USART_Send(komunikat, strlen(komunikat));
	free(clock);
}

//Reads RTC RAM in burst mode and prints it
void RTCprintRAM() {
	uint8_t *RAM = RTCreadBurstRAM();
	uint8_t i;
	int8_t komunikat[16];
	for (i = 0; i < 31; i++) {
		sprintf(komunikat, "Bajt %d: %d", 192 + (2 * i), RAM[i]);
		USART_Send(komunikat, strlen(komunikat));
	}
	free(RAM);
}

//SEND FUNCTIONS

//Sends command byte to RTC and then sends data byte to RTC
void RTCsendByte(uint8_t commandByte, uint8_t dataByte) {
	if (commandByte < seconds_address || (commandByte & RTC_read)
			|| commandByte == clockBurst_address
			|| commandByte == RAMBurst_address) {
		uint8_t msg[50];
		sprintf(msg, "Wrong command byte (%d) when sending", commandByte);
		USART_Send(msg, strlen(msg));
		return;
	}
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
	HAL_SPI_Transmit(&hspi2, &commandByte, 1, 10);
	HAL_SPI_Transmit(&hspi2, &dataByte, 1, 10);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
}

//Sends data to RTC in burst mode
void RTCsendBurstClock(uint8_t data[]) {
	uint8_t address = clockBurst_address;
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
	HAL_SPI_Transmit(&hspi2, &address, 1, 10);
	HAL_SPI_Transmit(&hspi2, data, 9, 100);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
}

//Sends data to RTC RAM in burst mode
void RTCsendBurstRAM(uint8_t data[]) {
	uint8_t address = RAMBurst_address;
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
	HAL_SPI_Transmit(&hspi2, &address, 1, 10);
	HAL_SPI_Transmit(&hspi2, data, 31, 300);
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
}

//Sets clock halt flag to 0 if argument is false
//Sets clock halt flag to 1 if argument is true
void RTCsetClockHaltFlag(bool haltFlag) {
	if (haltFlag)
		RTCsendByte(seconds_address, (RTCreadByte(seconds_address) | 0x80));
	else
		RTCsendByte(seconds_address, (RTCreadByte(seconds_address) & 0x7F));
}

//Sets write protect bit to 0 if argument is false
//Sets write protect bit to 1 if argument is true
void RTCsetWriteProtectBit(bool bit) {
	if (bit)
		RTCsendByte(writeProtectBit_address, 0x80);
	else
		RTCsendByte(writeProtectBit_address, 0x00);
}

//Sets time on RTC
void RTCsetTime(Date date) {
	if (date.seconds == 255)
		return;
	RTCsendByte(seconds_address, decToBcd(date.seconds));
	RTCsendByte(minutes_address, decToBcd(date.minutes));
	RTCsendByte(hours_address, decToBcd(date.hours));
}

//Sets date on RTC
void RTCsetDate(Date date) {
	if (date.seconds == 255)
		return;
	RTCsendByte(dayOfTheMonth_address, decToBcd(date.dayOfTheMonth));
	RTCsendByte(month_address, decToBcd(date.month));
	RTCsendByte(dayOfTheWeek_address, decToBcd(date.dayOfTheWeek));
	RTCsendByte(years_address, decToBcd(date.years));
}

//Sets full date on RTC based on argument
void RTCsetFullDate(Date date) {
	RTCsetTime(date);
	RTCsetDate(date);
}

//OTHER

//Changes BCD uint8 to decimal
uint8_t bcdToDec(uint8_t bcd) {
	return (((bcd >> 4) * 10) + (bcd & 0x0F));
}

//Changes decimal uint8 to BCD
uint8_t decToBcd(uint8_t dec) {
	return (((dec / 10) << 4) | (dec % 10));
}

//Constructor for Date struct
Date constructor(uint8_t hours, uint8_t minutes, uint8_t seconds,
		uint8_t dayOfTheWeek, uint8_t dayOfTheMonth, uint8_t month,
		uint8_t years) {
	Date date;
	if (seconds >= 60 || minutes >= 60 || hours >= 24 || dayOfTheMonth > 31
			|| dayOfTheMonth == 0 || month > 12 || month == 0
			|| dayOfTheWeek > 7 || dayOfTheWeek == 0 || years >= 100) {
		USART_Send("Wrong date", 10);
		date.seconds = 255;
		return date;
	}
	date.seconds = seconds;
	date.minutes = minutes;
	date.hours = hours;
	date.dayOfTheMonth = dayOfTheMonth;
	date.month = month;
	date.dayOfTheWeek = dayOfTheWeek;
	date.years = years;
	return date;
}
