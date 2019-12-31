#include "ringbuf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

ringbuf_t ring;
#define BUFSIZE (10)


void print_ringbuf_param( ringbuf_t *r ){
    int32_t cnt;
    printf( "r          =\t%p\r\n", r );
    printf( "r->pbuf    =\t%p\r\n", r->pbuf );
	printf( "r->bufsize =\t%d\r\n", r->bufsize );
    printf( "r->nextin  =\t%d\r\n", r->nextin );
    printf( "r->nextout =\t%d\r\n", r->nextout );
    printf( "r->datalen =\t%d\r\n", r->datalen );
    printf( "Buf data:\r\n\t" );
    for( cnt=0; cnt<r->bufsize; cnt++ ){
        if( cnt && !(cnt % 16) ){
            printf( "\r\n\t" );
        }
        printf( "%02X ", 0xFF & ((uint8_t *)r->pbuf)[cnt] );
    }
    printf( "\r\n--------------------------------------------------------\r\n" );
}


#if RINGBUF_USE_MALLOC
static inline void ringbuf_test_malloc( void ){
	char buf[BUFSIZE];
    char tmpbuf[ 20 ] = {0};
    uint8_t tmpByte;
	int32_t cnt;
    printf( "\r\n\n\n================== test ringbuf with malloc ====================\r\n" );
	ringbuf_t *mring;
	ringbuf_t *mring2;
	mring = ringbuf_new( 128 );
	if( !mring ){
		return;
	}
	printf( "ringbuf_new()\r\n" );
	print_ringbuf_param( mring ); 

	printf( "put 10 bytes in ringbuf, ret=%d\r\n", ringbuf_putbyte_n( mring, "0123456789", 10 ));
	printf( "ringbuf_getdatalen() = %d\r\n", ringbuf_getdatalen( mring ) ); 
	printf( "ringbuf_getfreespace() = %d\r\n", ringbuf_getfreespace( mring ) ); 
	print_ringbuf_param( mring ); 

	printf( "put 1 byte 0x71 in ringbuf, ret=%d\r\n", ringbuf_putbyte( mring, 0x71 ));
	printf( "ringbuf_getdatalen() = %d\r\n", ringbuf_getdatalen( mring ) ); 
	printf( "ringbuf_getfreespace() = %d\r\n", ringbuf_getfreespace( mring ) ); 
	print_ringbuf_param( mring ); 

	printf( "get 12 bytes from ringbuf, ret=%d, result:\r\n\t", ringbuf_getbyte_n( mring, &tmpbuf, 12 ));
	for( cnt=0; cnt<sizeof(tmpbuf); cnt++ ){
		if( cnt && !cnt % 16 ){
			printf( "\r\n\t" );
		}
        printf( "%02X ", 0xFF & tmpbuf[cnt] );
	}
	printf( "\r\n" );
	printf( "ringbuf_getdatalen() = %d\r\n", ringbuf_getdatalen( mring ) ); 
	printf( "ringbuf_getfreespace() = %d\r\n", ringbuf_getfreespace( mring ) ); 
	print_ringbuf_param( mring ); 
	memset( tmpbuf, 0, sizeof(tmpbuf) );

	printf( "ringbuf_free()\r\n" );
	ringbuf_free( mring );	
	print_ringbuf_param( mring ); 

	mring = ringbuf_new( 128 );
	if( !mring ){
		return;
	}
	printf( "ringbuf_new()\r\n" );
	print_ringbuf_param( mring ); 

	mring2 = ringbuf_new( 64 );
	if( !mring ){
		return;
	}
	printf( "ringbuf_new()\r\n" );
	print_ringbuf_param( mring2 ); 

	ringbuf_free( mring );
	ringbuf_free( mring2 );


}
#endif


#if RINGBUF_USE_MUTEX

void *thread_routine_1( void *arg ){
	struct timespec ts;
	clock_gettime( CLOCK_PROCESS_CPUTIME_ID, &ts );
	printf( "\r\n[%lu]--------- Thread 1 start ---------\r\n", ts.tv_nsec );	
	sleep(2);
	clock_gettime( CLOCK_PROCESS_CPUTIME_ID, &ts );
	printf( "\r\n[%lu]--------- Thread 1 exit ----------\r\n", ts.tv_nsec );	
	return NULL;
}

void *thread_routine_2( void *arg ){
	struct timespec ts;
	clock_gettime( CLOCK_PROCESS_CPUTIME_ID, &ts );
	printf( "\r\n[%lu]--------- Thread 2 start ---------\r\n", ts.tv_nsec );	
	sleep(1);
	clock_gettime( CLOCK_PROCESS_CPUTIME_ID, &ts );
	printf( "\r\n[%lu]--------- Thread 2 exit ----------\r\n", ts.tv_nsec );	
	return NULL;
}

