/*
 * Copyright (c) 2011-2014, SmeshLink Technology Ltd.
 * All rights reserved.
 *
 * RestResponse.cpp
 *
 *      Contiki REST response wrapper.
 *
 *      Author: Long
 */

#include <stdarg.h>
#include "RestResponse.h"

RestResponse::RestResponse(void *response, uint8_t *buffer, int32_t *offset) :
		response(response), buffer(buffer), offset(offset), payloadLength(0) {

}

RestResponse::~RestResponse() {
	flush();
}

/*
 * Prints a char.
 *
 * \param c the char to print
 * \return the actual count of written bytes.
 */
size_t RestResponse::print(char c) {
	return printf("%c", c);
}

/*
 * Prints an int.
 *
 * \param i the int to print
 * \return the actual count of written bytes.
 */
size_t RestResponse::print(int i) {
	return printf("%d", i);
}

/*
 * Prints an unsigned int.
 *
 * \param u the unsigned int to print
 * \return the actual count of written bytes.
 */
size_t RestResponse::print(unsigned int u) {
	return printf("%u", u);
}

/*
 * Prints a long.
 *
 * \param l the long to print
 * \return the actual count of written bytes.
 */
size_t RestResponse::print(long l)
{
	return printf("%l", l);
}

/*
 * Prints an unsigned long.
 *
 * \param u the unsigned long to print
 * \return the actual count of written bytes.
 */
size_t RestResponse::print(unsigned long u)
{
	return printf("%lu", u);
}

/*
 * Prints a float.
 *
 * \param f the float to print
 * \param digits the precision
 * \return the actual count of written bytes.
 */
size_t RestResponse::print(float f, int digits) {
#if CONTIKI
	return printFloat(f, digits);
#else
	return printf("%f", f);
#endif
}

/*
 * Prints a double.
 *
 * \param d the double to print
 * \param digits the precision
 * \return the actual count of written bytes.
 */
size_t RestResponse::print(double d, int digits) {
#if CONTIKI
	return printFloat(d, digits);
#else
	return printf("%lf", d);
#endif
}

/*
 * Prints a string.
 *
 * \param s the string to print
 * \return the actual count of written bytes.
 */
size_t RestResponse::print(const char *s) {
	return write(s, strlen(s));
}

/*
 * Prints a formatted string.
 */
size_t RestResponse::printf(const char *format, ...) {
	va_list arg_ptr;
	va_start(arg_ptr, format);
	int printed = vsnprintf((char *)buffer + payloadLength, REST_MAX_CHUNK_SIZE - payloadLength, format, arg_ptr);
	va_end(arg_ptr);
	payloadLength += printed;
	return printed;
}

/*
 * Writes some bytes.
 *
 * \param data the bytes to write
 * \param length the length of data
 * \return the actual count of written bytes.
 *         This might be less than length if there is no enough space left in buffer.
 */
size_t RestResponse::write(const char *data, size_t length) {
	size_t copied = REST_MAX_CHUNK_SIZE - payloadLength;
	if (copied > length)
		copied = length;
	memcpy(this->buffer + payloadLength, data, copied);
	payloadLength += copied;
	return copied;
}

/*
 * Flushes the response.
 */
void RestResponse::flush() {
	REST.set_response_payload(response, buffer, payloadLength);
}

/*
 * Sets the status code of a response.
 */
int RestResponse::setStatusCode(unsigned int code) {
	return REST.set_response_status(response, code);
}

/*
 * Sets the Content-Type of a response.
 */
int RestResponse::setContentType(unsigned int contentType) {
	return REST.set_header_content_type(response, contentType);
}

/*
 * Sets the payload option of a response.
 */
int RestResponse::setPayload(const void *payload, size_t length) {
	setPayloadLength(length);
	return REST.set_response_payload(response, payload, length);
}

/*
 * Sets the payload length of a response.
 */
void RestResponse::setPayloadLength(size_t length) {
	payloadLength = length;
}

/*
 * Sets the Max-Age option of a response.
 */
int RestResponse::setMaxAge(uint32_t age) {
	return REST.set_header_max_age(response, age);
}

/*
 * Sets the ETag option of a response.
 */
int RestResponse::setETag(const uint8_t *etag, size_t length) {
	return REST.set_header_etag(response, etag, length);
}

/*
 * Sets the location option of a response.
 */
int RestResponse::setLocation(const char *location) {
	return REST.set_header_location(response, location);
}

int32_t RestResponse::getOffset() {
	return *offset;
}

void RestResponse::blockAppend(int32_t length) {
	*offset += length;
}

void RestResponse::blockComplete() {
	*offset = -1;
}

#if CONTIKI
#include <math.h>

size_t RestResponse::printFloat(double number, uint8_t digits) {
	size_t n = 0;

	if (isnan(number))
		return print("nan");
	if (isinf(number))
		return print("inf");
	if (number > 4294967040.0)
		return print("ovf");  // constant determined empirically
	if (number < -4294967040.0)
		return print("ovf");  // constant determined empirically

	// Handle negative numbers
	if (number < 0.0) {
		n += print('-');
		number = -number;
	}

	// Round correctly so that print(1.999, 2) prints as "2.00"
	double rounding = 0.5;
	for (uint8_t i = 0; i < digits; ++i)
		rounding /= 10.0;

	number += rounding;

	// Extract the integer part of the number and print it
	unsigned long int_part = (unsigned long) number;
	double remainder = number - (double) int_part;
	n += print(int_part);

	// Print the decimal point, but only if there are digits beyond
	if (digits > 0) {
		n += print(".");
	}

	// Extract digits from the remainder one at a time
	while (digits-- > 0) {
		remainder *= 10.0;
		int toPrint = int(remainder);
		n += print(toPrint);
		remainder -= toPrint;
	}

	return n;
}
#endif
