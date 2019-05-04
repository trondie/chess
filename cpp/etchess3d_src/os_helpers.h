#ifndef _OS_HELPERS_H_
#define _OS_HELPERS_H_

#if defined( PLATFORM_ARM_ANDROID )
#define APPDIR "/data/data/com.eskilsund.etchess3d.etchess3d/"
#define DATAPATH APPDIR "cache/"
#define PATHSTR( folder, file ) (DATAPATH "/"  file)
#define APPDIRSTR(file) (APPDIR "/" file)
#else
#define DATAPATH "../assets/"
#define PATHSTR( folder, file ) (DATAPATH "\\" file)
#endif

#if defined( PLATFORM_WINDOWS )
#ifndef snprintf
#define snprintf _snprintf
#endif
#define STRCMPI( strA, strB ) strcmpi( strA, strB )
#else
#define STRCMPI( strA, strB ) mystrcmpi( strA, strB )
#endif
char * new_os_url( const char *url );
unsigned long long get_timer_tick_resolution();
unsigned long long timer_get_time();
int mystrcmpi( const char *strA, const char *strB );

#endif /* _OS_HELPERS_H_ */