static inline void ringbuf_test_mutex( void ){
	char *buf = malloc( BUFSIZE ); 
    char tmpbuf[ 20 ] = {0};
    uint8_t tmpByte;
	pthread_t thread_1, thread_2; 
	pthread_attr_t thread_attr_1, thread_attr_2;
	int ret; 
	int cnt;
	struct timespec ts;

    printf( "\r\n\n\n================== test ringbuf with mutex ====================\r\n" );
    if( !buf ){
        printf( "Failed to malloc\r\n" );
        return;
    }

	printf( "init ringbuf\r\n" );
	ringbuf_init( &ring, buf, BUFSIZE ); 
	printf( "ringbuf_getdatalen() = %d\r\n", ringbuf_getdatalen( &ring ) ); 
	printf( "ringbuf_getfreespace() = %d\r\n", ringbuf_getfreespace( &ring ) ); 
	print_ringbuf_param( &ring );  

	// Thread 1
	ret = pthread_attr_init( &thread_attr_1 );	
	if( ret != 0 ){
		printf( "Failed to init thread_attr_1\r\n" );
	}
	ret = pthread_attr_setdetachstate( &thread_attr_1, PTHREAD_CREATE_DETACHED );	
	if( ret != 0 ){
		printf( "Failed to set thread_attr_1 detach\r\n" );
	}
	clock_gettime( CLOCK_PROCESS_CPUTIME_ID, &ts );
	printf( "[%lu] Creat thread 1\r\n", ts.tv_nsec );
	ret = pthread_create( &thread_1, &thread_attr_1, thread_routine_1, NULL );
	if( ret != 0 ){
		printf( "Failed to create thread 1\r\n" );
	}

	usleep( 1 ); // Switch to run Thread 1 first

	// Thread 2
	ret = pthread_attr_init( &thread_attr_2 );	
	if( ret != 0 ){
		printf( "Failed to init thread_attr_2\r\n" );
	}
	ret = pthread_attr_setdetachstate( &thread_attr_2, PTHREAD_CREATE_DETACHED );	
	if( ret != 0 ){
		printf( "Failed to set thread_attr_2 detach\r\n" );
	}
	clock_gettime( CLOCK_PROCESS_CPUTIME_ID, &ts );
	printf( "[%lu] Creat thread 2\r\n", ts.tv_nsec );
	ret = pthread_create( &thread_2, &thread_attr_2, thread_routine_2, NULL );
	if( ret != 0 ){
		printf( "Failed to create thread 2\r\n" );
	}

// 	ret = pthread_join( thread_1, NULL );
// 	if( ret != 0 ){
//         printf( "Failed to join thread 1\r\n" );
// 	}
//
// 	ret = pthread_join( thread_2, NULL );
// 	if( ret != 0 ){
// 		printf( "Failed to join thread 2\r\n" );
// 	}

	sleep( 4 );

	free( buf );
}
#endif


