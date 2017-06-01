/* exif-mnote-data.c
 *
 * Copyright (C) 2003 Lutz Mueller <lutz@users.sourceforge.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
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

#include "exif-mnote-data.h"

#include <stdlib.h>
#include <string.h>



void ExifMnoteData::exif_mnote_data_construct (ExifMem *mem0)
{
	if (!mem0) return;
	mem = mem0;
}


void ExifMnoteData::exif_mnote_data_free ()
{
	if (log)
	{
		log=NULL;
	}
	if (mem)
	{
		mem=NULL;
	}
	
	
}


/*! Load the MakerNote data from a memory buffer.
 *
 * \param[in] buf pointer to raw MakerNote tag data
 * \param[in] buf_siz number of bytes of data at buf
 */
void ExifMnoteData::exif_mnote_data_load (const unsigned char *buf,
		      unsigned int buf_size)
{
	load (buf, buf_size);
}
/*!
 * Save the raw MakerNote data into a memory buffer.  The buffer is
 * allocated by this function and must subsequently be freed by the
 * caller.
 *
 * \param[in,out] d extract the data from this structure 
 * \param[out] buf pointer to buffer pointer containing MakerNote data on return
 * \param[out] buf_siz pointer to the size of the buffer
 */
void ExifMnoteData::exif_mnote_data_save (unsigned char **buf,
		      unsigned int *buf_size)
{
	save (buf, buf_size);
}

void ExifMnoteData::exif_mnote_data_set_byte_order (ExifByteOrder o)
{
	set_byte_order (o);
}

void ExifMnoteData::exif_mnote_data_set_offset (unsigned int o)
{
	set_offset (o);
}

/*! Return the number of tags in the MakerNote.
 * \return number of tags, or 0 if no MakerNote or the type is not supported
 */
unsigned int ExifMnoteData::exif_mnote_data_count ()
{
	return get_count ();
}

/*! Return the MakerNote tag number for the tag at the specified index within
 * the MakerNote.
 *
 * \param[in] n index of the entry within the MakerNote data
 * \return MakerNote tag number
 */
unsigned int ExifMnoteData::exif_mnote_data_get_id (unsigned int n)
{
	return get_id (n);
}

/*! Returns textual name of the given MakerNote tag. The name is a short,
 * unique (within this type of MakerNote), non-localized text string
 * containing only US-ASCII alphanumeric characters.
 *
 * \return textual name of the tag
 */
const char *ExifMnoteData::exif_mnote_data_get_name (unsigned int n)
{
	return get_name (n);
}

/*! Returns textual title of the given MakerNote tag.
 * The title is a short, localized textual description of the tag.
 *
 * \param[in] n index of the entry within the MakerNote data
 * \return textual name of the tag
 */
const char *ExifMnoteData::exif_mnote_data_get_title (unsigned int n)
{
	return get_title (n);
}

/*! Returns verbose textual description of the given MakerNote tag.
 *
 * \param[in] n index of the entry within the MakerNote data
 * \return textual description of the tag
 */
const char *ExifMnoteData::exif_mnote_data_get_description (unsigned int n)
{
	return get_description (n);
}

/*! Return a textual representation of the value of the MakerNote entry.
 *
 * \warning The character set of the returned string may be in
 *          the encoding of the current locale or the native encoding
 *          of the camera.
 *
 * \param[in] n index of the entry within the MakerNote data
 * \param[out] val buffer in which to store value
 * \param[in] maxlen length of the buffer val
 * \return val pointer, or NULL on error
 */
char *ExifMnoteData::exif_mnote_data_get_value (unsigned int n, char *val, unsigned int maxlen)
{
	return get_value (n, val, maxlen);
}

void ExifMnoteData::exif_mnote_data_log (ExifLog *log0)
{

	log = log0;
}
