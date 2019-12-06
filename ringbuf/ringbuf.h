#ifndef __RINGBUF_H__
#define __RINGBUF_H__

#include <stdint.h>
#include <stdio.h>

typedef struct ringbuf_struct{
    int32_t bufsize;
    int32_t datalen;
    void *pbuf;
    void *pin;
    void *pout;
}ringbuf_t;

// Initialize a ring-buffer.
void ringbuf_init( ringbuf_t *ring, void *pbuf, int32_t size );

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
int32_t ringbuf_putbyte_n( ringbuf_t *ring, void *src, int32_t n );

// Return:  
// 1 if 1 byte is read
// 0 if nothing is read
int32_t ringbuf_getbyte( ringbuf_t *ring, void *des );

// Return:  
// x if x byte is read (no enough data to get if x<n)
int32_t ringbuf_getbyte_n( ringbuf_t *ring, void *des, int32_t n );

#endif

