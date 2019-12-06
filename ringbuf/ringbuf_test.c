#include "ringbuf.h"
#include <stdio.h>
#include <stdlib.h>

ringbuf_t ring;
#define BUFSIZE (10)

void print_ringbuf_param( ringbuf_t *r ){
    int32_t cnt;
    printf( "r=\t\t%p\r\n", r );
    printf( "r->pbuf=\t%p\r\n", r->pbuf );
    printf( "r->pin=\t\t%p\r\n", r->pin );
    printf( "r->pout=\t%p\r\n", r->pout );
    printf( "r->bufsize=\t%d\r\n", r->bufsize );
    printf( "r->datalen=\t%d\r\n", r->datalen );
    printf( "Buf data:\r\n\t" );
    for( cnt=0; cnt<r->bufsize; cnt++ ){
        if( cnt && !cnt % 16 ){
            printf( "\r\n\t" );
        }
        printf( "%02X ", ((uint8_t *)r->pbuf)[cnt] );
    }
    printf( "\r\n-----------------------------------------------------\r\n" );
}


int main( void **argv, int argc ){
    char *buf = malloc( BUFSIZE ); 
    char tmpbuf[ 20 ] = {0};
    uint8_t tmpByte;
    int cnt;
    if( !buf ){
        printf( "Failed to malloc\r\n" );
        return 0;
    }
    printf( "init ringbuf\r\n" );
    ringbuf_init( &ring, buf, BUFSIZE ); 
    print_ringbuf_param( &ring ); 
    
    printf( "Put 1 byte in ringbuf, ret=%d\r\n", ringbuf_putbyte( &ring, 0xA5 ) );
    print_ringbuf_param( &ring ); 

    printf( "Get 1 byte from ringbuf, ret=%d", ringbuf_getbyte( &ring, &tmpByte ) );
    printf( ", result=0x%02X\r\n", tmpByte );
    print_ringbuf_param( &ring ); 

    printf( "put 10 byte in ringbuf, ret=%d\r\n", ringbuf_putbyte_n( &ring, "0123456789", 10 ));
    print_ringbuf_param( &ring ); 

    printf( "get 11 byte from ringbuf, ret=%d, result:\r\n\t", ringbuf_getbyte_n( &ring, &tmpbuf, 11 ));
    for( cnt=0; cnt<sizeof(tmpbuf); cnt++ ){
        if( cnt && !cnt % 16 ){
            printf( "\r\n\t" );
        }
        printf( "%02X ", tmpbuf[cnt] );
    }
    printf( "\r\n" );
    print_ringbuf_param( &ring ); 

    // TODO: Multithread

    return 0;
}





