#include "ringbuf.h"

#define USE_ASSERT 1
#if USE_ASSERT
    #include <assert.h>
    #define ASSERT(exp) assert(exp)
#endif

// Reset the ring-buffer, all data are dumped.
void ringbuf_reset( ringbuf_t *r ){
    r->pin = r->pbuf;
    r->pout = r->pbuf;
    r->datalen = 0;
}

// Initialize a ring-buffer.
void ringbuf_init( ringbuf_t *r, uint8_t *pbuf, int32_t size, uint8_t mode ){
    ASSERT( r );
    ASSERT( pbuf );
    r->pbuf = pbuf;
    r->pout = pbuf;
    r->pin = pbuf;
    r->bufsize = size;
    r->datalen = 0;
}

// Return the size of data. (Shout be greater than or equal to 0)
int32_t ringbuf_getdatalen( ringbuf_t *r ){
    ASSERT( r );
    return r->datalen;
}

// Return the size of available space.
int32_t ringbuf_getfreespace( ringbuf_t *r ){
    ASSERT( r );
    return r->bufsize - r->datalen;
}

// Return: 
// 1 if 1 byte is written
// -1 if 1 byte is written and the earliest byte is override.
int32_t ringbuf_putbyte( ringbuf_t *r, uint8_t byte ){
    int32_t ret = 0;
    ASSERT( r );
    if( r->datalen >= r->bufsize ){
        ret = -2;   // -2+1 == -1
        r->datalen --;
    }
    *r->pin = byte;
    if( ++r->pin >= (r->pbuf + r->bufsize) ){
        r->pin = r->pbuf;   // Warp to the head of buffer.
    }
    r->datalen ++;
    return ret+1;
}

// Return: 
// x if x byte is written
// 0 if nothing is written (insufficient free space)
// To calculate how many data have been truncated
// with (n - ringbuf_getfreespace()) 
int32_t ringbuf_putbyte_n( ringbuf_t *r, uint8_t *src, int32_t n ){
    int32_t cnt = n;
    ASSERT( r );
    ASSERT( src );
    while( cnt-- ){
        ringbuf_putbyte( r, *(src++) );
    }
    return n;
}

// Return:  
// 1 if 1 byte is read
// 0 if nothing is read
int32_t ringbuf_getbyte( ringbuf_t *r, uint8_t *des ){
    ASSERT( r );
    ASSERT( des );
    if( r->datalen < 1 ){
        return 0; // Empty buffer.
    }
    *des = *r->pout;
    if( --r->pout < r->pbuf ){
        r->pout = r->pbuf + r->bufsize - 1; // Warp to the tail of buffer.
    }
    r->datalen --;
    return 1;
}

// Return:  
// x if x byte is read (no enough data to get if x<n)
int32_t ringbuf_getbyte_n( ringbuf_t *r, uint8_t *des, int32_t n ){
    int32_t remain = n;
    ASSERT( r );
    ASSERT( des );
    while( remain-- ){
        if( !ringbuf_getbyte(r, des++) ){
            break;
        }
    }
    return n-remain;
}


// Return:  
// x if x byte is read
int32_t ringbuf_getbyte_all( ringbuf_t *r, uint8_t *des ){
    int32_t cnt=0, remain=r->datalen;
    ASSERT( r );
    ASSERT( des );
    while( remain-- ){
        ringbuf_getbyte(r, &des[cnt++]);
    }
    return cnt;
}




