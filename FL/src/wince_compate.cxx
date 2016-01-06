#include "wince_compate.h"

#if __FLTK_WINCE__

// errno.c
int errno;
int _doserrno;
int _sys_nerr;

// signal.c
int raise(int sig)
{
	return 0;
}

// wince.c
/* char -> wchar_t */
wchar_t* wce_mbtowc(const char* a)
{
	int length;
	wchar_t *wbuf;

	length = MultiByteToWideChar(CP_ACP, 0, 
		a, -1, NULL, 0);
	wbuf = (wchar_t*)malloc( (length+1)*sizeof(wchar_t) );
	MultiByteToWideChar(CP_ACP, 0,
		a, -1, wbuf, length);

	return wbuf;
}

/* wchar_t -> char */
char* wce_wctomb(const wchar_t* w)
{
	DWORD charlength;
	char* pChar;

	charlength = WideCharToMultiByte(CP_ACP, 0, w,
		-1, NULL, 0, NULL, NULL);
	pChar = (char*)malloc(charlength+1);
	WideCharToMultiByte(CP_ACP, 0, w,
		-1, pChar, charlength, NULL, NULL);

	return pChar;
}

// stdio.c
char* buftostring(const void* buf, size_t count);

int _rename(const char *oldname, const char *newname)
{
	wchar_t *wold, *wnew;
	BOOL rc;

	wold = wce_mbtowc(oldname);
	wnew = wce_mbtowc(newname);

	/* replace with MoveFile. */
	rc = MoveFileW(wold, wnew);

	free(wold);
	free(wnew);

	return rc==TRUE ? 0 : -1;
}

int _unlink(const char *file)
{
	wchar_t *wfile;
	BOOL rc;

	/* replace with DeleteFile. */
	wfile = wce_mbtowc(file);
	rc = DeleteFileW(wfile);
	free(wfile);

	return rc==TRUE ? 0 : -1;
}

/* replace "open" with "CreateFile", etc. */
int win_open(const char *file, int mode, va_list arg)
{
	wchar_t *wfile;
	DWORD access=0, share=0, create=0;
	int h;

	wfile = wce_mbtowc(file);
	
	h = _wopen(wfile, mode);

	free(wfile);
	return h;
}

int close(int fd)
{
	CloseHandle( (HANDLE)fd );
	return 0;
}

int _read(int fd, void *buffer, int length)
{
	DWORD dw;
	ReadFile( (HANDLE)fd, buffer, length, &dw, NULL );
	return (int)dw;
}

int _write(int fd, const void *buffer, unsigned count)
{	
	if(fd == 1) {
		const char* cbuf = buftostring(buffer, count);
		fprintf(stdout, (char*) cbuf);
		free((void*)cbuf);
		return count;
	} else if (fd == 2) {
		const char* cbuf = buftostring(buffer, count);
		fprintf(stdout, (char*) buffer);
		free((void*)cbuf);
		return count;
	} else {
		DWORD dw;
		dw = WriteFile((HANDLE) fd, buffer, count, &dw, NULL );
		return (int) dw;
	}	
}

long _lseek(int handle, long offset, int origin)
{
	DWORD flag, ret;

	switch(origin)
	{
	case SEEK_SET: flag = FILE_BEGIN;   break;
	case SEEK_CUR: flag = FILE_CURRENT; break;
	case SEEK_END: flag = FILE_END;     break;
	default:       flag = FILE_CURRENT; break;
	}

	ret = SetFilePointer( (HANDLE)handle, offset, NULL, flag );
	return ret==0xFFFFFFFF ? -1 : 0;
}

