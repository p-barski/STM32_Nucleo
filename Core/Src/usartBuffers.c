#include "usartBuffers.h"
#include "rtc_ds1302_functions.h"

CircularBuffer RxBuffer, TxBuffer;

void USART_Begin() {
	RxBuffer.busy = 0;
	RxBuffer.empty = 0;
	TxBuffer.busy = 0;
	TxBuffer.empty = 0;
	HAL_UART_Receive_IT(&huart2, &RxBuffer.array[RxBuffer.empty], 1);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART2) {
		RxBuffer.empty++;
		if (RxBuffer.empty >= sizeOfBuffer)
			RxBuffer.empty = 0;
		if (RxBuffer.busy == RxBuffer.empty)
			RxBuffer.busy++;
		if (RxBuffer.busy >= sizeOfBuffer)
			RxBuffer.busy = 0;
		HAL_UART_Receive_IT(&huart2, &RxBuffer.array[RxBuffer.empty], 1);
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART2) {
		if (TxBuffer.busy != TxBuffer.empty) {
			uint8_t tmpChar = TxBuffer.array[TxBuffer.busy];
			TxBuffer.busy++;
			if (TxBuffer.busy >= sizeOfBuffer)
				TxBuffer.busy = 0;
			HAL_UART_Transmit_IT(&huart2, &tmpChar, 1);
		}
	}
}

//Macro for incremeting empty and checking if empty or busy is out of bounds
#define emptyPlusPlus tmpEmpty++;\
	if (tmpEmpty >= sizeOfBuffer)\
		tmpEmpty = 0;\
	if (tmpEmpty == TxBuffer.busy)\
		TxBuffer.busy++;\
	if (TxBuffer.busy >= sizeOfBuffer)\
		TxBuffer.busy = 0;

void USART_Send(uint8_t message[], bufferType lengthOfMsg) {
	bufferType i, tmpEmpty;
	tmpEmpty = TxBuffer.empty;

	//Adding start byte on beginning
	TxBuffer.array[tmpEmpty] = startByte;
	emptyPlusPlus

	for (i = 0; i < lengthOfMsg; i++) {
		//Message longer than one frame
		if (i != 0 && (i % sizeOfCommand) == 0) {
			//Adding stop byte
			TxBuffer.array[tmpEmpty] = endByte;
			emptyPlusPlus
			//Adding start of new frame
			TxBuffer.array[tmpEmpty] = startByte;
			emptyPlusPlus
		}
		//If startByte is in the message, change it to helpByte and startByteEncoded
		if (message[i] == startByte) {
			TxBuffer.array[tmpEmpty] = helpByte;
			emptyPlusPlus
			TxBuffer.array[tmpEmpty] = startByteEncoded;
			emptyPlusPlus
		}
		//If helpByte is in the message, change it to helpByte and helpByteEncoded
		else if (message[i] == helpByte) {
			TxBuffer.array[tmpEmpty] = helpByte;
			emptyPlusPlus
			TxBuffer.array[tmpEmpty] = helpByteEncoded;
			emptyPlusPlus
		}
		//If endByte is in the message, change it to helpByte and endByteEncoded
		else if (message[i] == endByte) {
			TxBuffer.array[tmpEmpty] = helpByte;
			emptyPlusPlus
			TxBuffer.array[tmpEmpty] = endByteEncoded;
			emptyPlusPlus
		}
		//Regular characters
		else {
			TxBuffer.array[tmpEmpty] = message[i];
			emptyPlusPlus
		}
	}

	//Adding stop byte at the end
	TxBuffer.array[tmpEmpty] = endByte;
	emptyPlusPlus

	__disable_irq();
	if (TxBuffer.busy == TxBuffer.empty
			&& (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_TXE) == SET)) {
		TxBuffer.empty = tmpEmpty;
		uint8_t tmpChar = TxBuffer.array[TxBuffer.busy];
		TxBuffer.busy++;
		if (TxBuffer.busy >= sizeOfBuffer)
			TxBuffer.busy = 0;
		HAL_UART_Transmit_IT(&huart2, &tmpChar, 1);

	}
	else {
		TxBuffer.empty = tmpEmpty;
	}
	__enable_irq();
}

