#ifndef _WINCE_COMPATE_H_
#define _WINCE_COMPATE_H_

#include "Fl_Platform.h"

#if __FLTK_WINCE__
#include <stdio.h>
#include <time.h>
#include <Windows.h>

// fcntl.h
#define F_SETFL         1
#define F_SETFD         2
#define F_GETFL         3

#define _O_RDONLY       0x0000  /* open for reading only */
#define _O_WRONLY       0x0001  /* open for writing only */
#define _O_RDWR         0x0002  /* open for reading and writing */

#define _O_NONBLOCK     0x0004

#define _O_APPEND       0x0008  /* writes done at eof */
#define _O_CREAT        0x0100  /* create and open file */
#define _O_TRUNC        0x0200  /* open and truncate */
#define _O_EXCL         0x0400  /* open only if file doesn't already exist */
#define _O_TEXT         0x4000  /* file mode is text (translated) */
#define _O_BINARY       0x8000  /* file mode is binary (untranslated) */
#define _O_ACCMODE      0x10000

#define _O_NOINHERIT    0
#define O_NOINHERIT     _O_NOINHERIT

#define O_RDONLY        _O_RDONLY
#define O_WRONLY        _O_WRONLY
#define O_RDWR          _O_RDWR

#define O_NONBLOCK      _O_NONBLOCK

#define O_APPEND        _O_APPEND
#define O_CREAT         _O_CREAT
#define O_TRUNC         _O_TRUNC
#define O_EXCL          _O_EXCL
#define O_TEXT          _O_TEXT
#define O_BINARY        _O_BINARY
#define O_ACCMODE       _O_ACCMODE

// errno.h
#define EPERM           1
#define ENOENT          2
#define ESRCH           3
#define EINTR           4
#define EIO             5
#define ENXIO           6
#define E2BIG           7
#define ENOEXEC         8
#define EBADF           9
#define ECHILD          10
#define EAGAIN          11
#define ENOMEM          12
#define EACCES          13
#define EFAULT          14
#define EOSERR          15 // rk
#define EBUSY           16
#define EEXIST          17
#define EXDEV           18
#define ENODEV          19
#define ENOTDIR         20
#define EISDIR          21
#define EINVAL          22
#define ENFILE          23
#define EMFILE          24
#define ENOTTY          25
#define EFBIG           27
#define ENOSPC          28
#define ESPIPE          29
#define EROFS           30
#define EMLINK          31
#define EPIPE           32
#define EDOM            33
#define ERANGE          34
#define EDEADLK         36
#define ENOSYS          37

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _ERRNO_T_DEFINED
#define _ERRNO_T_DEFINED
	typedef int errno_t;
#endif /* _ERRNO_T_DEFINED */

	extern int errno;
	extern int _doserrno;
	extern int _sys_nerr;

#define sys_nerr _sys_nerr

#ifdef __cplusplus
};
#endif

// signal.h
#define SIGHUP		1
#define SIGINT      2
#define SIGQUIT		3
#define SIGILL      4
#define SIGPIPE     5
#define SIGFPE      8
#define SIGUSR1		10
#define SIGSEGV     11
#define SIGUSR2		12
#define SIGTERM     15
#define SIGCHLD		17
#define SIGTSTP		20
#define SIGBREAK    21
#define SIGABRT     22
#define NSIG        22

#define SA_NOCLDSTOP	1
#define SA_SHIRQ	0x04000000
#define SA_STACK	0x08000000
#define SA_RESTART	0x10000000
#define SA_INTERRUPT	0x20000000
#define SA_NOMASK	0x40000000
#define SA_ONESHOT	0x80000000

/* signal action codes */

#define SIG_DFL (void (*)(int))0           /* default signal action */
#define SIG_IGN (void (*)(int))1           /* ignore signal */
#define SIG_SGE (void (*)(int))3           /* signal gets error */
#define SIG_ACK (void (*)(int))4           /* acknowledge */
#define SIG_ERR (void (*)(int))-1          /* signal error value */

#define SIG_BLOCK          0	/* for blocking signals */
#define SIG_UNBLOCK        1	/* for unblocking signals */
#define SIG_SETMASK        2	/* for setting the signal mask */

#ifdef __cplusplus
extern "C" {
#endif

typedef void (* SIGHANDLER)(int);
typedef void (* sighandler_t)(int);

typedef int sig_atomic_t;
typedef unsigned int sigset_t;

struct sigaction{
	sighandler_t sa_handler;
	sigset_t sa_mask;
	unsigned long sa_flags;
	void (*sa_restorer)(void);
};

int raise(int sig);

#ifdef __cplusplus
};
#endif