/* _findfirst, _findnext, _findclose. */
/* replace them with FindFirstFile, etc. */
/*
long _findfirst( char *file, struct _finddata_t *fi )
{
	HANDLE h;
	WIN32_FIND_DATAA fda;

	h = FindFirstFile( file, &fda );
	if( h==NULL )
	{
		errno = EINVAL; return -1;
	}

	fi->attrib      = fda.dwFileAttributes;
	fi->time_create = wce_FILETIME2time_t( &fda.ftCreationTime );
	fi->time_access = wce_FILETIME2time_t( &fda.ftLastAccessTime );
	fi->time_write  = wce_FILETIME2time_t( &fda.ftLastWriteTime );
	fi->size        = fda.nFileSizeLow + (fda.nFileSizeHigh<<32);
	strcpy( fi->name, fda.cFileName );

	return (long)h;
}

int _findnext( long handle, struct _finddata_t *fi )
{
	WIN32_FIND_DATAA fda;
	BOOL b;

	b = FindNextFile( (HANDLE)handle, &fda );

	if( b==FALSE )
	{
		errno = ENOENT; return -1;
	}

	fi->attrib      = fda.dwFileAttributes;
	fi->time_create = wce_FILETIME2time_t( &fda.ftCreationTime );
	fi->time_access = wce_FILETIME2time_t( &fda.ftLastAccessTime );
	fi->time_write  = wce_FILETIME2time_t( &fda.ftLastWriteTime );
	fi->size        = fda.nFileSizeLow + (fda.nFileSizeHigh<<32);
	strcpy( fi->name, fda.cFileName );

	return 0;
}

int _findclose( long handle )
{
	BOOL b;
	b = FindClose( (HANDLE)handle );
	return b==FALSE ? -1 : 0;
}
*/

/* below functions unsupported... */
/* I have no idea how to replace... */
int _chsize(int handle, long size)
{
	errno = EACCES;
	return -1;
}

int _umask(int cmask)
{
	return 0;
}

int _chmod(const char *path, int mode)
{
	return 0;
}

/* WinCE doesn't have dup and dup2.  */
/* so, we cannot use missing/dup2.c. */
int dup( int handle )
{
	errno = EBADF;
	return -1;
}
/*
int dup2( int handle1, int handle2 )
{
	errno = EBADF;
	return -1;
}
*/
int _isatty(int fd)
{
	if( fd==(int)_fileno(stdin) || 
		fd==(int)_fileno(stdout)||
		fd==(int)_fileno(stderr) )
		return 1;
	else
		return 0;
}

int _pipe(int *phandles, unsigned int psize, int textmode)
{
	return -1;
}

int _access(const char *filename, int flags)
{
	return 0;
}

int _open_osfhandle( long osfhandle, int flags)
{
/*	return 0; */
	return (int)osfhandle;
}

long _get_osfhandle( int filehandle )
{
/*	return 0; */
	return (long)filehandle;
}

int _wsopen_s( int * _FileHandle, const wchar_t * _Filename, int _OpenFlag, int _ShareFlag, int _PermissionFlag) {
	return 0;
}

int _waccess(const wchar_t * _Filename,  int _AccessMode) {
	return 0;
}

int _wunlink(const wchar_t * _Filename) {
	return 0;
}

int _wrename(const wchar_t * _OldFilename, const wchar_t * _NewFilename) {
	return 0;
}

int _wmkdir(const wchar_t * dir) {
	return 0;
}

char* buftostring(const void* buf, size_t count) {
	char* cbuf = (char*)malloc((count + 1) * sizeof(char));
	memcpy(cbuf, buf, count);
	*(cbuf + count) = '\0';
	return cbuf;
}

int _wopen(const wchar_t *filename, int mode) {

	DWORD access = 0, create = 0;
	HANDLE h;

	if((mode & _O_RDWR) != 0) 
		access = GENERIC_READ | GENERIC_WRITE;
	else if((mode & _O_WRONLY) != 0)
		access = GENERIC_WRITE;
	else
		access = GENERIC_READ;

	if((mode & _O_CREAT) != 0)
		create = CREATE_ALWAYS;
	else
		create = OPEN_ALWAYS;
	
	h = CreateFileW(filename, access, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
			create, 0, NULL );

	return (int)h;
}

// process.c
void abort(void)
{
	exit(3);
}