static inline void ringbuf_test_static_assign( void ){
	char buf[BUFSIZE] = {0};
    char tmpbuf[ 20 ] = {0};
    uint8_t tmpByte;
    int cnt;
    if( !buf ){
        printf( "Failed to malloc\r\n" );
        return;
    }
    printf( "init ringbuf\r\n" );
    ringbuf_init( &ring, buf, BUFSIZE ); 
    printf( "ringbuf_getdatalen() = %d\r\n", ringbuf_getdatalen( &ring ) ); 
    printf( "ringbuf_getfreespace() = %d\r\n", ringbuf_getfreespace( &ring ) ); 
    print_ringbuf_param( &ring ); 
    
    printf( "Put 1 byte 0xA5 in ringbuf, ret=%d\r\n", ringbuf_putbyte( &ring, 0xA5 ) );
    printf( "ringbuf_getdatalen() = %d\r\n", ringbuf_getdatalen( &ring ) ); 
    printf( "ringbuf_getfreespace() = %d\r\n", ringbuf_getfreespace( &ring ) ); 
    print_ringbuf_param( &ring ); 

    printf( "Get 1 byte from ringbuf, ret=%d", ringbuf_getbyte( &ring, &tmpByte ) );
    printf( ", result=0x%02X\r\n", tmpByte );
    printf( "ringbuf_getdatalen() = %d\r\n", ringbuf_getdatalen( &ring ) ); 
    printf( "ringbuf_getfreespace() = %d\r\n", ringbuf_getfreespace( &ring ) ); 
    print_ringbuf_param( &ring ); 

    printf( "put 10 bytes \"0123456789\" in ringbuf, ret=%d\r\n", ringbuf_putbyte_n( &ring, "0123456789", 10 ));
    printf( "ringbuf_getdatalen() = %d\r\n", ringbuf_getdatalen( &ring ) ); 
    printf( "ringbuf_getfreespace() = %d\r\n", ringbuf_getfreespace( &ring ) ); 
    print_ringbuf_param( &ring ); 

	printf( "put 1 byte 0x81 in ringbuf, ret=%d\r\n", ringbuf_putbyte( &ring, 0x81 ));
    printf( "ringbuf_getdatalen() = %d\r\n", ringbuf_getdatalen( &ring ) ); 
    printf( "ringbuf_getfreespace() = %d\r\n", ringbuf_getfreespace( &ring ) ); 
    print_ringbuf_param( &ring ); 

    printf( "get 12 bytes from ringbuf, ret=%d, result:\r\n\t", ringbuf_getbyte_n( &ring, &tmpbuf, 12 ));
    for( cnt=0; cnt<sizeof(tmpbuf); cnt++ ){
        if( cnt && !cnt % 16 ){
            printf( "\r\n\t" );
        }
        printf( "%02X ", 0xFF & tmpbuf[cnt] );
    }
    printf( "\r\n" );
    printf( "ringbuf_getdatalen() = %d\r\n", ringbuf_getdatalen( &ring ) ); 
    printf( "ringbuf_getfreespace() = %d\r\n", ringbuf_getfreespace( &ring ) ); 
    print_ringbuf_param( &ring ); 
    memset( tmpbuf, 0, sizeof(tmpbuf) );

	printf( "put 1 byte 0x00 in ringbuf, ret=%d\r\n", ringbuf_putbyte( &ring, 0x00 ));
	printf( "ringbuf_getdatalen() = %d\r\n", ringbuf_getdatalen( &ring ) ); 
	printf( "ringbuf_getfreespace() = %d\r\n", ringbuf_getfreespace( &ring ) ); 
	print_ringbuf_param( &ring ); 

    printf( "put 2 bytes \"AB\" in ringbuf, ret=%d\r\n", ringbuf_putbyte_n( &ring, "AB", 2 ));
    printf( "ringbuf_getdatalen() = %d\r\n", ringbuf_getdatalen( &ring ) ); 
    printf( "ringbuf_getfreespace() = %d\r\n", ringbuf_getfreespace( &ring ) ); 
    print_ringbuf_param( &ring ); 
 
    printf( "get 99999999 bytes from ringbuf, ret=%d, result:\r\n\t", ringbuf_getbyte_n( &ring, &tmpbuf, 99999999 ));
    for( cnt=0; cnt<sizeof(tmpbuf); cnt++ ){
        if( cnt && !cnt % 16 ){
            printf( "\r\n\t" );
        }
        printf( "%02X ", 0xFF & tmpbuf[cnt] );
    }   
    printf( "\r\n" );
    printf( "ringbuf_getdatalen() = %d\r\n", ringbuf_getdatalen( &ring ) ); 
    printf( "ringbuf_getfreespace() = %d\r\n", ringbuf_getfreespace( &ring ) ); 
    print_ringbuf_param( &ring ); 
    memset( tmpbuf, 0, sizeof(tmpbuf) );
 
    printf( "put 2 bytes \"AB\" in ringbuf, ret=%d\r\n", ringbuf_putbyte_n( &ring, "AB", 2 ));
    printf( "ringbuf_getdatalen() = %d\r\n", ringbuf_getdatalen( &ring ) ); 
    printf( "ringbuf_getfreespace() = %d\r\n", ringbuf_getfreespace( &ring ) ); 
    print_ringbuf_param( &ring ); 
 
    printf( "get -1 bytes from ringbuf, ret=%d, result:\r\n\t", ringbuf_getbyte_n( &ring, &tmpbuf, -1 ));
    for( cnt=0; cnt<sizeof(tmpbuf); cnt++ ){
        if( cnt && !cnt % 16 ){
            printf( "\r\n\t" );
        }
        printf( "%02X ", 0xFF & tmpbuf[cnt] );
    }   
    printf( "\r\n" );
    printf( "ringbuf_getdatalen() = %d\r\n", ringbuf_getdatalen( &ring ) ); 
    printf( "ringbuf_getfreespace() = %d\r\n", ringbuf_getfreespace( &ring ) ); 
    print_ringbuf_param( &ring ); 
    memset( tmpbuf, 0, sizeof(tmpbuf) );	
}



int main( void **argv, int argc ){
	ringbuf_test_static_assign();
#if RINGBUF_USE_MALLOC
	ringbuf_test_malloc();
#endif
#if RINGBUF_USE_MUTEX
	ringbuf_test_mutex();
#endif
	printf( "\r\n======================== The end of testing ========================\r\n" );
    return 0;
} 
