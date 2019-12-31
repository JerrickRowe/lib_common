#include "ringbuf.h"

#define USE_ASSERT 1
#if USE_ASSERT
    #include <assert.h>
    #define ASSERT(exp) assert(exp)
#else
    #define ASSERT(exp) 
#endif

//--------------- Mutex wrappers --------------------------

#if RINGBUF_USE_COND

	#define MUTEX_COND_INIT( r ) do{\
		ASSERT( r );\
		pthread_cond_init( &((ringbuf_t *)r)->mutex_obj.datain, NULL );\
		pthread_cond_init( &((ringbuf_t *)r)->mutex_obj.dataout, NULL );\
	}while(0)

	#define MUTEX_COND_DATAIN_WAIT( r ) do{\
		ASSERT( r );\
		pthread_cond_wait( &((ringbuf_t *)r)->mutex_obj.datain, &((ringbuf_t *)r)->mutex_obj.mutex );\
	}while(0)

	#define MUTEX_COND_DATAOUT_WAIT( r ) do{\
		ASSERT( r );\
		pthread_cond_wait( &((ringbuf_t *)r)->mutex_obj.dataout, &((ringbuf_t *)r)->mutex_obj.mutex );\
	}while(0)

	#define MUTEX_COND_DATAIN( r ) do{\
		ASSERT( r );\
		pthread_cond_signal( &((ringbuf_t *)r)->mutex_obj.datain );\
	}while(0)

	#define MUTEX_COND_DATAOUT( r ) do{\
		ASSERT( r );\
		pthread_cond_signal( &((ringbuf_t *)r)->mutex_obj.dataout );\
	}while(0)

#else
	#define MUTEX_COND_INIT( r )
	#define MUTEX_COND_DATAIN_WAIT( r )
	#define MUTEX_COND_DATAOUT_WAIT( r )
	#define MUTEX_COND_DATAIN( r )
	#define MUTEX_COND_DATAOUT( r )
#endif


#if RINGBUF_USE_MUTEX

	#define MUTEX_INIT( r ) do{\
		ASSERT( r );\
		pthread_mutex_init( &((ringbuf_t *)r)->mutex_obj.mutex, NULL );\
		MUTEX_COND_INIT( r );\
	}while(0)

	#define MUTEX_LOCK( r )		do{\
		ASSERT( r );\
		pthread_mutex_lock( &((ringbuf_t *)r)->mutex_obj.mutex );\
	}while(0)

	#define MUTEX_UNLOCK( r )	do{\
		ASSERT( r );\
		pthread_mutex_unlock( &((ringbuf_t *)r)->mutex_obj.mutex );\
	}while(0)	

#else
	#define MUTEX_LOCK( r )	
	#define MUTEX_UNLOCK( r )	
#endif

#define CONCURRENCY_TEST	1
#if CONCURRENCY_TEST	

#include <unistd.h>
#define TEST_DELAY() do{\
	usleep( 2000 );\
}while(0)

#else
	#define TEST_DELAY()		
#endif

//---------------------------------------------------------

// Reset the ring-buffer, all data are dumped.
void ringbuf_reset( ringbuf_t *r ){
	MUTEX_LOCK( r );
    ASSERT( r );
	r->nextin = 0; 
	r->nextout = 0; 
	TEST_DELAY(); 
	r->datalen = 0;
	MUTEX_UNLOCK( r );
}

// Initialize a ring-buffer.
void ringbuf_init( ringbuf_t *r, void *pbuf, int32_t size ){
	ASSERT( r );
	ASSERT( pbuf );
	MUTEX_INIT( r );
	MUTEX_LOCK( r );
	r->datalen = 0;
	r->nextout = 0; 
	r->nextin = 0; 
	TEST_DELAY(); 
	r->bufsize = size;
	r->pbuf = pbuf;
	MUTEX_UNLOCK( r );
}

