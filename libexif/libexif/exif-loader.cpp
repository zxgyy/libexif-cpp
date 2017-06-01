/* exif-loader.c
 *
 * Copyright (c) 2002 Lutz Mueller <lutz@users.sourceforge.net>
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

#include "exif-loader.h"
#include "exif-utils.h"
#include "i18n.h"

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef _MSC_VER
#ifndef _SSIZE_T_DEFINED
#define _SSIZE_T_DEFINED
#undef ssize_t
#ifdef _WIN64
typedef __int64 ssize_t;
#else
typedef int ssize_t;
#endif /* _WIN64 */
#endif /* _SSIZE_T_DEFINED */
#endif /* _MSC_VER */



/*! Magic number for EXIF header */
static const unsigned char ExifHeader[] = {0x45, 0x78, 0x69, 0x66, 0x00, 0x00};

unsigned char* ExifLoader::exif_loader_alloc (unsigned int i)
{
	unsigned char *d=NULL;

	if (!i)	return NULL;

	mem->exif_mem_alloc (&d,i/sizeof(unsigned char));
	if (d) 
		return d;

	EXIF_LOG_NO_MEMORY_PTR (log, "ExifLog", i);
	return NULL;
}


/*! Load a file into the given #ExifLoader from the filesystem.
 * The relevant data is copied in raw form into the #ExifLoader.
 *
 * \param[in] fname path to the file to read
 */
void ExifLoader::exif_loader_write_file (const char *path)
{
	FILE *f;
	int size;
	unsigned char data[1024];

	f = fopen (path, "rb");
	if (!f) {
		log->exif_log (EXIF_LOG_CODE_NONE, "ExifLoader",
			  _("The file '%s' could not be opened."), path);
		return;
	}
	while (1) {
		size = fread (data, 1, sizeof (data), f);
		if (size <= 0) 
			break;
		if (!exif_loader_write (data, size)) 
			break;
	}
	fclose (f);
}

unsigned int ExifLoader::exif_loader_copy (unsigned char *buf, unsigned int len)
{
	if ((len && !buf) || ( bytes_read >=  size)) 
		return 0;

	/* If needed, allocate the buffer. */
	if (! buf) 
		 buf = exif_loader_alloc (size);
	if (! buf) 
		return 0;

	/* Copy memory */
	len = MIN (len,  size -  bytes_read);
	memcpy ( buf +  bytes_read, buf, len);
	 bytes_read += len;

	return ( bytes_read >=  size) ? 0 : 1;
}

/*! Load a buffer into the #ExifLoader from a memory buffer.
 * The relevant data is copied in raw form into the #ExifLoader.
 *
 * \param[in] buf buffer to read from
 * \param[in] sz size of the buffer
 * \return 1 while EXIF data is read (or while there is still hope that
 *   there will be EXIF data later on), 0 otherwise.
 */
unsigned char ExifLoader::exif_loader_write (unsigned char *buf, unsigned int len)
{
	unsigned int i=0;

	if ((len && !buf)) 
		return 0;

	switch (state) {
	case EL_EXIF_FOUND:
		return exif_loader_copy (buf, len);
	case EL_SKIP_BYTES:
		if ( size > len) { 
			 size -= len; 
			return 1; 
		}
		len -=  size;
		buf +=  size;
		 size = 0;
		 b_len = 0;
		switch ( data_format) {
		case EL_DATA_FORMAT_FUJI_RAW:
			 state = EL_READ_SIZE_BYTE_24;
			break;
		default:
			 state = EL_READ;
			break;
		}
		break;

	case EL_READ:
	default:
		break;
	}

	if (!len)
		return 1;
	log->exif_log(EXIF_LOG_CODE_DEBUG, "ExifLoader",
		  "Scanning %i byte(s) of data...", len);

	/*
	 * First fill the small buffer. Only continue if the buffer
	 * is filled. Note that EXIF data contains at least 12 bytes.
	 */
	i = MIN (len, sizeof ( b) -  b_len);
	if (i) {
		memcpy (& b[ b_len], buf, i);
		 b_len += i;
		if ( b_len < sizeof ( b)) 
			return 1;
		buf += i;
		len -= i;
	}

	switch ( data_format) {
	case EL_DATA_FORMAT_UNKNOWN:

		/* Check the small buffer against known formats. */
		if (!memcmp ( b, "FUJIFILM", 8)) {

			/* Skip to byte 84. There is another offset there. */
			 data_format = EL_DATA_FORMAT_FUJI_RAW;
			 size = 84;
			 state = EL_SKIP_BYTES;
			 size = 84;

		} else if (!memcmp ( b + 2, ExifHeader, sizeof (ExifHeader))) {

			/* Read the size (2 bytes). */
			 data_format = EL_DATA_FORMAT_EXIF;
			 state = EL_READ_SIZE_BYTE_08;
		}
	default:
		break;
	}

	for (i = 0; i < sizeof ( b); i++)
		switch ( state) {
		case EL_EXIF_FOUND:
			if (!exif_loader_copy (b + i,
					sizeof ( b) - i)) 
				return 0;
			return exif_loader_copy (buf, len);
		case EL_SKIP_BYTES:
			 size--;
			if (! size) 
				 state = EL_READ;
			break;

		case EL_READ_SIZE_BYTE_24:
			 size |=  b[i] << 24;
			 state = EL_READ_SIZE_BYTE_16;
			break;
		case EL_READ_SIZE_BYTE_16:
			 size |=  b[i] << 16;
			 state = EL_READ_SIZE_BYTE_08;
			break;
		case EL_READ_SIZE_BYTE_08:
			 size |=  b[i] << 8;
			 state = EL_READ_SIZE_BYTE_00;
			break;
		case EL_READ_SIZE_BYTE_00:
			 size |=  b[i] << 0;
			switch ( data_format) {
			case EL_DATA_FORMAT_JPEG:
				 state = EL_SKIP_BYTES;
				 size -= 2;
				break;
			case EL_DATA_FORMAT_FUJI_RAW:
				 data_format = EL_DATA_FORMAT_EXIF;
				 state = EL_SKIP_BYTES;
				 size -= 86;
				break;
			case EL_DATA_FORMAT_EXIF:
				 state = EL_EXIF_FOUND;
				break;
			default:
				break;
			}
			break;

		default:
			switch ( b[i]) {
			case JPEG_MARKER_APP1:
			  if (!memcmp ( b + i + 3, ExifHeader, MIN((ssize_t)(sizeof(ExifHeader)), MAX(0, ((ssize_t)(sizeof( b))) - ((ssize_t)i) - 3)))) {
					 data_format = EL_DATA_FORMAT_EXIF;
				} else {
					 data_format = EL_DATA_FORMAT_JPEG; /* Probably JFIF - keep searching for APP1 EXIF*/
				}
				 size = 0;
				 state = EL_READ_SIZE_BYTE_08;
				break;
			case JPEG_MARKER_DHT:
			case JPEG_MARKER_DQT:
			case JPEG_MARKER_APP0:
			case JPEG_MARKER_APP2:
			case JPEG_MARKER_APP13:
			case JPEG_MARKER_COM:
				 data_format = EL_DATA_FORMAT_JPEG;
				 size = 0;
				 state = EL_READ_SIZE_BYTE_08;
				break;
			case 0xff:
			case JPEG_MARKER_SOI:
				break;
			default:
				log->exif_log(EXIF_LOG_CODE_CORRUPT_DATA,
					"ExifLoader", _("The data supplied "
						"does not seem to contain "
						"EXIF data."));
				exif_loader_reset ();
				return 0;
			}
		}

	/*
	 * If we reach this point, the buffer has not been big enough
	 * to read all data we need. Fill it with new data.
	 */
	 b_len = 0;
	return exif_loader_write (buf, len);
}

