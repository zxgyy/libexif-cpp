/* exif-mnote-data-canon.h
 *
 * Copyright (c) 2002, 2003 Lutz Mueller <lutz@users.sourceforge.net>
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

#ifndef __EXIF_MNOTE_DATA_CANON_H__
#define __EXIF_MNOTE_DATA_CANON_H__

#include "../exif-mem.h"
#include "../exif-byte-order.h"
#include "../exif-mnote-data.h"
#include "../exif-data.h"
#include "../canon/mnote-canon-entry.h"

class ExifMnoteDataCanon:public ExifMnoteData 
{
public:
	ExifMnoteDataCanon()
	{
		Init();
	}

	void inline Init()
	{
		count=0;
		entries=NULL;
		options=EXIF_DATA_OPTION_IGNORE_UNKNOWN;
		order=EXIF_BYTE_ORDER_MOTOROLA;
		offset=0;
	}
	~ExifMnoteDataCanon()
	{
		exif_mnote_data_canon_clear ();
	}
public:
	void exif_mnote_data_canon_clear ();
	void exif_mnote_data_canon_get_tags (unsigned int n,unsigned int *m, unsigned int *s);
public:
	void free();
	void set_byte_order(ExifByteOrder o);
	void set_offset (unsigned int o);
	void save(unsigned char **buf, unsigned int *buf_size);
	void load (const unsigned char *buf, unsigned int buf_size);
	unsigned int get_count ();
	unsigned int get_id (unsigned int i);
	const char * get_name (unsigned int i);
	const char * get_title (unsigned int i);
	const char * get_description (unsigned int i);
	char * get_value (unsigned int n, char *val, unsigned int maxlen);
public:
	MnoteCanonEntry *entries;
	unsigned int count;

	ExifByteOrder order;
	unsigned int offset;

	ExifDataOption options;

	
};

#endif /* __EXIF_MNOTE_DATA_CANON_H__ */
