#ifndef __RINGBUF_H__
#define __RINGBUF_H__

#include <stdint.h>
#include <stdio.h>

typedef struct ringbuf_struct{
    int32_t bufsize;
    int32_t datalen;
    uint8_t *pbuf;
    uint8_t *pin;
    uint8_t *pout;
}ringbuf_t;

// Initialize a ring-buffer.
void ringbuf_init( ringbuf_t *ring, uint8_t *pbuf, int32_t size, uint8_t mode );

// Reset the ring-buffer, all data are dumped.
void ringbuf_reset( ringbuf_t *ring );

// Return the size of data. (Shout be greater than or equal to 0)
int32_t ringbuf_getdatalen( ringbuf_t *ring );

// Return the size of available space.
int32_t ringbuf_getfreespace( ringbuf_t *ring );

// Return: 
// 1 if 1 byte is written
// -1 if 1 byte is written and the earliest byte is override.
int32_t ringbuf_putbyte( ringbuf_t *ring, uint8_t byte );

// Return: 
// x if x byte is written
// 0 if nothing is written (insufficient free space)
// To calculate how many data have been truncated
// with (n - ringbuf_getfreespace()) 
int32_t ringbuf_putbyte_n( ringbuf_t *ring, uint8_t *src, int32_t n );

// Return:  
// 1 if 1 byte is read
// 0 if nothing is read
int32_t ringbuf_getbyte( ringbuf_t *ring, uint8_t *des );

// Return:  
// x if x byte is read (no enough data to get if x<n)
int32_t ringbuf_getbyte_n( ringbuf_t *ring, uint8_t *des, int32_t n );

#endif

