/* mnote-olympus-data.h
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

#ifndef __MNOTE_OLYMPUS_CONTENT_H__
#define __MNOTE_OLYMPUS_CONTENT_H__

#include "../exif-mem.h"
#include "../olympus/mnote-olympus-entry.h"
#include "../exif-byte-order.h"
#include "../exif-data.h"
#include <stdio.h>

enum OlympusVersion {
	unrecognized = 0,
	nikonV1 = 1,
	nikonV2 = 2,
	olympusV1 = 3,
	olympusV2 = 4,
	sanyoV1 = 5,
	epsonV1 = 6,
	nikonV0 = 7
};
class ExifMnoteData;

class ExifMnoteDataOlympus:public ExifMnoteData 
{

public:
	ExifMnoteDataOlympus()
	{
		Init();
	}
	void inline Init()
	{
		entries=NULL;
		count=0;
		offset=0;
		order = EXIF_BYTE_ORDER_MOTOROLA;
		version=unrecognized;
	}
	~ExifMnoteDataOlympus()
	{
		exif_mnote_data_olympus_clear ();
	}
	void exif_mnote_data_olympus_clear ();
	void free ();
	char *get_value (unsigned int i, char *val, unsigned int maxlen);
	void save (unsigned char **buf, unsigned int *buf_size);
	void load (const unsigned char *buf, unsigned int buf_size);
	unsigned int get_count ();
	unsigned int get_id (unsigned int n);
	const char * get_name (unsigned int i);
	const char * get_title (unsigned int i);
	const char *get_description (unsigned int i);
	void set_byte_order (ExifByteOrder o);
	void set_offset (unsigned int o);
public:
	MnoteOlympusEntry *entries;
	unsigned int count;

	ExifByteOrder order;
	unsigned int offset;
	enum OlympusVersion version;
};

enum OlympusVersion exif_mnote_data_olympus_identify_variant (const unsigned char *buf,unsigned int buf_size);

#endif /* __MNOTE_OLYMPUS_CONTENT_H__ */
