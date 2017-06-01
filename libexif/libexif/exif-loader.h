/*! \file exif-loader.h
 * \brief Defines the ExifLoader type
 */
/*
 * Copyright (c) 2003 Lutz Mueller <lutz@users.sourceforge.net>
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

#ifndef __EXIF_LOADER_H__
#define __EXIF_LOADER_H__

#include "exif-mem.h"
#include "exif-data.h"
#include "exif-log.h"



/*! Data used by the loader interface */
#undef JPEG_MARKER_DHT
#define JPEG_MARKER_DHT  0xc4
#undef JPEG_MARKER_SOI
#define JPEG_MARKER_SOI  0xd8
#undef JPEG_MARKER_DQT
#define JPEG_MARKER_DQT  0xdb
#undef JPEG_MARKER_APP0
#define JPEG_MARKER_APP0 0xe0
#undef JPEG_MARKER_APP1
#define JPEG_MARKER_APP1 0xe1
#undef JPEG_MARKER_APP2
#define JPEG_MARKER_APP2 0xe2
#undef JPEG_MARKER_APP13
#define JPEG_MARKER_APP13 0xed
#undef JPEG_MARKER_COM
#define JPEG_MARKER_COM 0xfe

typedef enum {
	EL_READ = 0,
	EL_READ_SIZE_BYTE_24,
	EL_READ_SIZE_BYTE_16,
	EL_READ_SIZE_BYTE_08,
	EL_READ_SIZE_BYTE_00,
	EL_SKIP_BYTES,
	EL_EXIF_FOUND,
} ExifLoaderState;

typedef enum {
	EL_DATA_FORMAT_UNKNOWN,
	EL_DATA_FORMAT_EXIF,
	EL_DATA_FORMAT_JPEG,
	EL_DATA_FORMAT_FUJI_RAW
} ExifLoaderDataFormat;

/*! \internal */
class ExifLoader 
{

public:
	ExifLoader()
	{
		Init();
	}

	void inline Init()
	{
		state = EL_READ;
		data_format= EL_DATA_FORMAT_UNKNOWN;
		buf=NULL;
		size=0;
		log=NULL;
		mem=NULL;
		bytes_read=0;
	}
	void exif_loader_get_data (ExifData *ed);
	void exif_loader_write_file (const char *path);
	unsigned char exif_loader_write (unsigned char *buf, unsigned int len);
	unsigned int exif_loader_copy (unsigned char *buf, unsigned int len);
	unsigned char* exif_loader_alloc (unsigned int i);
	void exif_loader_reset ();
	void exif_loader_new (ExifMem *mem);
	void exif_loader_get_buf (const unsigned char **buf,  unsigned int *buf_size);
	void exif_loader_log (ExifLog *log0);
	void exif_loader_free ();

public:
	ExifLoaderState state;
	ExifLoaderDataFormat data_format;

	/*! Small buffer used for detection of format */
	unsigned char b[12];

	/*! Number of bytes in the small buffer \c b */
	unsigned char b_len;

	unsigned int size;
	unsigned char *buf;
	unsigned int bytes_read;

	ExifLog *log;
	ExifMem *mem;
};

#endif /* __EXIF_LOADER_H__ */