/*! Allocate a new #ExifLoader.
 *
 *  \return allocated ExifLoader
 */
void ExifLoader::exif_loader_new (ExifMem *mem0)
{
	if (!mem0) 
		return ;

	Init();

	mem = mem0;
	return ;
}


void ExifLoader::exif_loader_free ()
{

	exif_loader_reset();
	if (log)
	{
		log=NULL;
	}
	if (mem)
	{
		delete mem;
		mem=NULL;
	}
	
}

/*! Free any data previously loaded and reset the #ExifLoader to its
 * newly-initialized state.
 *
 * \param[in] loader the loader
 */
void ExifLoader::exif_loader_reset ()
{
	size>1?delete [] buf:delete buf;
	buf=NULL;
	size = 0;
	bytes_read = 0;
	state = EL_READ;
	b_len = 0;
	data_format = EL_DATA_FORMAT_UNKNOWN;
}

/*! Create an #ExifData from the data in the loader. The loader must
 * already contain data from a previous call to #exif_loader_write_file
 * or #exif_loader_write.
 *
 * \note The #ExifData returned is created using its default options, which
 * may take effect before the data is returned. If other options are desired,
 * an #ExifData must be created explicitly and data extracted from the loader
 * using #exif_loader_get_buf instead.
 *
 * \param[in] loader the loader
 * \return allocated ExifData
 *
 * \see exif_loader_get_buf
 */
void ExifLoader::exif_loader_get_data (ExifData *ed)
{
	if (!ed || (data_format == EL_DATA_FORMAT_UNKNOWN) || !bytes_read)
		return ;

	ed->exif_data_new ();
	ed->exif_data_log ();
	ed->exif_data_load_data (buf, bytes_read);

	return ;
}
/*! Return the raw data read by the loader.  The returned pointer is only
 * guaranteed to be valid until the next call to a function modifying
 * this #ExifLoader.  Either or both of buf and buf_size may be NULL on
 * entry, in which case that value is not returned.
 *
 * \param[out] buf read-only pointer to the data read by the loader, or NULL
 *                 in case of error
 * \param[out] buf_size size of the data at buf, or 0 in case of error
 */
void ExifLoader::exif_loader_get_buf (const unsigned char **buf0,  unsigned int *buf_size)
{
	const unsigned char* b = NULL;
	unsigned int s = 0;

	if ((data_format == EL_DATA_FORMAT_UNKNOWN)) {
		log->exif_log ( EXIF_LOG_CODE_DEBUG, "ExifLoader",
			  "Loader format unknown");
	} else {
		b = buf;
		s = bytes_read;
	}
	if (buf0)
		*buf0 = b;
	if (buf_size)
		*buf_size = s;
}

/*! Set the log message object used by this #ExifLoader.
 * \param[in] log #ExifLog
 */
void ExifLoader::exif_loader_log (ExifLog *log0)
{
	if (log)
	{
		log=NULL;
	}
	
	log = log0;
}
