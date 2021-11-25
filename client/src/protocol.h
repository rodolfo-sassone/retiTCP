/*
 * protocol.h
 *
 *  Created on: 23 nov 2021
 *      Author: Rodolfo Pio Sassone
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#define LENGHT_IP 16

typedef struct
{
	char operator;
	int num1;
	int num2;
	int result;
}message;

#endif /* PROTOCOL_H_ */
