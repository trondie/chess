#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#if defined( PLATFORM_ARM_ANDROID )
#include <unistd.h>
#include <sys/time.h>
#endif
#if defined( PLATFORM_WINDOWS )
#include <windows.h>
#endif
#include "os_helpers.h"
#include "mymath.h"

char * new_os_url( const char *url )
{
	char *os_url = (char *)malloc(strlen(DATAPATH) + strlen(url) + 1);
	int i;
	strcpy( os_url, DATAPATH );
	strcpy( &os_url[strlen(DATAPATH)], url );
	os_url[ strlen(DATAPATH) + strlen(url) ] = 0;
	for ( i = 0; i < strlen( os_url ); i++ )
	{
#if defined( PLATFORM_ARM_ANDROID )
		if ( os_url[i] == '\\' ) os_url[i] = '/';
#elif defined( PLATFORM_WINDOWS )
		if ( os_url[i] == '/' ) os_url[i] = '\\';
#endif
	}
	return os_url;
}


unsigned long long get_timer_tick_resolution()
{
#if defined( PLATFORM_WINDOWS )
	return 1000ULL;
#else
	return 1000000ULL;
#endif
}

unsigned long long timer_get_time()
{
#if PLATFORM_WINDOWS
	return (unsigned long long)GetTickCount();
#else
	/* Currently using gettimeofday, but more precise counters could be added later */
	struct timeval tval;
	if ( 0 == gettimeofday(&tval,NULL) )
	{
		return tval.tv_sec*1000000 + tval.tv_usec;
	}
	return 0;
#endif
}

int mystrcmpi( const char *strA, const char *strB )
{
	int i = 0;
	int len = MIN( strlen( strA ), strlen( strB) );
	int diff = 0;
	for ( ;i < len; i++ )
	{
		if ( toupper( strA[i] ) != toupper( strB[i] ) ) diff++;
	}
	return diff;
}

