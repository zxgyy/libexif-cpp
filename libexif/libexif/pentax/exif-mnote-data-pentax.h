/* exif-mnote-data-pentax.h
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

#ifndef __EXIF_MNOTE_DATA_PENTAX_H__
#define __EXIF_MNOTE_DATA_PENTAX_H__


#include "../exif-mem.h"
#include "../exif-byte-order.h"
#include "../exif-mnote-data.h"
#include "../pentax/mnote-pentax-entry.h"
#include "../exif-data.h"


enum PentaxVersion {pentaxV1 = 1, pentaxV2 = 2, pentaxV3 = 3, casioV2 = 4 };

class ExifMnoteDataPentax:public ExifMnoteData 
{
public:
	ExifMnoteDataPentax()
	{
		Init();
	}
	void inline Init()
	{
		count=0;
		entries=NULL;
		offset=0;
		order=EXIF_BYTE_ORDER_MOTOROLA;
		offset=0;
	}
	~ExifMnoteDataPentax()
	{
		exif_mnote_data_pentax_clear();
	}
	void exif_mnote_data_pentax_clear ();
	void free ();
	char *get_value (unsigned int i, char *val, unsigned int maxlen);
	void save (unsigned char **buf, unsigned int *buf_size);
	void load (const unsigned char *buf, unsigned int buf_size);
	unsigned int get_count();
	unsigned int get_id (unsigned int n);
	const char *get_name (unsigned int n);
	const char *get_title (unsigned int n);
	const char *get_description (unsigned int n);
	void set_offset (unsigned int o);
	void set_byte_order (ExifByteOrder o);
public:
	MnotePentaxEntry *entries;
	unsigned int count;

	ExifByteOrder order;
	unsigned int offset;

	enum PentaxVersion version;


};

#endif /* __EXIF_MNOTE_DATA_PENTAX_H__ */
