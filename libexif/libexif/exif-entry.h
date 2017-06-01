/*! \file exif-entry.h
 *  \brief Handling EXIF entries
 */
/*
 * Copyright (c) 2001 Lutz Mueller <lutz@users.sourceforge.net>
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

#ifndef __EXIF_ENTRY_H__
#define __EXIF_ENTRY_H__

#include <stdio.h>


#include "exif-mem.h"
#include "exif-content.h"
#include "exif-format.h"


class ExifContent;

class ExifEntryPrivate
{

public:
	ExifEntryPrivate()
	{
		mem=NULL;
		
	}
	ExifEntryPrivate& operator=(const ExifEntryPrivate& input)
	{
		mem=input.mem;
		return *this;
	}
public:
	ExifMem *mem;

};

/*! Data found in one EXIF tag */
class ExifEntry{
public:
	ExifEntry(ExifContent *EC)
	{
		Init();
		parent=EC;
	}
	ExifEntry()
	{
		Init();
		
	}
	ExifEntry(const ExifEntry &input)
	{  
		if (input.data)
		{
			size=input.size;
			data=new unsigned char[size];
			memcpy(data,input.data,size);
		}
		priv=input.priv;
		tag=input.tag;
		format=input.format;
		components=input.components;
		parent=input.parent;
		tag=input.tag;
	} 

	~ExifEntry()
	{
		exif_entry_free();
	}
	void inline Init()
	{
		parent=NULL;
		data=NULL;
		size=0;
		tag=EXIF_TAG_NULL;
		format=EXIF_FORMAT_NULL;
		components=0;
	}
private:
	void inline data_free()
	{
		if (data)
		{
			size>1?delete [] data:delete data;
			data=NULL;
		}
		size=0;
	}
 
public:
	ExifIfd exif_entry_get_ifd();
	unsigned char *exif_entry_alloc (unsigned int i);
	unsigned char *exif_entry_realloc (unsigned char *d_orig, unsigned int i);
	void exif_entry_free();
	ExifShort exif_get_short_convert (const unsigned char *buf,
										ExifFormat format,
										ExifByteOrder order);
	void exif_entry_fix ();
#ifndef NO_VERBOSE_TAG_STRINGS
	void exif_entry_log (ExifLogCode code, const char *format, ...);
#endif
	void exif_entry_format_value(char *val, size_t maxlen);
	void exif_entry_dump (unsigned int indent);
	int match_repeated_char(const unsigned char *data, unsigned char ch, size_t n);
	const char *exif_entry_get_value(char *val, unsigned int maxlen);
	void exif_entry_initialize (ExifTag tag);
public:
	/*! EXIF tag for this entry */
        ExifTag tag;
	
	/*! Type of data in this entry */
        ExifFormat format;

	/*! Number of elements in the array, if this is an array entry.
	 * Contains 1 for non-array data types. */
        unsigned long components;

	/*! Pointer to the raw EXIF data for this entry. It is allocated
	 * by #exif_entry_initialize and is NULL beforehand. Data contained
	 * here may be manipulated using the functions in exif-utils.h */
        unsigned char *data;

	/*! Number of bytes in the buffer at \c data. This must be no less
	 * than exif_format_get_size(format)*components */
        unsigned int size;

	/*! #ExifContent containing this entry. 
	 * \see exif_entry_get_ifd */
	ExifContent *parent;

	/*! Internal data to be used by libexif itself */
	ExifEntryPrivate priv;
};

#endif /* __EXIF_ENTRY_H__ */