// wince.h
wchar_t* wce_mbtowc(const char* a);
char* wce_wctomb(const wchar_t* w);

// stdio.h
#define _TRUNCATE 0

#ifdef __cplusplus
extern "C" {
#endif
//FILE *freopen(const char *filename, const char *mode, FILE *file);
//FILE *fdopen( int handle, const char *mode );
errno_t fopen_s(FILE ** file, const char * filename, const char * mode);
int sprintf_s(char *buffer, size_t sizeOfBuffer, const char *format, ... );
int vsnprintf_s(char *buffer, size_t sizeOfBuffer, size_t count, const char *format, va_list argptr );
int _vscprintf(const char *format, va_list argptr);
int _wopen(const wchar_t *filename, int oflag);
#ifdef __cplusplus
};
#endif

// io.h
int _chsize(int handle, long size);
int _rename (const char *oldname, const char *newname);
int _unlink(const char *file);
int _umask(int cmask);
int _chmod(const char *path, int mode);
int dup( int handle );
//int dup2( int handle1, int handle2 );
int _isatty(int fd);
int _pipe(int *phandles, unsigned int psize, int textmode);
int _access(const char *filename, int flags);
int _open_osfhandle ( long osfhandle, int flags);
long _get_osfhandle( int filehandle );
int wince_open(const char *file, int mode,...);
int close(int fd);
int _read(int fd, void *buffer, int length);
int _write(int fd, const void *buffer, unsigned count);
long _lseek(int handle, long offset, int origin);
/*
#undef WIN32_FIND_DATA
#define WIN32_FIND_DATA WIN32_FIND_DATAA

#ifndef _TIME_T_DEFINED
typedef unsigned long time_t;
#define _TIME_T_DEFINED
#endif

#ifndef _FSIZE_T_DEFINED
typedef unsigned long _fsize_t; // Could be 64 bits for Win32
#define _FSIZE_T_DEFINED
#endif

#ifndef _FINDDATA_T_DEFINED
struct _finddata_t {
	unsigned    attrib;
	time_t      time_create;    // -1 for FAT file systems
	time_t      time_access;    // -1 for FAT file systems
	time_t      time_write;
	_fsize_t    size;
	char        name[260];
};
#define _FINDDATA_T_DEFINED
#endif
long _findfirst( char *filespec, struct _finddata_t *fileinfo );
int _findnext( long handle, struct _finddata_t *fileinfo );
int _findclose( long handle );
*/

// process.h
void abort(void);

// types.h
#define BIG_ENDIAN    1234
#define LITTLE_ENDIAN 4321

#ifdef MIPS
#define BYTE_ORDER LITTLE_ENDIAN
#endif

//#if UNDER_CE > 201
//  typedef unsigned long time_t;
//  #define _TIME_T_DEFINED_
//#endif
typedef unsigned long dev_t;
typedef unsigned long ino_t;
#ifndef _MODE_T_DEFINED_
typedef unsigned long mode_t;
#define _MODE_T_DEFINED_
#endif

typedef long clock_t;

#ifndef _PTRDIFF_T_DEFINED
typedef long ptrdiff_t;
#define _PTRDIFF_T_DEFINED
#endif

typedef long off_t;

//typedef unsigned char u_char;
//typedef unsigned short u_short;

#ifndef _CADDR_T_DEFINED_
typedef unsigned char * caddr_t;
#define _CADDR_T_DEFINED_
#endif

#ifndef _SIZE_T_DEFINED_
typedef unsigned int size_t;
#define _SIZE_T_DEFINED_
#endif

#ifndef _SSIZE_T_DEFINED_
typedef unsigned long ssize_t;
#define _SSIZE_T_DEFINED_
#endif

//typedef unsigned char u_int8_t;

//typedef short int16_t;
//typedef unsigned short u_int16_t;

//typedef int int32_t;
//typedef unsigned int u_int32_t;

//typedef unsigned long u_long;
//typedef unsigned int u_int;

#ifndef _TIME_T_DEFINED_
typedef unsigned long time_t;
#define _TIME_T_DEFINED_
#endif

// stat.h
#define _S_IFMT         0170000         /* file type mask */
#define _S_IFDIR        0040000         /* directory */
#define _S_IFCHR        0020000         /* character special */
#define _S_IFIFO        0010000         /* pipe */
#define _S_IFREG        0100000         /* regular */
#define _S_IREAD        0000400         /* read permission, owner */
#define _S_IWRITE       0000200         /* write permission, owner */
#define _S_IEXEC        0000100         /* execute/search permission, owner */