// stat.c
const __int64 _onesec_in100ns = (__int64)10000000;
/* global for chdir, getcwd */
char _currentdir[MAX_PATH+1];
TCHAR *wce_replaceRelativeDir(const char* str)
{
	TCHAR *tbuf;

	if( 2<=strlen(str) && str[0]=='.' &&
		(str[1]=='/' || str[1]=='\\') )
	{
		char *buf;
		int len = strlen(str) + strlen(_currentdir);
		buf = (char *)malloc( len+1 );
		sprintf(buf, "%s%s", _currentdir, &str[1]);
		tbuf = wce_mbtowc(buf);
		free(buf);
	}
	else
		tbuf = wce_mbtowc(str);
	return tbuf;
}
/* __int64 <--> FILETIME */
static __int64 wce_FILETIME2int64(FILETIME f)
{
	__int64 t;

	t = f.dwHighDateTime;
	t <<= 32;
	t |= f.dwLowDateTime;
	return t;
}
/* FILETIME utility */
static FILETIME wce_getFILETIMEFromYear(WORD year)
{
	SYSTEMTIME s={0};
	FILETIME f;

	s.wYear      = year;
	s.wMonth     = 1;
	s.wDayOfWeek = 1;
	s.wDay       = 1;

	SystemTimeToFileTime( &s, &f );
	return f;
}
/* FILETIME <--> time_t */
time_t wce_FILETIME2time_t(const FILETIME* f)
{
	FILETIME f1601, f1970;
	__int64 t, offset;

	f1601 = wce_getFILETIMEFromYear(1601);
	f1970 = wce_getFILETIMEFromYear(1970);

	offset = wce_FILETIME2int64(f1970) - wce_FILETIME2int64(f1601);

	t = wce_FILETIME2int64(*f);

	t -= offset;
	return (time_t)(t / _onesec_in100ns);
}
int _stat(const char *filename, struct _stat *st)
{
	DWORD dwAttribute;
	HANDLE h;
	DWORD dwSizeLow=0, dwSizeHigh=0, dwError=0;
	WIN32_FIND_DATAW fd;
	wchar_t *wfilename;

	//	wfilename = wce_mbtowc(filename);
	wfilename = wce_replaceRelativeDir(filename);

	dwAttribute = GetFileAttributesW(wfilename);
	if(dwAttribute==0xFFFFFFFF)
	{
		free(wfilename);
		return -1;
	}

	st->st_mode = 0;
	if((dwAttribute & FILE_ATTRIBUTE_DIRECTORY) != 0)
		st->st_mode += S_IFDIR;
	else
		st->st_mode += S_IFREG;

	/* initialize */
	st->st_atime = 0;
	st->st_mtime = 0;
	st->st_ctime = 0;
	st->st_size  = 0;
	st->st_dev   = 0;

	h = FindFirstFileW(wfilename, &fd);
	if(h == INVALID_HANDLE_VALUE)
	{
		if(wfilename[wcslen(wfilename)-1]	== L'\\')
		{
			wfilename[wcslen(wfilename)-1] = L'\0';
			h = FindFirstFileW(wfilename, &fd);
			if(h == INVALID_HANDLE_VALUE)
			{
				free(wfilename);
				return 0;
			}
		}
		else
		{
			free(wfilename);
			return 0;
		}
	}

	/* FILETIME -> time_t */
	st->st_atime = wce_FILETIME2time_t(&fd.ftLastAccessTime);
	st->st_mtime = wce_FILETIME2time_t(&fd.ftLastWriteTime);
	st->st_ctime = wce_FILETIME2time_t(&fd.ftCreationTime);
	st->st_size  = fd.nFileSizeLow;

	FindClose( h );
	free(wfilename);
	return 0;
}

int fstat(int file, struct stat *sbuf)
{
	/* GetFileSize & GetFileTime */
	DWORD dwSize;
	FILETIME ctime, atime, mtime;

	dwSize = GetFileSize( (HANDLE)file, NULL );
	if( dwSize == 0xFFFFFFFF )
		return -1;

	sbuf->st_size = dwSize;
	sbuf->st_dev  = 0;
	sbuf->st_rdev = 0;
	sbuf->st_mode = _S_IFREG;
	sbuf->st_nlink= 1;

	GetFileTime( (HANDLE)file, &ctime, &atime, &mtime );
	sbuf->st_ctime = wce_FILETIME2time_t(&ctime);
	sbuf->st_atime = wce_FILETIME2time_t(&atime);
	sbuf->st_mtime = wce_FILETIME2time_t(&mtime);

	return 0;
}