void RxBufferLoop() {
	static uint8_t i = 0;
	static uint8_t command[sizeOfCommand];
	static bool startWasSent = false;
	static bool helpByteWasSent = false;
	if (RxBuffer.busy != RxBuffer.empty) {
		//Inside of frame
		if (startWasSent) {
			//End of frame
			if (RxBuffer.array[RxBuffer.busy] == endByte && !helpByteWasSent) {
				//if i is lesser than 1 then frame is empty
				if (i >= 1)
					executeCommand(command, i - 1);
				startWasSent = false;
				i = 0;
			}
			//Start byte received when start was already received earlier
			else if (RxBuffer.array[RxBuffer.busy] == startByte)
				i = 0;
			//Command too long
			else if (i >= sizeOfCommand) {
				i = 0;
				startWasSent = false;
			}
			//Help byte part 1
			else if (RxBuffer.array[RxBuffer.busy] == helpByte
					&& !helpByteWasSent) {
				helpByteWasSent = true;
			}
			//Help byte part 2
			else if (helpByteWasSent) {
				helpByteWasSent = false;
				if (RxBuffer.array[RxBuffer.busy] == startByteEncoded) {
					command[i] = startByte;
					i++;
				}
				else if (RxBuffer.array[RxBuffer.busy] == helpByteEncoded) {
					command[i] = helpByte;
					i++;
				}
				else if (RxBuffer.array[RxBuffer.busy] == endByteEncoded) {
					command[i] = endByte;
					i++;
				}
				else if (RxBuffer.array[RxBuffer.busy] == startByte) {
					//New frame
					i = 0;
				}
				else {
					//Error
					i = 0;
					startWasSent = false;
				}
			}
			//Normal character
			else {
				command[i] = RxBuffer.array[RxBuffer.busy];
				i++;
			}
		}
		//Start of frame
		else if (RxBuffer.array[RxBuffer.busy] == startByte)
			startWasSent = true;
		RxBuffer.busy++;
		if (RxBuffer.busy >= sizeOfBuffer)
			RxBuffer.busy = 0;
	}
}

void executeCommand(uint8_t command[], uint8_t parametersAmmount) {
	switch (command[0]) {
	case commandLED_on:
		//Turning on LED
		if (parametersAmmount == 0)
			HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
		break;
	case commandLED_off:
		//Turning off LED
		if (parametersAmmount == 0)
			HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
		break;
	case commandRTC_startReading:
		//Reading date from RTC every second and sending that to PC
		if (parametersAmmount == 0)
			dateReading = true;
		break;
	case commandRTC_stopReading:
		//Stopping reading date every second
		if (parametersAmmount == 0)
			dateReading = false;
		break;
	case commandRTC_printDate:
		//Reads date and sends it to PC
		if (parametersAmmount == 0)
			RTCprintDate();
		break;
	case commandRTC_sendByte:
		//Sends given byte to RTC
		//Command byte has to be an even number between (128-188, 192-252)
		if (parametersAmmount == 2)
			RTCsendByte(command[1], command[2]);
		break;
	case commandRTC_readByte:
		//Sends read command and sends received byte to PC
		//Command byte has to be an even number between (128-188, 192-252)
		if (parametersAmmount == 1) {
			uint8_t msg[21];
			sprintf(msg, "Odczytany bajt: %d", RTCreadByte(command[1]));
			USART_Send(msg, strlen(msg));
		}
		break;
	case commandRTC_printBurstClock:
		//Reads date and sends it to PC
		//The same as printDate but uses burst mode
		if (parametersAmmount == 0)
			RTCprintDateBurst();
		break;
	case commandRTC_printBurstRAM:
		//Reads RAM content of RTC and sends it to PC
		if (parametersAmmount == 0)
			RTCprintRAM();
		break;
	case commandRTC_sendBurstRAM:
		//Sends data to RTC RAM
		if (parametersAmmount == 29) {
			commandType i;
			uint8_t RAM[31];
			for (i = 1; i <= 29; i++) {
				RAM[i - 1] = command[i];
			}
			RAM[29] = RTCreadByte(0xFA);
			RAM[30] = RTCreadByte(0xFC);
			RTCsendBurstRAM(RAM);
			break;
		}
	case commandRTC_setTime:
		//Sets time on RTC, format - hour(0-23), minute(0-59), second(0-59)
		//Doesn't change date on RTC
		if (parametersAmmount == 3) {
			RTCsetTime(
					constructor(command[1], command[2], command[3], 1, 1, 1,
							1));
		}
		break;
	case commandRTC_setDate:
		//Sets date on RTC, format - day of the week(1-7), day of the month(1-31), month(1-12), year(0-99)
		//Doesn't change time on RTC
		if (parametersAmmount == 4) {
			RTCsetDate(
					constructor(1, 1, 1, command[1], command[2], command[3],
							command[4]));
		}
		break;
	case commandRTC_setFullDate:
		//Sets date and time on RTC, format - hour(0-23), minute(0-59), second(0-59),
		//day of the week(1-7), day of the month(1-31), month(1-12), year(0-99)
		if (parametersAmmount == 7) {
			RTCsetFullDate(
					constructor(command[1], command[2], command[3], command[4],
							command[5], command[6], command[7]));
		}
		break;
	case commandRTC_enableWrite:
		//Enables writing to RTC
		if (parametersAmmount == 0)
			RTCsetWriteProtectBit(false);
		break;
	case commandRTC_disableWrite:
		//Disables writing to RTC
		if (parametersAmmount == 0)
			RTCsetWriteProtectBit(true);
		break;
	case commandRTC_readClockHaltFlag:
		//Reads clock halt flag
		if (parametersAmmount == 0) {
			if (RTCreadClockHaltFlag())
				USART_Send("true", 4);
			else
				USART_Send("false", 5);
		}
		break;
	case commandRTC_haltClock:
		//Disables clock
		if (parametersAmmount == 0) {
			RTCsetClockHaltFlag(true);
		}
		break;
	case commandRTC_poweronClock:
		//Enables clock
		if (parametersAmmount == 0)
			RTCsetClockHaltFlag(false);
		break;
	}
}