#define S_IFMT   _S_IFMT
#define S_IFREG  _S_IFREG
#define S_IFCHR  _S_IFCHR
#define S_IFDIR  _S_IFDIR
#define S_IREAD  _S_IREAD
#define S_IWRITE _S_IWRITE
#define S_IEXEC  _S_IEXEC

#ifndef S_ISDIR
#define S_ISDIR(X) (((X) & S_IFMT) == S_IFDIR)
#endif
#ifndef S_ISREG
#define S_ISREG(X) (((X) & S_IFMT) == S_IFREG)
#endif

#ifdef __cplusplus
extern "C" {
#endif

	// in sys/types.h
	//typedef unsigned int _dev_t;
	//typedef long _off_t;
	//typedef unsigned short _ino_t;

#ifndef _STAT_DEFINED
	struct stat 
	{
		dev_t st_dev;
		ino_t st_ino;
		unsigned short st_mode;
		short st_nlink;
		short st_uid;
		short st_gid;
		dev_t st_rdev;
		off_t st_size;
		time_t st_atime;
		time_t st_mtime;
		time_t st_ctime;
	};
#define _STAT_DEFINED
#endif /* _STAT_DEFINED */

#define _stat stat

	int _stat(const char *filename, struct _stat *stat);
	int fstat(int file, struct stat *sbuf);

	int _wstat(const wchar_t *filename, struct _stat *stat);


#ifdef __cplusplus
};
#endif

// direct.h
#ifdef __cplusplus
extern "C" {
#endif

	char *getcwd(char* buffer, int maxlen);
	int _chdir(const char * dirname);
	int _rmdir(const char * dir);
	int _mkdir(const char * dir);

	int _wchdir(const wchar_t * dirname);
	int _wmkdir(const wchar_t * dir);
	int _wrmdir(const wchar_t * dir);

#ifdef __cplusplus
};
#endif

#define chdir      _chdir
#define rmdir      _rmdir
#define mkdir      _mkdir

// time.h
#ifdef __cplusplus
extern "C" {
#endif

extern int daylight;
extern char *tzname[2];

#if 0
#define _DAY_SEC           (24L * 60L * 60L)    /* secs in a day */
#define _YEAR_SEC          (365L * _DAY_SEC)    /* secs in a year */
#define _FOUR_YEAR_SEC     (1461L * _DAY_SEC)   /* secs in a 4 year interval */
#define _DEC_SEC           315532800L           /* secs in 1970-1979 */
#define _BASE_YEAR         70L                  /* 1970 is the base year */
#define _BASE_DOW          4                    /* 01-01-70 was a Thursday */
#define _LEAP_YEAR_ADJUST  17L                  /* Leap years 1900 - 1970 */
#define _MAX_YEAR          138L                 /* 2038 is the max year */
#endif

#ifndef _TM_DEFINED
	struct tm {
		int tm_sec;     /* seconds after the minute - [0,59] */
		int tm_min;     /* minutes after the hour - [0,59] */
		int tm_hour;    /* hours since midnight - [0,23] */
		int tm_mday;    /* day of the month - [1,31] */
		int tm_mon;     /* months since January - [0,11] */
		int tm_year;    /* years since 1900 */
		int tm_wday;    /* days since Sunday - [0,6] */
		int tm_yday;    /* days since January 1 - [0,365] */
		int tm_isdst;   /* daylight savings time flag */
	};
#define _TM_DEFINED
#endif


	typedef struct {
		int  yr;        // year of interest
		int  yd;        // day of year 
		long ms;        // milli-seconds in the day 
	} transitionTime;

	time_t mktime(struct tm* pt);
	time_t time( time_t *timer );
	struct tm *localtime(const time_t *ptime);
	struct tm *gmtime(const time_t *tod);
	char* ctime( const time_t *t );
	char* asctime(const struct tm *tptr);
	time_t wce_FILETIME2time_t(const FILETIME* pf);

	int ctime_s(char* buffer, size_t sizeInBytes, const time_t *time);


#ifdef __cplusplus
};
#endif

//
char* strerror(int errno);

void *bsearch( const void *key, const void *base, size_t num, size_t width, int ( __cdecl *compare )(const void *, const void *));

char *getenv(char *v);

int access(const char *filename, int flags);

#endif

#endif