int _wstat(const wchar_t *filename, struct _stat *stat) {
	return 0;
}

// direct.h
char *getcwd(char* buffer, int maxlen)
{
	strcpy( buffer, _currentdir );
	return buffer;
}

int _chdir(const char * dirname)
{
	if( MAX_PATH < strlen(dirname) )
		return -1;

	strcpy( _currentdir, dirname );
	return 0;
}

int _rmdir(const char * dir)
{
	wchar_t *wdir;
	BOOL rc;

	/* replace with RemoveDirectory. */
	wdir = wce_mbtowc(dir);
	rc = RemoveDirectoryW(wdir);
	free(wdir);

	return rc==TRUE ? 0 : -1;
}

int _mkdir(const char * dir)
{
	wchar_t* wdir;
	BOOL rc;

	/* replace with CreateDirectory. */
	wdir = wce_mbtowc(dir);
	rc = CreateDirectoryW(wdir, NULL);
	free(wdir);

	return rc==TRUE ? 0 : -1;
}

int _wrmdir(const wchar_t * dir)
{
	BOOL rc;

	/* replace with RemoveDirectory. */
	rc = RemoveDirectoryW(dir);

	return rc==TRUE ? 0 : -1;
}

// time.c
/* globals */
int   daylight;
char *tzname[2];

static FILETIME wce_int642FILETIME(__int64 t)
{
	FILETIME f;

	f.dwHighDateTime = (DWORD)((t >> 32) & 0x00000000FFFFFFFF);
	f.dwLowDateTime  = (DWORD)( t        & 0x00000000FFFFFFFF);
	return f;
}

/* FILETIME utility */
static time_t wce_getYdayFromSYSTEMTIME(const SYSTEMTIME* s)
{
	__int64 t;
	FILETIME f1, f2;

	f1 = wce_getFILETIMEFromYear( s->wYear );
	SystemTimeToFileTime( s, &f2 );

	t = wce_FILETIME2int64(f2)-wce_FILETIME2int64(f1);

	return (time_t)((t/_onesec_in100ns)/(60*60*24));
}

/* tm <--> SYSTEMTIME */
static SYSTEMTIME wce_tm2SYSTEMTIME(struct tm *t)
{
	SYSTEMTIME s;

	s.wYear      = t->tm_year + 1900;
	s.wMonth     = t->tm_mon  + 1;
	s.wDayOfWeek = t->tm_wday;
	s.wDay       = t->tm_mday;
	s.wHour      = t->tm_hour;
	s.wMinute    = t->tm_min;
	s.wSecond    = t->tm_sec;
	s.wMilliseconds = 0;

	return s;
}

static struct tm wce_SYSTEMTIME2tm(SYSTEMTIME *s)
{
	struct tm t;

	t.tm_year  = s->wYear - 1900;
	t.tm_mon   = s->wMonth- 1;
	t.tm_wday  = s->wDayOfWeek;
	t.tm_mday  = s->wDay;
	t.tm_yday  = wce_getYdayFromSYSTEMTIME(s);
	t.tm_hour  = s->wHour;
	t.tm_min   = s->wMinute;
	t.tm_sec   = s->wSecond;
	t.tm_isdst = 0;

	return t;
}

FILETIME wce_time_t2FILETIME(const time_t t)
{
	FILETIME f, f1970;
	__int64 time;

	f1970 = wce_getFILETIMEFromYear(1970);

	time = t;
	time *= _onesec_in100ns;
	time += wce_FILETIME2int64(f1970);

	f = wce_int642FILETIME(time);

	return f;
}

/* time.h difinition */
time_t time( time_t *timer )
{
	SYSTEMTIME s;
	FILETIME   f;

	if( timer==NULL ) return 0;

	GetSystemTime( &s );

	SystemTimeToFileTime( &s, &f );

	*timer = wce_FILETIME2time_t(&f);
	return *timer;
}