// Return the size of data. (Shout be greater than or equal to 0)
int32_t ringbuf_getdatalen( ringbuf_t *r ){
	int32_t ret = 0;
	ASSERT( r );
	MUTEX_LOCK( r );
	ret = r->datalen;
	MUTEX_UNLOCK( r );
	return ret;
}

// Return the size of available space.
int32_t ringbuf_getfreespace( ringbuf_t *r ){
	int32_t ret = 0;
	ASSERT( r );
	MUTEX_LOCK( r );
	ret = r->bufsize - r->datalen;
	MUTEX_UNLOCK( r );
	return ret;
}

// Return: 
// 1 if 1 byte is written
// -1 if 1 byte is written and the earliest byte is override.
int32_t ringbuf_putbyte( ringbuf_t *r, uint8_t byte ){
	int32_t ret = 0;
	ASSERT( r );
	MUTEX_LOCK( r );
	if( r->datalen >= r->bufsize ){
		// override old data when full
		ret = -2;   // -2+1 == -1
		r->datalen --;
		r->nextout ++;
		r->nextout %= r->bufsize; // Wrap to the head of buffer.
	}
	TEST_DELAY(); 
	((uint8_t *)r->pbuf)[r->nextin++] = byte;
	r->nextin %= r->bufsize;	// Wrap to the head of buffer.
	r->datalen ++;
	MUTEX_UNLOCK( r );
	return ret+1;
}

// Return: 
// Must be 1. function will be blocked until buffer space avaiable.
int32_t ringbuf_putbyte_block( ringbuf_t *r, uint8_t byte ){
	ASSERT( r );
	MUTEX_LOCK( r );
	if( r->datalen >= r->bufsize ){
		MUTEX_COND_DATAOUT_WAIT( r );
	}
	((uint8_t *)r->pbuf)[r->nextin++] = byte;
	TEST_DELAY(); 
	r->nextin %= r->bufsize;	// Wrap to the head of buffer.
	r->datalen ++;
	MUTEX_COND_DATAIN( r );
	MUTEX_UNLOCK( r );
	return 1;
}

// Return: 
// x if x byte is written
// 0 if nothing is written (insufficient free space)
// To calculate how many data have been truncated
// with (n - ringbuf_getfreespace()) 
int32_t ringbuf_putbyte_n( ringbuf_t *r, void *src, int32_t n ){
	int32_t cnt = n;
	ASSERT( r );
	ASSERT( src );
	// Wait until there is enough space
	// while( (r->datalen + n) >= r->bufsize );
	MUTEX_LOCK( r );
	while( cnt-- > 0 ){
		if( r->datalen >= r->bufsize ){
			// override old data when full
			r->datalen --;
			r->nextout ++;
			r->nextout %= r->bufsize;	// Wrap to the head of buffer.
		}
		((uint8_t *)r->pbuf)[r->nextin++] =  *((uint8_t *)src++);
		r->nextin %= r->bufsize;	// Wrap to the head of buffer.
		TEST_DELAY(); 
		r->datalen ++;
	}
	MUTEX_UNLOCK( r );
	return n;
}

// Return: 
// Must be n, function will be blocked until there are n bytes space available.
int32_t ringbuf_putbyte_n_block( ringbuf_t *r, void *src, int32_t n ){
	int32_t cnt = n;
	ASSERT( r );
	ASSERT( src );
	MUTEX_LOCK( r );
	while( r->datalen+n > r->bufsize ){
		MUTEX_COND_DATAOUT_WAIT( r );
	}
	while( cnt-- > 0 ){
		((uint8_t *)r->pbuf)[r->nextin++] =  *((uint8_t *)src++);
		r->nextin %= r->bufsize;	// Wrap to the head of buffer.
		TEST_DELAY(); 
		r->datalen ++;
	}
	MUTEX_COND_DATAIN( r );
	MUTEX_UNLOCK( r );
	return n;
}

