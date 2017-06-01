/*! \file exif-data.h
 * \brief Defines the ExifData type and the associated functions.
 */
/*
 * \author Lutz Mueller <lutz@users.sourceforge.net>
 * \date 2001-2005
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

#ifndef __EXIF_DATA_H__
#define __EXIF_DATA_H__

#pragma warning(disable:4996)

#include "exif-byte-order.h"
#include "exif-data-type.h"
#include "exif-ifd.h"
#include "exif-log.h"
#include "exif-tag.h"
#include "exif-data.h"
#include "exif-entry.h"


#include "exif-mem.h"
#include "exif-content.h"
#include "exif-mnote-data.h"

class ExifContent;
class ExifEntry;
class ExifDataPrivate;


typedef void (* ExifDataForeachContentFunc) (ExifContent *, void *user_data);



/*! Options to configure the behaviour of #ExifData */
typedef enum {
	EXIF_DATA_OPTION_IGNORE_UNKNOWN     = 0,
	/*! Act as though unknown tags are not present */
	EXIF_DATA_OPTION_IGNORE_UNKNOWN_TAGS = 1 << 0,

	/*! Fix the EXIF tags to follow the spec */
	EXIF_DATA_OPTION_FOLLOW_SPECIFICATION = 1 << 1,

	/*! Leave the MakerNote alone, which could cause it to be corrupted */
	EXIF_DATA_OPTION_DONT_CHANGE_MAKER_NOTE = 1 << 2
} ExifDataOption;

class  ExifDataPrivate
{
public:
	ExifDataPrivate()
	{
		Init();
	}
	~ExifDataPrivate();

	void inline Init()
	{
		order=EXIF_BYTE_ORDER_MOTOROLA;
		md=NULL;
		options=EXIF_DATA_OPTION_IGNORE_UNKNOWN;
		data_type=EXIF_DATA_TYPE_UNCOMPRESSED_CHUNKY;
	}

	virtual void inline data_free()
	{
		if (md)
		{
			delete md;
			md=NULL;
		}

	}
	void exif_data_set_option(ExifDataOption Typex);
	int exif_data_load_data_entry (ExifEntry *entry,
		const unsigned char *d,
		unsigned int size, unsigned int offset);
	unsigned char *exif_data_alloc (unsigned int i);
	void exif_data_save_data_entry (ExifEntry *e,
		unsigned char **d, unsigned int *ds,
		unsigned int offset);

public:
	ExifByteOrder order;

	ExifMnoteData *md;

	ExifLog log;
	ExifMem mem;

	/* Temporarily used while loading data */
	unsigned int offset_mnote;

	ExifDataOption options;
	ExifDataType data_type;
};

/*! Represents the entire EXIF data found in an image */
class ExifData
{
public:
	ExifData();
	~ExifData();
public:
	void exif_data_new_from_data(const unsigned char *data, unsigned int size);
	void Init();
	ExifMnoteData *exif_data_get_mnote_data ();
	ExifEntry *exif_data_get_entry(ExifTag t);
	void exif_data_free();
	void exif_data_new ();
	void exif_data_dump ();
	void exif_data_set_option(ExifDataOption Typex);
	void exif_data_set_data_type (ExifDataType dt);
	void exif_data_load_data (const unsigned char *d_orig,unsigned int ds);
	void exif_data_load_data_content (ExifIfd ifd,
		const unsigned char *d,
		unsigned int ds, unsigned int offset, unsigned int recursion_depth);
	void exif_data_load_data_thumbnail (const unsigned char *d,
				unsigned int ds, ExifLong o, ExifLong s);
	void exif_data_save_data (unsigned char **d, unsigned int *ds);
	void exif_data_save_data_content (ExifContent *ifd0,
		unsigned char **d, unsigned int *ds,
		unsigned int offset);
	void exif_data_save_data_entry (ExifEntry *e,
		unsigned char **d, unsigned int *ds,
		unsigned int offset);
	void interpret_maker_note(const unsigned char *d, unsigned int ds);
	void exif_data_foreach_content (ExifDataForeachContentFunc func, void *user_data);
	void exif_data_fix ();
	void exif_data_unset_option(ExifDataOption o);
	ExifDataType exif_data_get_data_type ();
	const char *exif_data_option_get_name (ExifDataOption o);
	const char *exif_data_option_get_description (ExifDataOption o);
	ExifLog *exif_data_get_log ();
	void exif_data_new_from_file (const char *path);
	void exif_data_log ();
	ExifByteOrder exif_data_get_byte_order ();
	void exif_data_set_byte_order (ExifByteOrder order);
public:
	//Camera Type
	int exif_mnote_data_olympus_identify (const ExifEntry *e);
	
	int exif_mnote_data_fuji_identify (const ExifEntry *e);
	int  exif_mnote_data_canon_identify (const ExifEntry *e);
	int exif_mnote_data_pentax_identify (const ExifEntry *e);
	void entry_set_byte_order (ExifEntry *e, void *data);
public:
	/*! Data for each IFD */
	ExifContent *ifd[EXIF_IFD_COUNT];

	/*! Pointer to thumbnail image, or NULL if not available */
	unsigned char *data;

	/*! Number of bytes in thumbnail image at \c data */
	unsigned int size;

	ExifDataPrivate priv;

};



#endif /* __EXIF_DATA_H__ */
