/* mnote-pentax-entry.h
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

#ifndef __MNOTE_PENTAX_ENTRY_H__
#define __MNOTE_PENTAX_ENTRY_H__

#include "../exif-format.h"
#include "../exif-byte-order.h"
#include "../pentax/mnote-pentax-tag.h"
#include "../exif-mnote-entry.h"
#include <stdio.h>

class MnotePentaxEntry:public ExifMnoteEntry
{


public:
	MnotePentaxEntry()
	{
		Init();
	}
	void inline Init()
	{
		tag=MNOTE_TAG_NULL0;
		format=EXIF_FORMAT_NULL;
		components=0;
		data=NULL;
		size=0;
		order = EXIF_BYTE_ORDER_MOTOROLA;
	}
	char *mnote_pentax_entry_get_value (char *val, unsigned int maxlen);
public:
	MnotePentaxTag tag;
	ExifFormat format;
	unsigned long components;

	unsigned char *data;
	unsigned int size;

	ExifByteOrder order;
};



#endif /* __MNOTE_PENTAX_ENTRY_H__ */
