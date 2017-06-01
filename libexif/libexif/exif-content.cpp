/* exif-content.c
 *
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

#include "config.h"

#include "exif-content.h"
#include "exif-system.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* unused constant
 * static const unsigned char ExifHeader[] = {0x45, 0x78, 0x69, 0x66, 0x00, 0x00};
 */
/*! Return the #ExifEntry in this IFD corresponding to the given tag.
 * This is a pointer into a member of the #ExifContent array and must NOT be
 * freed or unrefed by the caller.
 *
 * \param[in] content EXIF content for an IFD
 * \param[in] tag EXIF tag to return
 * \return #ExifEntry of the tag, or NULL on error
 */
ExifEntry *ExifContent::exif_content_get_entry (ExifTag tag)
{
	for (std::vector<ExifEntry>::iterator it=entries.begin(); it != entries.end(); ++it)
		if (it->tag == tag)
			return &(*it);
	return (NULL);
}
ExifContent::~ExifContent()
{
	exif_content_free();
}

void ExifContent::exif_content_free ()
{
	unsigned int i=0;

	//for (std::vector<ExifEntry>::iterator it=entries.begin(); it != entries.end(); ++it)
	//{
	//	it->exif_entry_free();
	//}
	entries.clear();
}
/*! Dump contents of the IFD to stdout.
 * This is intended for diagnostic purposes only.
 *
 * \param[in] content IFD data
 * \param[in] indent how many levels deep to indent the data
 */
void ExifContent::exif_content_dump (unsigned int indent)
{
	char buf[1024];
	unsigned int i;

	for (i = 0; i < 2 * indent; i++)
		buf[i] = ' ';
	buf[i] = '\0';


	printf ("%sDumping exif content (%u entries)...\n", buf, entries.size());
	for (std::vector<ExifEntry>::iterator it=entries.begin(); it != entries.end(); ++it)
	{
		it->exif_entry_dump (indent + 1);
	}
}

void ExifContent::exif_content_add_entry (ExifEntry &ee)
{
	ee.parent=this;
	ee.priv.mem=priv.mem;
	/* One tag can only be added once to an IFD. */
	if (exif_content_get_entry (ee.tag)) {
		priv.log->exif_log(EXIF_LOG_CODE_DEBUG, "ExifContent",
			"An attempt has been made to add "
			"the tag '%s' twice to an IFD. This is against "
			"specification.", exif_tag_get_name (ee.tag));
		return;
	}

	entries.push_back(ee);
	
}
/*! Remove an EXIF tag from an IFD.
 * If this tag does not exist in the IFD, this function does nothing.
 *

 * \param[in] index indexed EXIF entry to remove
 */
void ExifContent::exif_content_remove_entry (unsigned int index)
{
	unsigned int i=0;

	if (index == entries.size()) return;
			

	/* Remove the entry */
	if (entries.size() > 1) 
	{
		entries[index].exif_entry_free();
		if (index == (entries.size()-1))
		{
			entries[index].exif_entry_free();
			entries.pop_back();
		}
		else
		{
			entries[index].exif_entry_free();
			int i=0;
			for (std::vector<ExifEntry>::iterator it=entries.begin(); it!= entries.end(); )
			{
				++i;
				++it;
				if (i==index)
				{
					it->exif_entry_free();
					entries.erase(it);
				}
				
				
			}
			
			
		}
		
		
		
	}
}

/*! Executes function on each EXIF tag in this IFD in turn.
 * The tags will not necessarily be visited in numerical order.
 *
 * \param[in,out] content IFD over which to iterate
 * \param[in] func function to call for each entry
 * \param[in] user_data data to pass into func on each call
 */
void ExifContent::exif_content_foreach_entry ()
{
	remove_not_recorded();
	
		
}
/*! Set the log message object for this IFD.
 *
 * \param[in] content IFD
 * \param[in] log #ExifLog*
 */
