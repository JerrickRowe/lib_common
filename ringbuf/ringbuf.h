#ifndef __RINGBUF_H__
#define __RINGBUF_H__

#include <stdint.h>
#include <stdio.h>

#define RINGBUF_USE_MUTEX	1
#if RINGBUF_USE_MUTEX	
	#define RINGBUF_USE_COND	1
#else
	#define RINGBUF_USE_COND	0
#endif

#if RINGBUF_USE_MUTEX 
	#include <pthread.h> 
	typedef struct{
		pthread_mutex_t mutex;
		pthread_cond_t datain;
		pthread_cond_t dataout;
	}MUTEX_TYPE;
#endif

typedef struct ringbuf_struct{
	void *pbuf;
    int32_t bufsize;
    int32_t nextin;
    int32_t nextout;
	int32_t datalen;
#if RINGBUF_USE_MUTEX 
	MUTEX_TYPE mutex_obj;
#endif
}ringbuf_t;

#define RINGBUF_USE_MALLOC 1
#if RINGBUF_USE_MALLOC
    // Alloc a new ringbuf
    ringbuf_t *ringbuf_new( int32_t bufsize );
    // Free up memmory
    void ringbuf_free( ringbuf_t *ring );
#endif

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