struct tm *localtime( const time_t *timer )
{
	SYSTEMTIME ss, ls, s;
	FILETIME   sf, lf, f;
	__int64 t, diff;
	static struct tm tms;

	GetSystemTime(&ss);
	GetLocalTime(&ls);

	SystemTimeToFileTime( &ss, &sf );
	SystemTimeToFileTime( &ls, &lf );

	diff = wce_FILETIME2int64(sf) - wce_FILETIME2int64(lf);

	f = wce_time_t2FILETIME(*timer);
	t = wce_FILETIME2int64(f) - diff;
	f = wce_int642FILETIME(t);

	FileTimeToSystemTime( &f, &s );

	tms = wce_SYSTEMTIME2tm(&s);

	return &tms;
}

time_t mktime(struct tm* pt)
{
	SYSTEMTIME ss, ls, s;
	FILETIME   sf, lf, f;
	__int64 diff;

	GetSystemTime(&ss);
	GetLocalTime(&ls);
	SystemTimeToFileTime( &ss, &sf );
	SystemTimeToFileTime( &ls, &lf );

	diff = (wce_FILETIME2int64(lf)-wce_FILETIME2int64(sf))/_onesec_in100ns;

	s = wce_tm2SYSTEMTIME(pt);
	SystemTimeToFileTime( &s, &f );
	return wce_FILETIME2time_t(&f) - (time_t)diff;
}

struct tm *gmtime(const time_t *t)
{
	FILETIME f;
	SYSTEMTIME s;
	static struct tm tms;

	f = wce_time_t2FILETIME(*t);
	FileTimeToSystemTime(&f, &s);
	tms = wce_SYSTEMTIME2tm(&s);
	return &tms;
}

char* ctime( const time_t *t )
{
	// Wed Jan 02 02:03:55 1980\n\0
	static char buf[30]={0};
	char week[] = "Sun Mon Tue Wed Thr Fri Sat ";
	char month[]= "Jan Feb Mar Apl May Jun Jul Aug Sep Oct Nov Dec ";
	struct tm tms;

	tms = *localtime(t);

	strncpy( buf,    week+tms.tm_wday*4, 4 );
	strncpy( buf+4,  month+tms.tm_mon*4, 4 );
	sprintf( buf+8,  "%02d ", tms.tm_mday );
	sprintf( buf+11, "%02d:%02d:%02d %d\n", 
		tms.tm_hour, tms.tm_min, tms.tm_sec, tms.tm_year+1900 );
	return buf;
}

int ctime_s(char* buffer, size_t sizeInBytes, const time_t *time) {
	char* src;

	if(buffer == NULL || sizeInBytes == 0) {
		return EINVAL;
	}

	src = ctime(time);
	memcpy(buffer, src, sizeInBytes);
	return 0;
}

char *asctime(const struct tm *pt)
{
	static char buf[30]={0};
	char week[] = "Sun Mon Tue Wed Thr Fri Sat ";
	char month[]= "Jan Feb Mar Apl May Jun Jul Aug Sep Oct Nov Dec ";

	strncpy( buf,    week+pt->tm_wday*4, 4 );
	strncpy( buf+4,  month+pt->tm_mon*4, 4 );
	sprintf( buf+8,  "%02d ", pt->tm_mday );
	sprintf( buf+11, "%02d:%02d:%02d %d\n", 
		pt->tm_hour, pt->tm_min, pt->tm_sec, pt->tm_year+1900 );
	return buf;
}

//
char* strerror(int errno)
{
	static char buf[32]="wince::strerror called.";
	return buf;
}

void *bsearch( const void *key, const void *base, size_t num, size_t width, int ( __cdecl *compare )(const void *, const void *))
{
	size_t i;
	const void* p = base;
	const char* px;

	for( i=0; i<num; i++ )
	{
		if( 0==compare( key, p ) )
			return (void*)p;
		px = (const char*)p; px+=width; p=(const void*)px;
	}
	return NULL;
}

char *getenv(char *v)
{
	return NULL;
}

int access(const char *filename, int flags)
{
	return 0;
}

#endif
