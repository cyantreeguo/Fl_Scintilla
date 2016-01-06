// Copyright 2015-2016 by cyantree <cyantree.guo@gmail.com>

#ifndef PLATFLTK_H
#define PLATFLTK_H

#include "FL/Fl_Group.H"
#include "FL/Fl_Double_Window.H"
#include "FL/fl_draw.H"

#include "CallTip.h"
#include "Scintilla.h"
#include "iconv.h"

typedef struct {
	unsigned char type; // 0-Fl_ScintillaBase, 1-listbox, 2-calltip
	void *wid;

	struct {
		int x, y, w, h;
	} rc_client;

	struct {
		int x, y;
	} main_client;
} SCIWinType;

class ICONVConverter
{
private:
	iconv_t cd;
	char *out_;
	int outlen_;
public:
	ICONVConverter() 
	{ 
		cd = 0;

		out_ = NULL;
		outlen_ = 0; 
	}
	~ICONVConverter() 
	{ 
		Close();

		if ( out_ != NULL ) free(out_);
	}

	unsigned char Open4UTF8(int codepage)
	{
		if ( codepage == 936   ) return Open("GB18030", "UTF-8"); // SC_CHARSET_GB2312
		if ( codepage == 950   ) return Open("BIG5", "UTF-8"); // SC_CHARSET_CHINESEBIG5 繁体中文
		if ( codepage == 1257  ) return Open("", "UTF-8"); // SC_CHARSET_BALTIC 波罗的海字集		
		if ( codepage == 1250  ) return Open("", "UTF-8"); // SC_CHARSET_EASTEUROPE 中欧语系		
		if ( codepage == 1253  ) return Open("", "UTF-8"); // SC_CHARSET_GREEK 希腊文
		if ( codepage == 949   ) return Open("HANGUL", "UTF-8"); // SC_CHARSET_HANGUL 韩文
		if ( codepage == 10000 ) return Open("", "UTF-8"); // SC_CHARSET_MAC
		if ( codepage == 437   ) return Open("", "UTF-8"); // SC_CHARSET_OEM 地区自订
		if ( codepage == 1251  ) return Open("", "UTF-8"); // SC_CHARSET_RUSSIAN 俄文 (斯拉夫语系)
		if ( codepage == 932   ) return Open("", "UTF-8"); // SC_CHARSET_SHIFTJIS 日语
		if ( codepage == 1254  ) return Open("", "UTF-8"); // SC_CHARSET_TURKISH 土耳其文
		if ( codepage == 1361  ) return Open("JOHAB", "UTF-8"); // SC_CHARSET_JOHAB 韩语
		if ( codepage == 1255  ) return Open("", "UTF-8"); // SC_CHARSET_HEBREW 希伯来文
		if ( codepage == 1256  ) return Open("", "UTF-8"); // SC_CHARSET_ARABIC 阿拉伯文
		if ( codepage == 1258  ) return Open("", "UTF-8"); // SC_CHARSET_VIETNAMESE 越南文
		if ( codepage == 874   ) return Open("", "UTF-8"); // SC_CHARSET_THAI 泰文
		if ( codepage == 28605 ) return Open("ISO-8859-15", "UTF-8"); // SC_CHARSET_8859_15

		return Open("", "UTF-8");
	}

	unsigned char Open4MB(int codepage)
	{
		if ( codepage == 936   ) return Open("UTF-8", "GB18030"); // SC_CHARSET_GB2312
		if ( codepage == 950   ) return Open("UTF-8", "BIG5"); // SC_CHARSET_CHINESEBIG5 繁体中文
		if ( codepage == 1257  ) return Open("UTF-8", ""); // SC_CHARSET_BALTIC 波罗的海字集		
		if ( codepage == 1250  ) return Open("UTF-8", ""); // SC_CHARSET_EASTEUROPE 中欧语系		
		if ( codepage == 1253  ) return Open("UTF-8", ""); // SC_CHARSET_GREEK 希腊文
		if ( codepage == 949   ) return Open("UTF-8", "HANGUL"); // SC_CHARSET_HANGUL 韩文
		if ( codepage == 10000 ) return Open("UTF-8", ""); // SC_CHARSET_MAC
		if ( codepage == 437   ) return Open("UTF-8", ""); // SC_CHARSET_OEM 地区自订
		if ( codepage == 1251  ) return Open("UTF-8", ""); // SC_CHARSET_RUSSIAN 俄文 (斯拉夫语系)
		if ( codepage == 932   ) return Open("UTF-8", ""); // SC_CHARSET_SHIFTJIS 日语
		if ( codepage == 1254  ) return Open("UTF-8", ""); // SC_CHARSET_TURKISH 土耳其文
		if ( codepage == 1361  ) return Open("UTF-8", "JOHAB"); // SC_CHARSET_JOHAB 韩语
		if ( codepage == 1255  ) return Open("UTF-8", ""); // SC_CHARSET_HEBREW 希伯来文
		if ( codepage == 1256  ) return Open("UTF-8", ""); // SC_CHARSET_ARABIC 阿拉伯文
		if ( codepage == 1258  ) return Open("UTF-8", ""); // SC_CHARSET_VIETNAMESE 越南文
		if ( codepage == 874   ) return Open("UTF-8", ""); // SC_CHARSET_THAI 泰文
		if ( codepage == 28605 ) return Open("UTF-8", "ISO-8859-15"); // SC_CHARSET_8859_15

		return Open("UTF-8", "");
	}

	unsigned char Open(const char *from_charset,const char *to_charset)
	{
		Close();
		cd = iconv_open(to_charset, from_charset);
#if __FLTK_MACOSX__ || __FLTK_IPHONEOS__
		if ( (uintptr_t)cd > 0 ) return 1;
#else
		if ( (int)cd > 0 ) return 1;
#endif
		return 0;
	}	

	void Close()
	{
#if __FLTK_MACOSX__ || __FLTK_IPHONEOS__
		if ( (uintptr_t)cd > 0 ) iconv_close(cd);
#else
		if ( (int)cd > 0 ) iconv_close(cd);
#endif
		cd = 0;
	}

	int GetLength(char *src, int len)
	{
		if ( len < 1 ) return 0;
		int outlen=len*8;
		void *p;
		if ( outlen > outlen_ ) {
			p = realloc(out_, outlen);
			if ( p == NULL ) return 0;
			out_ = (char *)p;
			outlen_ = outlen;
		}
		int r = convert(src, len, out_, outlen);

		return r;
	}

	int convert(char *inbuf, int inlen, char *outbuf, int outlen) 
	{
		char **pin = &inbuf;
		char **pout = &outbuf;
		int len = outlen;

#if __FLTK_MACOSX__ || __FLTK_IPHONEOS__
		if ( (uintptr_t)cd > 0 ) {
#else
		if ( (int)cd > 0 ) {
#endif
			int r = iconv(cd,(const char**)pin,(size_t *)&inlen,pout,(size_t *)&len);
			if ( r < 0 ) return 0;
			return outlen-len;
		} else {
			return 0;
		}
	}
};

extern void Platform_Initialise();
extern void Platform_Finalise();

#endif
