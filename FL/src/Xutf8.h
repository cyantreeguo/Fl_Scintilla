/* "$Id: Xutf8.h 9958 2013-09-02 15:05:58Z manolo $"
 *
 * Author: Jean-Marc Lienher ( http://oksid.ch )
 * Copyright 2000-2010 by O'ksi'D.
 *
 * This library is free software. Distribution and use rights are outlined in
 * the file "COPYING" which should have been included with this file.  If this
 * file is missing or damaged, see the license at:
 *
 *     http://www.fltk.org/COPYING.php
 *
 * Please report all bugs and problems on the following page:
 *
 *     http://www.fltk.org/str.php
 */

#if ! ( defined(_Xutf8_h) || defined(FL_DOXYGEN) )
#define _Xutf8_h

#include "Fl_Platform.h"
#if __FLTK_LINUX__

/*
#  ifdef __cplusplus
extern "C" {
#  endif
*/

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xlocale.h>
#include <X11/Xutil.h>

typedef struct {
	int nb_font;
	char **font_name_list;
	int *encodings;
	XFontStruct **fonts;
	Font fid;
	int ascent;
	int descent;
	int *ranges;
} XUtf8FontStruct;

extern XUtf8FontStruct *
XCreateUtf8FontStruct (
        Display         *dpy,
        const char      *base_font_name_list);

extern void
XUtf8DrawString(
        Display         	*display,
        Drawable        	d,
        XUtf8FontStruct  *font_set,
        GC              	gc,
        int             	x,
        int             	y,
        const char      	*string,
        int             	num_bytes);

extern void
XUtf8_measure_extents(
        Display         	*display,
        Drawable        	d,
        XUtf8FontStruct  *font_set,
        GC              	gc,
        int             	*xx,
        int             	*yy,
        int             	*ww,
        int             	*hh,
        const char      	*string,
        int             	num_bytes);

extern void
XUtf8DrawRtlString(
        Display         	*display,
        Drawable        	d,
        XUtf8FontStruct  *font_set,
        GC              	gc,
        int             	x,
        int             	y,
        const char      	*string,
        int             	num_bytes);

extern void
XUtf8DrawImageString(
        Display         *display,
        Drawable        d,
        XUtf8FontStruct         *font_set,
        GC              gc,
        int             x,
        int             y,
        const char      *string,
        int             num_bytes);

extern int
XUtf8TextWidth(
        XUtf8FontStruct  *font_set,
        const char      	*string,
        int             	num_bytes);
extern int
XUtf8UcsWidth(
        XUtf8FontStruct  *font_set,
        unsigned int            ucs);

extern int
fl_XGetUtf8FontAndGlyph(
        XUtf8FontStruct  *font_set,
        unsigned int            ucs,
        XFontStruct     **fnt,
        unsigned short  *id);

extern void
XFreeUtf8FontStruct(
        Display         	*dpy,
        XUtf8FontStruct 	*font_set);


extern int
XConvertUtf8ToUcs(
        const unsigned char 	*buf,
        int 			len,
        unsigned int 		*ucs);

extern int
XConvertUcsToUtf8(
        unsigned int 		ucs,
        char 			*buf);

extern int
XUtf8CharByteLen(
        const unsigned char 	*buf,
        int 			len);

extern int
XCountUtf8Char(
        const unsigned char *buf,
        int len);

extern int
XFastConvertUtf8ToUcs(
        const unsigned char 	*buf,
        int 			len,
        unsigned int 		*ucs);

extern long
XKeysymToUcs(
        KeySym 	keysym);

#ifdef X_HAVE_UTF8_STRING
#define XUtf8LookupString Xutf8LookupString
#else
extern int
XUtf8LookupString(
        XIC                 ic,
        XKeyPressedEvent*   event,
        char*               buffer_return,
        int                 bytes_buffer,
        KeySym*             keysym,
        Status*             status_return);
#endif		

extern unsigned short
XUtf8IsNonSpacing(
        unsigned int ucs);

extern unsigned short
XUtf8IsRightToLeft(
        unsigned int ucs);


extern int
XUtf8Tolower(
        int ucs);

extern int
XUtf8Toupper(
        int ucs);

/*
#  ifdef __cplusplus
}
#  endif
*/

#endif

#endif

/*
 *  End of "$Id: Xutf8.h 9958 2013-09-02 15:05:58Z manolo $".
 */
