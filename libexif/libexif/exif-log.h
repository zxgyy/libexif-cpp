/*! \file exif-log.h
 *  \brief Log message infrastructure
 */
/*
 * Copyright (c) 2004 Lutz Mueller <lutz@users.sourceforge.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details. 
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301  USA.
 */

#ifndef __EXIF_LOG_H__
#define __EXIF_LOG_H__


#include "exif-mem.h"
#include <stdarg.h>


typedef enum {
	EXIF_LOG_CODE_NULL = 0x0000,
	EXIF_LOG_CODE_NONE = 0x0001,
	EXIF_LOG_CODE_DEBUG= 0x0002,
	EXIF_LOG_CODE_NO_MEMORY= 0x0003,
	EXIF_LOG_CODE_CORRUPT_DATA= 0x0004

} ExifLogCode;






/*! Log callback function prototype.
 */
typedef void (* ExifLogFunc) (ExifLogCode, const char *domain,
			      const char *format, va_list args, void *data);

class ExifLog 
{
public:
	ExifLog()
	{
		Init();
	}
	void inline Init()
	{
		func=NULL;
		data = NULL;
	}
	~ExifLog()
	{
	}
	void exif_log_set_func (ExifLogFunc func0, void *data0);
	const char *exif_log_code_get_title  (ExifLogCode code);
	const char *exif_log_code_get_message (ExifLogCode code);

#ifndef NO_VERBOSE_TAG_STRINGS
	void exif_log  (ExifLogCode, const char *domain,const char *format, ...);
#endif
	void exif_logv (ExifLogCode, const char *domain, const char *format, va_list args);
public:

	ExifLogFunc func;
	void *data;
};



/* For your convenience */
#define EXIF_LOG_NO_MEMORY(l,d,s) l.exif_log (EXIF_LOG_CODE_NO_MEMORY, (d), "Could not allocate %lu byte(s).", (unsigned long)(s))
#define EXIF_LOG_NO_MEMORY_PTR(l,d,s) l->exif_log (EXIF_LOG_CODE_NO_MEMORY, (d), "Could not allocate %lu byte(s).", (unsigned long)(s))


#endif /* __EXIF_LOG_H__ */
