/*
 * "$Id: numericsort.c 8864 2011-07-19 04:49:30Z greg.ercolano $"
 *
 * Numeric sorting routine for the Fast Light Tool Kit (FLTK).
 *
 * Copyright 1998-2010 by Bill Spitzak and others.
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

/* My own scandir sorting function, useful for the film industry where
   we have many files with numbers in their names: */

#include "fltk_config.h"
#include <ctype.h>
#include <stdlib.h>

#if __FLTK_WINCE__
#include "wince_compate.h"
#else
#include <sys/types.h>
#endif

#if !defined(WIN32) || defined(__CYGWIN__)
#  ifdef HAVE_DIRENT_H
#    include <dirent.h>
#  else
#    define dirent direct
#    if HAVE_SYS_NDIR_H
#      include <sys/ndir.h>
#    endif /* HAVE_SYS_NDIR_H */
#    if HAVE_SYS_DIR_H
#      include <sys/dir.h>
#    endif /* HAVE_SYS_DIR_H */
#    if HAVE_NDIR_H
#      include <ndir.h>
#    endif /* HAVE_NDIR_H */
#  endif /* HAVE_DIRENT_H */
#endif /* !WIN32 || __CYGWIN__ */

#include "filename.H"

/*
 * 'numericsort()' - Compare two directory entries, possibly with
 *                   a case-insensitive comparison...
 */

#  ifdef __cplusplus
extern "C" {
#  endif

	static int numericsort(struct dirent **A, struct dirent **B, int cs)
	{
		const char* a = (*A)->d_name;
		const char* b = (*B)->d_name;
		int ret = 0;
		for (;;) {
			if (isdigit(*a & 255) && isdigit(*b & 255)) {
				int diff,magdiff;
				while (*a == '0') a++;
				while (*b == '0') b++;
				while (isdigit(*a & 255) && *a == *b) {
					a++;
					b++;
				}
				diff = (isdigit(*a & 255) && isdigit(*b & 255)) ? *a - *b : 0;
				magdiff = 0;
				while (isdigit(*a & 255)) {
					magdiff++;
					a++;
				}
				while (isdigit(*b & 255)) {
					magdiff--;
					b++;
				}
				if (magdiff) {
					ret = magdiff;        /* compare # of significant digits*/
					break;
				}
				if (diff) {
					ret = diff;        /* compare first non-zero digit */
					break;
				}
			} else {
				if (cs) {
					/* compare case-sensitive */
					if ((ret = *a-*b)) break;
				} else {
					/* compare case-insensitve */
					if ((ret = tolower(*a & 255)-tolower(*b & 255))) break;
				}

				if (!*a) break;
				a++;
				b++;
			}
		}
		if (!ret) return 0;
		else return (ret < 0) ? -1 : 1;
	}

	/*
	 * 'fl_casenumericsort()' - Compare directory entries with case-sensitivity.
	 */

	int fl_casenumericsort(struct dirent **A, struct dirent **B)
	{
		return numericsort(A, B, 0);
	}

	/*
	 * 'fl_numericsort()' - Compare directory entries with case-sensitivity.
	 */

	int fl_numericsort(struct dirent **A, struct dirent **B)
	{
		return numericsort(A, B, 1);
	}

#  ifdef __cplusplus
}
#  endif

/*
 * End of "$Id: numericsort.c 8864 2011-07-19 04:49:30Z greg.ercolano $".
 */
