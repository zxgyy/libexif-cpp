/* exif-log.c
 *
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

#include "config.h"

#include "exif-log.h"
#include "i18n.h"

#include <stdlib.h>
#include <string.h>



static const struct {
	ExifLogCode code;
	const char *title;
	const char *message;
} codes[] = {
	{ EXIF_LOG_CODE_DEBUG, N_("Debugging information"),
	  N_("Debugging information is available.") },
	{ EXIF_LOG_CODE_NO_MEMORY, N_("Not enough memory"),
	  N_("The system cannot provide enough memory.") },
	{ EXIF_LOG_CODE_CORRUPT_DATA, N_("Corrupt data"),
	  N_("The data provided does not follow the specification.") },
	{ EXIF_LOG_CODE_NULL, NULL, NULL }
};

/*! Return a textual description of the given class of error log.
 *
 * \param[in] code logging message class
 * \return textual description of the log class
 */
const char *ExifLog::exif_log_code_get_title (ExifLogCode code)
{
	unsigned int i;

	for (i = 0; codes[i].title; i++) if (codes[i].code == code) break;
	return _(codes[i].title);
}

/*! Return a verbose description of the given class of error log.
 *
 * \param[in] code logging message class
 * \return verbose description of the log class
 */
const char *ExifLog::exif_log_code_get_message (ExifLogCode code)
{
	unsigned int i;

	for (i = 0; codes[i].message; i++) if (codes[i].code == code) break;
	return _(codes[i].message);
}


/*! Register log callback function.
 * Calls to the log callback function are purely for diagnostic purposes.
 *
 * \param[in] log logging state variable
 * \param[in] func callback function to set
 * \param[in] data data to pass into callback function
 */
void ExifLog::exif_log_set_func (ExifLogFunc func0, void *data0)
{
	func = func0;
	data = data0;
}

#ifndef NO_VERBOSE_TAG_STRINGS
void ExifLog::exif_log (ExifLogCode code, const char *domain,
	  const char *format, ...)
{
	va_list args;

	va_start (args, format);
	exif_logv (code, domain, format, args);
	va_end (args);
}
#endif

void ExifLog::exif_logv (ExifLogCode code, const char *domain,
	   const char *format, va_list args)
{
	if (!func) return;
	func(code, domain, format, args, data);
}
