/* mnote-fuji-entry.h
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

#ifndef __MNOTE_FUJI_ENTRY_H__
#define __MNOTE_FUJI_ENTRY_H__

#include "../exif-format.h"
#include "../fuji/mnote-fuji-tag.h"

typedef struct _MnoteFujiEntryPrivate MnoteFujiEntryPrivate;

#include "../fuji/exif-mnote-data-fuji.h"
#include "../exif-mnote-entry.h"

class MnoteFujiEntry :public ExifMnoteEntry
{
public:
	MnoteFujiEntry()
	{
		Init();
	}
	void inline Init()
	{
		tag=MNOTE_FUJI_TAG_VERSION;
		format=EXIF_FORMAT_NULL;
		components=0;
		data=NULL;
		size=0;
		order = EXIF_BYTE_ORDER_MOTOROLA;

	}

	char *mnote_fuji_entry_get_value (char *val, unsigned int maxlen);

public:
	MnoteFujiTag tag;
	ExifFormat format;
	unsigned long components;

	ExifByteOrder order;
};



#endif /* __MNOTE_FUJI_ENTRY_H__ */
