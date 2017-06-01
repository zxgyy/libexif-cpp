/* exif-mnote-data-fuji.h
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

#ifndef __MNOTE_FUJI_CONTENT_H__
#define __MNOTE_FUJI_CONTENT_H__

#include "../exif-mnote-data.h"
#include "../exif-data.h"
#include "../fuji/mnote-fuji-entry.h"

class MnoteFujiEntry;

class ExifMnoteDataFuji:public ExifMnoteData 
{
public:
	~ExifMnoteDataFuji()
	{
		exif_mnote_data_fuji_clear ();
	}
	void exif_mnote_data_fuji_clear ();
	void free ();
	char *get_value (unsigned int i, char *val, unsigned int maxlen);
	void save (unsigned char **buf, unsigned int *buf_size);
	void load (const unsigned char *buf, unsigned int buf_size);
	unsigned int get_count ();
	unsigned int get_id (unsigned int n);
	const char *get_name (unsigned int i);
	const char *get_title (unsigned int i);
	const char *get_description (unsigned int i);
	void set_byte_order (ExifByteOrder o);
	void set_offset (unsigned int o);
public:
	MnoteFujiEntry *entries;
	unsigned int count;

	ExifByteOrder order;
	unsigned int offset;
};

/*! Detect if MakerNote is recognized as one handled by the Fuji module.
 * 
 * \param[in] ed image #ExifData to identify as as a Fuji type
 * \param[in] e #ExifEntry for EXIF_TAG_MAKER_NOTE, from within ed but
 *   duplicated here for convenience
 * \return 0 if not recognized, nonzero if recognized. The specific nonzero 
 *   value returned may identify a subtype unique within this module.
 */
int exif_mnote_data_fuji_identify (const ExifData *ed, const ExifEntry *e);

#endif /* __MNOTE_FUJI_CONTENT_H__ */