// Return:  
// 1 if 1 byte is read
// 0 if nothing is read
int32_t ringbuf_getbyte( ringbuf_t *r, void *des ){
	ASSERT( r );
	ASSERT( des );
	MUTEX_LOCK( r );
	if( r->datalen < 1 ){
		return 0; // Empty buffer.
	}
	*(uint8_t *)des = ((uint8_t *)r->pbuf)[r->nextout++];
	r->nextout %= r->bufsize;	// Wrap to the head of buffer.
	r->datalen --;
	MUTEX_UNLOCK( r );
	return 1;
}

// Return:  
// Must be 1, block if buffer is empty
int32_t ringbuf_getbyte_block( ringbuf_t *r, void *des ){
	ASSERT( r );
	ASSERT( des );
	MUTEX_LOCK( r );
	if( r->datalen < 1 ){
		MUTEX_COND_DATAIN_WAIT( r );
	}
	*(uint8_t *)des = ((uint8_t *)r->pbuf)[r->nextout++];
	r->nextout %= r->bufsize;	// Wrap to the head of buffer.
	r->datalen --;
	MUTEX_COND_DATAOUT( r );
	MUTEX_UNLOCK( r );
	return 1;
}

// Return:  
// x if x byte is read (no enough data to get if x<n)
int32_t ringbuf_getbyte_n( ringbuf_t *r, void *des, int32_t n ){
	int32_t cnt = 0;
	ASSERT( r );
	ASSERT( des );
	MUTEX_LOCK( r );
	while( r->datalen>0 && cnt<n ){
		*(uint8_t *)(des++) = ((uint8_t *)r->pbuf)[r->nextout++];
		r->nextout %= r->bufsize;	// Wrap to the head of buffer.
		r->datalen --;
		cnt ++;
	}
	MUTEX_UNLOCK( r );
	return cnt;
}

// Return:  
// Must be n. block if no enough data
int32_t ringbuf_getbyte_n_block( ringbuf_t *r, void *des, int32_t n ){
	int32_t cnt = 0;
	ASSERT( r );
	ASSERT( des );
	MUTEX_LOCK( r );
	while( cnt<n ){
		if( r->datalen < 1 ){
			MUTEX_COND_DATAIN_WAIT( r );
		}
		*(uint8_t *)(des++) = ((uint8_t *)r->pbuf)[r->nextout++];
		r->nextout %= r->bufsize;	// Wrap to the head of buffer.
		r->datalen --;
		cnt ++;
		MUTEX_COND_DATAOUT( r );
	}
	MUTEX_UNLOCK( r );
	return cnt;
}

// Return:  
// x if x byte is read
int32_t ringbuf_getbyte_all( ringbuf_t *r, void *des ){
	int32_t cnt=0;
	ASSERT( r );
	ASSERT( des );
	MUTEX_LOCK( r );
	while( r->datalen>0 ){
		*(uint8_t *)des = ((uint8_t *)r->pbuf)[r->nextout++];
		r->nextout %= r->bufsize;	// Wrap to the head of buffer.
		r->datalen --;
		cnt ++;
	}
	MUTEX_UNLOCK( r );
	return cnt;
}


#if  RINGBUF_USE_MALLOC
#include <stdlib.h>
#define MALLOC_WRAPPER(x) malloc(x)
#define FREE_WRAPPER(x) free(x)

ringbuf_t *ringbuf_new( int32_t bufsize ){
	ringbuf_t *pring;  
	uint8_t *pbuf;
	if( bufsize <= 0 ){
		return NULL;
	}
	pbuf = MALLOC_WRAPPER( bufsize );
	if( !pbuf ){
		return NULL;
	}
	pring = MALLOC_WRAPPER( sizeof(ringbuf_t) );
	if( !pring ){
		FREE_WRAPPER( pbuf );
		return NULL;
	}
	ringbuf_init( pring, pbuf, bufsize ); 
	return pring;
}

void ringbuf_free( ringbuf_t *r ){
	ASSERT( r );
	if( r->pbuf ){
		r->pbuf = NULL;
		r->nextin  = 0;
		r->nextout = 0;
		r->bufsize = 0;
		r->datalen = 0;
		FREE_WRAPPER( r->pbuf );
		FREE_WRAPPER( r );
	}
}

#endif




