/*! \file exif-mnote-entry.h
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

#ifndef __EXIF_MNOTEENTRY_H__
#define __EXIF_MNOTEENTRY_H__

#include <stdio.h>


#include "exif-mem.h"
#include "exif-content.h"
#include "exif-format.h"


class ExifContent;

class ExifMnoteEntry 
{

public:
	ExifMnoteEntry()
	{
		data=NULL;
		size=0;
	}
	~ExifMnoteEntry()
	{
		data_free();
	}
	void data_free();
	void data_new(unsigned int s);
public:
	unsigned char *data;
	unsigned int size;

};

#endif /* __EXIF_MNOTEENTRY_H__ */
