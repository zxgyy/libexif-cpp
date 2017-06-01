/*! \file exif-content.h
 *  \brief Handling EXIF IFDs
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

#ifndef __EXIF_CONTENT_H__
#define __EXIF_CONTENT_H__

#include <vector>
class ExifData;



#include "exif-mem.h"
#include "exif-tag.h"
#include "exif-data.h"
#include "exif-entry.h"
#include "exif-log.h"

class ExifEntry;
typedef void (* ExifContentForeachEntryFunc) (ExifEntry *, void *user_data);

class ExifContentPrivate
{
public:
	ExifContentPrivate()
	{
		log=NULL;
		mem=NULL;
		Init();
	}
	void inline Init()
	{
		if (mem)
		{
			delete mem;
			mem=NULL;
		}
		
		log=NULL;
	}
	~ExifContentPrivate()
	{
		
	}
public:

	ExifMem *mem;
	ExifLog *log;

};

class ExifContent
{

public:
	ExifContent(ExifData *ED)
	{
		parent=ED;
		Init();
	}
	~ExifContent();
	void inline Init()
	{
		priv.Init();
	}
	ExifEntry *exif_content_get_entry (ExifTag tag);
	void exif_content_add_entry (ExifEntry &ee);
	void exif_content_fix ();
	ExifIfd exif_content_get_ifd ();
	void exif_content_log_mem (ExifLog *log,ExifMem *mem);
	void exif_content_foreach_entry ();
	void exif_content_remove_entry (unsigned int index);
	void exif_content_dump (unsigned int indent);
	void exif_content_free ();
	void remove_not_recorded ();
public:
    std::vector<ExifEntry> entries;

	/*! Data containing this content */
	ExifData *parent;

	ExifContentPrivate priv;
	
};

#endif /* __EXIF_CONTENT_H__ */