void ExifContent::exif_content_log_mem (ExifLog *log,ExifMem *mem)
{
	if (log)
	{
		priv.log = log;
	}
	if (mem)
	{
		priv.mem = mem;
	}
	

	

	
}

ExifIfd ExifContent::exif_content_get_ifd ()
{
	if (!parent) return EXIF_IFD_COUNT;

	return 
		(parent->ifd[EXIF_IFD_EXIF] == this) ? EXIF_IFD_EXIF :
		(parent->ifd[EXIF_IFD_0] == this) ? EXIF_IFD_0 :
		(parent->ifd[EXIF_IFD_1] == this) ? EXIF_IFD_1 :
		(parent->ifd[EXIF_IFD_GPS] == this) ? EXIF_IFD_GPS :
		(parent->ifd[EXIF_IFD_INTEROPERABILITY] == this) ? EXIF_IFD_INTEROPERABILITY :
		EXIF_IFD_COUNT;
}

static void
fix_func (ExifEntry *e, void *UNUSED(data))
{
	e->exif_entry_fix ();
}

/*!
 * Check if this entry is unknown and if so, delete it.
 * \note Be careful calling this function in a loop. Deleting an entry from
 * an ExifContent changes the index of subsequent entries, as well as the
 * total size of the entries array.
 */
void ExifContent::remove_not_recorded ()
{
	for (std::vector<ExifEntry>::iterator it = entries.begin(); it != entries.end(); ++it)
	{
		ExifIfd ifd = exif_content_get_ifd();
		ExifContent *c = it->parent;
		ExifDataType dt = c->parent->exif_data_get_data_type ();
		ExifTag t = it->tag;

		if (exif_tag_get_support_level_in_ifd (t, ifd, dt) ==
			EXIF_SUPPORT_LEVEL_NOT_RECORDED) {
				c->priv.log->exif_log (EXIF_LOG_CODE_DEBUG, "exif-content",
					"Tag 0x%04x is not recorded in IFD '%s' and has therefore been "
					"removed.", t, exif_ifd_get_name (ifd));
				it->exif_entry_free();
				it=entries.erase(it);
		}
	}
	

}
/*! Fix the IFD to bring it into specification. Call #exif_entry_fix on
 * each entry in this IFD to fix existing entries, create any new entries
 * that are mandatory in this IFD but do not yet exist, and remove any
 * entries that are not allowed in this IFD.
 *
 * \param[in,out] c EXIF content for an IFD
 */
void ExifContent::exif_content_fix ()
{
	ExifIfd ifd = exif_content_get_ifd ();
	ExifDataType dt;
	unsigned int i, num;

	dt = parent->exif_data_get_data_type ();

	/*
	 * First of all, fix all existing entries.
	 */
	exif_content_foreach_entry ();

	/*
	 * Go through each tag and if it's not recorded, remove it. If one
	 * is removed, exif_content_foreach_entry() will skip the next entry,
	 * so if this happens do the loop again from the beginning to ensure
	 * they're all checked. This could be avoided if we stop relying on
	 * exif_content_foreach_entry but loop intelligently here.
	 */
	exif_content_foreach_entry ();

	/*
	 * Then check for non-existing mandatory tags and create them if needed
	 */
	num = exif_tag_table_count();
	for (i = 0; i < num; ++i) {
		const ExifTag t = exif_tag_table_get_tag (i);
		if (exif_tag_get_support_level_in_ifd (t, ifd, dt) ==
			EXIF_SUPPORT_LEVEL_MANDATORY) {
			if (exif_content_get_entry (t))
				/* This tag already exists */
				continue;
			priv.log->exif_log(EXIF_LOG_CODE_DEBUG, "exif-content",
					"Tag '%s' is mandatory in IFD '%s' and has therefore been added.",
					exif_tag_get_name_in_ifd (t, ifd), exif_ifd_get_name (ifd));
			ExifEntry en;
			exif_content_add_entry (en);
			en.exif_entry_initialize (t);
		}
	}
}
