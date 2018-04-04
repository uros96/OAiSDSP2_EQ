#include "processing.h"
#include "iir.h"

#define INT16_N 32767 //normal value of 1 in int16
#define INT16_S -32768 // normal value of -1 in int16
#define INT16_H 16384 // normal value of 1/2 in int16

void calculateShelvingCoeff(float c_alpha, Int16* output) {
	output[0] = c_alpha * INT16_N;
	output[1] = INT16_S;
	output[2] = INT16_N;
	output[3] = c_alpha * INT16_S;
}

void calculatePeekCoeff(float c_alpha, float c_beta, Int16* output) {
	output[0] = c_alpha * INT16_N;
	output[1] = -INT16_H * c_beta * (1 + c_alpha);
	output[2] = INT16_N;
	output[3] = INT16_N;
	output[4] = -INT16_H * c_beta * (1 + c_alpha);
	output[5] = c_alpha * INT16_N;
}

Int16 shelvingHP(Int16 input, Int16* coeff, Int16* x_history, Int16* y_history,
		Int16 k) {
	/* Your code here */

	Int16 buffer;
	Int32 output;

	buffer = first_order_IIR(input, coeff, x_history, y_history);
	output = (Int32) _smpy((((Int32) input + buffer) >> 1), k)
			+ (((Int32) input - buffer) >> 1);

	if (output > INT16_N)
		return INT16_N;
	else if (output < INT16_S)
		return INT16_S;
	else
		return (Int16) output;
}

Int16 shelvingLP(Int16 input, Int16* coeff, Int16* x_history, Int16* y_history,
		Int16 k) {
	/* Your code here */
	Int16 buffer;
	Int32 output;

	buffer = first_order_IIR(input, coeff, x_history, y_history);

	output = (Int32) _smpy(((Int32) input - buffer >> 1), k)
			+ (((Int32) input + buffer) >> 1);

	if (output > INT16_N)
		return INT16_N;
	else if (output < INT16_S)
		return INT16_S;
	else
		return (Int16) output;
}

Int16 shelvingPeek(Int16 input, Int16* coeff, Int16* x_history,
		Int16* y_history, Int16 k) {
	Int16 buffer;
	Int32 output;

	buffer = second_order_IIR(input, coeff, x_history, y_history);
	output = (Int32) _smpy((((Int32) input - buffer) >> 1), k)
			+ (((Int32) input + buffer) >> 1);

	if (output > INT16_N)
		return INT16_N;
	else if (output < INT16_S)
		return INT16_S;
	else
		return (Int16) output;
}
