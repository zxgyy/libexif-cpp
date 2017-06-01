/* exif-data.c
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

#include "exif-mnote-data.h"
#include "exif-data.h"
#include "exif-ifd.h"
#include "exif-utils.h"
#include "exif-loader.h"
#include "exif-log.h"
#include "i18n.h"
#include "exif-system.h"

#include "canon/exif-mnote-data-canon.h"
#include "fuji/exif-mnote-data-fuji.h"
#include "olympus/exif-mnote-data-olympus.h"
#include "pentax/exif-mnote-data-pentax.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#undef JPEG_MARKER_SOI
#define JPEG_MARKER_SOI  0xd8
#undef JPEG_MARKER_APP0
#define JPEG_MARKER_APP0 0xe0
#undef JPEG_MARKER_APP1
#define JPEG_MARKER_APP1 0xe1

static const unsigned char ExifHeader[] = {0x45, 0x78, 0x69, 0x66, 0x00, 0x00};


ExifData::ExifData()
{
	Init();
}
ExifData::~ExifData()
{
	exif_data_free();
}

/*! Allocate a new #ExifData and load EXIF data from a memory buffer.
 *
 * \param[in] data pointer to raw JPEG or EXIF data
 * \param[in] size number of bytes of data at data
 * \return allocated #ExifData, or NULL on error
 */
void ExifData::exif_data_new_from_data(const unsigned char *data, unsigned int size)
{
	exif_data_free();
	
	exif_data_new ();
	exif_data_load_data (data, size);
	return ;

}
void ExifData::Init()
{
	memset(&ifd[EXIF_IFD_0], 0, EXIF_IFD_COUNT*sizeof(ExifContent *));
	data=NULL;
	size=0;
}


void ExifData::exif_data_free()
{
	unsigned int i=0;

	for (i = 0; i < EXIF_IFD_COUNT; i++) 
	{
		if (ifd[i]) 
		{
			delete ifd[i];
			ifd[i] = NULL;
		}
	}

	if (data) {
		size>1?delete [] data:delete data;
		data=NULL;
		size=0;
	}
	priv.data_free();
	
}

/*! Allocate a new #ExifData. The #ExifData contains an empty
 * #ExifContent for each IFD and the default set of options,
 * which has #EXIF_DATA_OPTION_IGNORE_UNKNOWN_TAGS
 * and #EXIF_DATA_OPTION_FOLLOW_SPECIFICATION set.
 *
 * \return NULL;
 */

void ExifData::exif_data_new (void)
{
	exif_data_free();

	priv.Init();

	for (unsigned int i = 0; i < EXIF_IFD_COUNT; i++) 
	{
		ifd[i]=new ExifContent(this);
		if (!ifd[i]) 
		{
			return ;
		}
		ifd[i]->parent = this;
		ifd[i]->exif_content_log_mem(&priv.log, &priv.mem);
	}

	/* Default options */
#ifndef NO_VERBOSE_TAG_STRINGS
	/*
	 * When the tag list is compiled away, setting this option prevents
	 * any tags from being loaded
	 */
	exif_data_set_option (EXIF_DATA_OPTION_IGNORE_UNKNOWN_TAGS);
#endif
	exif_data_set_option (EXIF_DATA_OPTION_FOLLOW_SPECIFICATION);

	/* Default data type: none */
	exif_data_set_data_type (EXIF_DATA_TYPE_COUNT);

	return ;
}

/*! Set the given option on the given #ExifData.
 *
 * \param[in] Typex option
 */
void ExifData::exif_data_set_option(ExifDataOption Typex)
{
	priv.exif_data_set_option(Typex);
}

/*! Set the data type for the given #ExifData.
 *
 * \param[in] d EXIF data
 * \param[in] dt data type
 */
void ExifData::exif_data_set_data_type (ExifDataType dt)
{
	priv.data_type = dt;
}


/*! Return the MakerNote data out of the EXIF data.  Only certain
 * MakerNote formats that are recognized by libexif are supported.
 * The pointer references a member of the #ExifData structure and must NOT be
 * freed by the caller.
 *
 * \return MakerNote data, or NULL if not found or not supported
 */
ExifMnoteData * ExifData::exif_data_get_mnote_data()
{
	return priv.md;
}

/*! Return an #ExifEntry for the given tag if found in any IFD.
 * Each IFD is searched in turn and the first containing a tag with
 * this number is returned.
 *
 * \param[in] t #ExifTag
 * \return #ExifEntry* if found, else NULL if not found
 */
ExifEntry *ExifData::exif_data_get_entry(ExifTag t)
{
	return (ifd[EXIF_IFD_0]->exif_content_get_entry(t) ?
		ifd[EXIF_IFD_0]->exif_content_get_entry(t) :
		ifd[EXIF_IFD_1]->exif_content_get_entry(t) ?
		ifd[EXIF_IFD_1]->exif_content_get_entry(t) :
		ifd[EXIF_IFD_EXIF]->exif_content_get_entry(t) ?
		ifd[EXIF_IFD_EXIF]->exif_content_get_entry(t) :
		ifd[EXIF_IFD_GPS]->exif_content_get_entry(t) ?
		ifd[EXIF_IFD_GPS]->exif_content_get_entry(t) :
		ifd[EXIF_IFD_INTEROPERABILITY]->exif_content_get_entry(t) ?
		ifd[EXIF_IFD_INTEROPERABILITY]->exif_content_get_entry(t) : NULL);
}


unsigned char *ExifDataPrivate::exif_data_alloc (unsigned int i)
{
	unsigned char *d=NULL;

	if (!i)
		return NULL;

	mem.exif_mem_alloc (&d, i/sizeof(unsigned char));
	if (d) 
		return d;

	EXIF_LOG_NO_MEMORY (log, "ExifData", i);
	return NULL;
}


ExifDataPrivate::~ExifDataPrivate()
{
	data_free();
}


void ExifDataPrivate::exif_data_set_option(ExifDataOption Typex)
{
	options =static_cast<ExifDataOption> (options | Typex);
}

int ExifDataPrivate::exif_data_load_data_entry (ExifEntry *entry,
			   const unsigned char *d,
			   unsigned int size, unsigned int offset)
{
	unsigned int s, doff;

	entry->tag        = static_cast<ExifTag>(exif_get_short (d + offset + 0, order));
	entry->format     = static_cast<ExifFormat>(exif_get_short (d + offset + 2, order));
	entry->components = exif_get_long  (d + offset + 4, order);

	/* FIXME: should use exif_tag_get_name_in_ifd here but entry->parent 
	 * has not been set yet
	 */
	log.exif_log ( EXIF_LOG_CODE_DEBUG, "ExifData",
		  "Loading entry 0x%x ('%s')...", entry->tag,
		  exif_tag_get_name (entry->tag));

	/* {0,1,2,4,8} x { 0x00000000 .. 0xffffffff } 
	 *   -> { 0x000000000 .. 0x7fffffff8 } */
	s = exif_format_get_size(entry->format) * entry->components;
	if ((s < entry->components) || (s == 0)){
		return 0;
	}

	/*
	 * Size? If bigger than 4 bytes, the actual data is not
	 * in the entry but somewhere else (offset).
	 */
	if (s > 4)
		doff = exif_get_long (d + offset + 8, order);
	else
		doff = offset + 8;

	/* Sanity checks */
	if ((doff + s < doff) || (doff + s < s) || (doff + s > size)) {
		log.exif_log ( EXIF_LOG_CODE_DEBUG, "ExifData",
				  "Tag data past end of buffer (%u > %u)", doff+s, size);	
		return 0;
	}
	entry->exif_entry_free();
	entry->data =exif_data_alloc (s);
	if (entry->data) {
		entry->size = s;
		memcpy (entry->data, d + doff, s);
	} else {
		/* FIXME: What do our callers do if (entry->data == NULL)? */
		EXIF_LOG_NO_MEMORY(log, "ExifData", s);
	}

	/* If this is the MakerNote, remember the offset */
	if (entry->tag == EXIF_TAG_MAKER_NOTE) {
		if (!entry->data) {
			log.exif_log ( EXIF_LOG_CODE_DEBUG, "ExifData",
					  "MakerNote found with empty data");	
		} else if (entry->size > 6) {
			log.exif_log (
					       EXIF_LOG_CODE_DEBUG, "ExifData",
					       "MakerNote found (%02x %02x %02x %02x "
					       "%02x %02x %02x...).",
					       entry->data[0], entry->data[1], entry->data[2],
					       entry->data[3], entry->data[4], entry->data[5],
					       entry->data[6]);
		}
		offset_mnote = doff;
	}
	return 1;
}

void ExifDataPrivate::exif_data_save_data_entry (ExifEntry *e,
			   unsigned char **d, unsigned int *ds,
			   unsigned int offset)
{

	unsigned int doff=0, s=0;
	unsigned int ts=0;

	/*
	 * Each entry is 12 bytes long. The memory for the entry has
	 * already been allocated.
	 */
	exif_set_short (*d + 6 + offset + 0,
			order, (ExifShort) e->tag);
	exif_set_short (*d + 6 + offset + 2,
			order, (ExifShort) e->format);

	if (!(options & EXIF_DATA_OPTION_DONT_CHANGE_MAKER_NOTE)) {
		/* If this is the maker note tag, update it. */
		if ((e->tag == EXIF_TAG_MAKER_NOTE) && md) {
			/* TODO: this is using the wrong ExifMem to free e->data */
			e->size = 0;
			md->exif_mnote_data_set_offset (*ds - 6);
			md->exif_mnote_data_save (&e->data, &e->size);
			e->components = e->size;
		}
	}

	exif_set_long  (*d + 6 + offset + 4,order, e->components);

	/*
	 * Size? If bigger than 4 bytes, the actual data is not in
	 * the entry but somewhere else.
	 */
	s = exif_format_get_size (e->format) * e->components;
	if (s > 4) {
		unsigned char *t;
		doff = *ds - 6;
		ts = *ds + s;

		/*
		 * According to the TIFF specification,
		 * the offset must be an even number. If we need to introduce
		 * a padding byte, we set it to 0.
		 */
		if (s & 1)
			ts++;
		t =mem.exif_mem_realloc (d, ts);
		if (!t) {
			EXIF_LOG_NO_MEMORY (log, "ExifData", ts);
		  	return;
		}
		*d = t;
		*ds = ts;
		exif_set_long (*d + 6 + offset + 8, order, doff);
		if (s & 1) 
			*(*d + *ds - 1) = '\0';

	} else
		doff = offset + 8;

	/* Write the data. Fill unneeded bytes with 0. Do not crash with
	 * e->data is NULL */
	if (e->data) {
		memcpy (*d + 6 + doff, e->data, s);
	} else {
		memset (*d + 6 + doff, 0, s);
	}
	if (s < 4) 
		memset (*d + 6 + doff + s, 0, (4 - s));
}

void
ExifData::exif_data_load_data_thumbnail (const unsigned char *d,
			       unsigned int ds, ExifLong o, ExifLong s)
{
	/* Sanity checks */
	if ((o + s < o) || (o + s < s) || (o + s > ds) || (o > ds)) {
		priv.log.exif_log (EXIF_LOG_CODE_DEBUG, "ExifData",
			  "Bogus thumbnail offset (%u) or size (%u).",
			  o, s);
		return;
	}

	if (data)
	{
		priv.data_free();
	}
	if (!(data =priv.exif_data_alloc (s))) {
		EXIF_LOG_NO_MEMORY (priv.log, "ExifData", s);
		size = 0;
		return;
	}
	size = s;
	memcpy (data, d + o, s);
}

#undef CHECK_REC
#define CHECK_REC(i) 					\
if ((i) == ifd0) {				\
	priv.log.exif_log(EXIF_LOG_CODE_DEBUG, \
		"ExifData", "Recursive entry in IFD "	\
		"'%s' detected. Skipping...",		\
		exif_ifd_get_name (i));			\
	break;						\
}							\
if (ifd[(i)]->entries.size()) {				\
	priv.log.exif_log(EXIF_LOG_CODE_DEBUG,	\
		"ExifData", "Attempt to load IFD "	\
		"'%s' multiple times detected. "	\
		"Skipping...",				\
		exif_ifd_get_name (i));			\
	break;						\
}

/*! Load data for an IFD.
 *
 * \param[in] ifd IFD to load
 * \param[in] d pointer to buffer containing raw IFD data
 * \param[in] ds size of raw data in buffer at \c d
 * \param[in] offset offset into buffer at \c d at which IFD starts
 * \param[in] recursion_depth number of times this function has been
 * recursively called without returning
 */
void ExifData::exif_data_load_data_content (ExifIfd ifd0,
			     const unsigned char *d,
			     unsigned int ds, unsigned int offset, unsigned int recursion_depth)
{
	ExifLong o, thumbnail_offset = 0, thumbnail_length = 0;
	ExifShort n;
	unsigned int i;
	ExifTag tag;
	

	/* check for valid ExifIfd enum range */
	if ((((int)ifd0) < 0) || ( ((int)ifd0) >= EXIF_IFD_COUNT))
	  return;

	if (recursion_depth > 30) {
		priv.log.exif_log(EXIF_LOG_CODE_CORRUPT_DATA, "ExifData",
			  "Deep recursion detected!");
		return;
	}

	/* Read the number of entries */
	if ((offset + 2 < offset) || (offset + 2 < 2) || (offset + 2 > ds)) {
		priv.log.exif_log(EXIF_LOG_CODE_CORRUPT_DATA, "ExifData",
			  "Tag data past end of buffer (%u > %u)", offset+2, ds);
		return;
	}
	n = exif_get_short (d + offset, priv.order);
	priv.log.exif_log(EXIF_LOG_CODE_DEBUG, "ExifData",
	          "Loading %hu entries...", n);
	offset += 2;

	/* Check if we have enough data. */
	if (offset + 12 * n > ds) {
		n = (ds - offset) / 12;
		priv.log.exif_log(EXIF_LOG_CODE_DEBUG, "ExifData",
				  "Short data; only loading %hu entries...", n);
	}

	for (i = 0; i < n; i++) {

		tag =static_cast<ExifTag>( exif_get_short (d + offset + 12 * i, priv.order));
		switch (tag) {
		case EXIF_TAG_EXIF_IFD_POINTER:
		case EXIF_TAG_GPS_INFO_IFD_POINTER:
		case EXIF_TAG_INTEROPERABILITY_IFD_POINTER:
		case EXIF_TAG_JPEG_INTERCHANGE_FORMAT_LENGTH:
		case EXIF_TAG_JPEG_INTERCHANGE_FORMAT:
			o = exif_get_long (d + offset + 12 * i + 8,
					   priv.order);
			/* FIXME: IFD_POINTER tags aren't marked as being in a
			 * specific IFD, so exif_tag_get_name_in_ifd won't work
			 */
			priv.log.exif_log(EXIF_LOG_CODE_DEBUG, "ExifData",
				  "Sub-IFD entry 0x%x ('%s') at %u.", tag,
				  exif_tag_get_name(tag), o);
			switch (tag) {
			case EXIF_TAG_EXIF_IFD_POINTER:
				CHECK_REC (EXIF_IFD_EXIF);
				exif_data_load_data_content (EXIF_IFD_EXIF, d, ds, o, recursion_depth + 1);
				break;
			case EXIF_TAG_GPS_INFO_IFD_POINTER:
				CHECK_REC (EXIF_IFD_GPS);
				exif_data_load_data_content (EXIF_IFD_GPS, d, ds, o, recursion_depth + 1);
				break;
			case EXIF_TAG_INTEROPERABILITY_IFD_POINTER:
				CHECK_REC (EXIF_IFD_INTEROPERABILITY);
				exif_data_load_data_content (EXIF_IFD_INTEROPERABILITY, d, ds, o, recursion_depth + 1);
				break;
			case EXIF_TAG_JPEG_INTERCHANGE_FORMAT:
				thumbnail_offset = o;
				if (thumbnail_offset && thumbnail_length)
					exif_data_load_data_thumbnail (d,
								       ds, thumbnail_offset,
								       thumbnail_length);
				break;
			case EXIF_TAG_JPEG_INTERCHANGE_FORMAT_LENGTH:
				thumbnail_length = o;
				if (thumbnail_offset && thumbnail_length)
					exif_data_load_data_thumbnail (d,
								       ds, thumbnail_offset,
								       thumbnail_length);
				break;
			default:
				return;
			}
			break;
		default:

			/*
			 * If we don't know the tag, don't fail. It could be that new 
			 * versions of the standard have defined additional tags. Note that
			 * 0 is a valid tag in the GPS IFD.
			 */
			if (!exif_tag_get_name_in_ifd (tag, ifd0)) {

				/*
				 * Special case: Tag and format 0. That's against specification
				 * (at least up to 2.2). But Photoshop writes it anyways.
				 */
				if (!memcmp (d + offset + 12 * i, "\0\0\0\0", 4)) {
					priv.log.exif_log(EXIF_LOG_CODE_DEBUG, "ExifData",
						  "Skipping empty entry at position %u in '%s'.", i, 
						  exif_ifd_get_name (ifd0));
					break;
				}
				priv.log.exif_log(EXIF_LOG_CODE_DEBUG, "ExifData",
					  "Unknown tag 0x%04x (entry %u in '%s'). Please report this tag "
					  "to <libexif-devel@lists.sourceforge.net>.", tag, i,
					  exif_ifd_get_name (ifd0));
				if (priv.options & EXIF_DATA_OPTION_IGNORE_UNKNOWN_TAGS)
					break;
			}
			ExifEntry entry;
			if (priv.exif_data_load_data_entry (&entry, d, ds, offset + 12 * i))
				ifd[ifd0]->exif_content_add_entry (entry);
			break;
		}
	}
}

static int
cmp_func (const unsigned char *p1, const unsigned char *p2, ExifByteOrder o)
{
	ExifShort tag1 = exif_get_short (p1, o);
	ExifShort tag2 = exif_get_short (p2, o);

	return (tag1 < tag2) ? -1 : (tag1 > tag2) ? 1 : 0;
}

static int
cmp_func_intel (const void *elem1, const void *elem2)
{
	return cmp_func ((const unsigned char *) elem1,
			 (const unsigned char *) elem2, EXIF_BYTE_ORDER_INTEL);
}

static int
cmp_func_motorola (const void *elem1, const void *elem2)
{
	return cmp_func ((const unsigned char *) elem1,
			 (const unsigned char *) elem2, EXIF_BYTE_ORDER_MOTOROLA);
}

void ExifData::exif_data_save_data_content (ExifContent *ifd0,
			     unsigned char **d, unsigned int *ds,
			     unsigned int offset)
{
	unsigned int j=0, n_ptr = 0, n_thumb = 0;
	ExifIfd i=EXIF_IFD_0;
	unsigned char *t=NULL;
	unsigned int ts=0;

	if (!ifd || !d || !ds) 
		return;

	for (int i = EXIF_IFD_0; i < EXIF_IFD_COUNT; i++)
		if (ifd0 == ifd[i])
			break;
	if (i == EXIF_IFD_COUNT)
		return;	/* error */

	/*
	 * Check if we need some extra entries for pointers or the thumbnail.
	 */
	switch (i) {
	case EXIF_IFD_0:

		/*
		 * The pointer to IFD_EXIF is in IFD_0. The pointer to
		 * IFD_INTEROPERABILITY is in IFD_EXIF.
		 */
		if (ifd[EXIF_IFD_EXIF]->entries.size() ||
		    ifd[EXIF_IFD_INTEROPERABILITY]->entries.size())
			n_ptr++;

		/* The pointer to IFD_GPS is in IFD_0. */
		if (ifd[EXIF_IFD_GPS]->entries.size())
			n_ptr++;

		break;
	case EXIF_IFD_1:
		if (size)
			n_thumb = 2;
		break;
	case EXIF_IFD_EXIF:
		if (ifd[EXIF_IFD_INTEROPERABILITY]->entries.size())
			n_ptr++;
	default:
		break;
	}

	/*
	 * Allocate enough memory for all entries
	 * and the number of entries.
	 */
	ts = *ds + (2 + (ifd0->entries.size() + n_ptr + n_thumb) * 12 + 4);
	t =priv.mem.exif_mem_realloc (d, ts);
	if (!t) {
		EXIF_LOG_NO_MEMORY (priv.log, "ExifData", ts);
	  	return;
	}
	*d = t;
	*ds = ts;

	/* Save the number of entries */
	exif_set_short (*d + 6 + offset, priv.order,
			(ExifShort) (ifd0->entries.size() + n_ptr + n_thumb));
	offset += 2;

	/*
	 * Save each entry. Make sure that no memcpys from NULL pointers are
	 * performed
	 */
	priv.log.exif_log(EXIF_LOG_CODE_DEBUG, "ExifData",
		  "Saving %i entries (IFD '%s', offset: %i)...",
		  ifd0->entries.size(), exif_ifd_get_name (i), offset);
	for (std::vector<ExifEntry>::iterator it = ifd0->entries.begin(); it < ifd0->entries.end(); ++it) 
	{
		priv.exif_data_save_data_entry (&(*it), d, ds,offset + 12 * j);
		j++;
	}

	offset += 12 * ifd0->entries.size();

	/* Now save special entries. */
	switch (i) {
	case EXIF_IFD_0:

		/*
		 * The pointer to IFD_EXIF is in IFD_0.
		 * However, the pointer to IFD_INTEROPERABILITY is in IFD_EXIF,
		 * therefore, if IFD_INTEROPERABILITY is not empty, we need
		 * IFD_EXIF even if latter is empty.
		 */
		if (ifd[EXIF_IFD_EXIF]->entries.size() ||
		    ifd[EXIF_IFD_INTEROPERABILITY]->entries.size()) {
			exif_set_short (*d + 6 + offset + 0, priv.order,
					EXIF_TAG_EXIF_IFD_POINTER);
			exif_set_short (*d + 6 + offset + 2, priv.order,
					EXIF_FORMAT_LONG);
			exif_set_long  (*d + 6 + offset + 4, priv.order,
					1);
			exif_set_long  (*d + 6 + offset + 8, priv.order,
					*ds - 6);
			exif_data_save_data_content (ifd[EXIF_IFD_EXIF], d, ds, *ds - 6);
			offset += 12;
		}

		/* The pointer to IFD_GPS is in IFD_0, too. */
		if (ifd[EXIF_IFD_GPS]->entries.size()) {
			exif_set_short (*d + 6 + offset + 0, priv.order,
					EXIF_TAG_GPS_INFO_IFD_POINTER);
			exif_set_short (*d + 6 + offset + 2, priv.order,
					EXIF_FORMAT_LONG);
			exif_set_long  (*d + 6 + offset + 4, priv.order,
					1);
			exif_set_long  (*d + 6 + offset + 8, priv.order,
					*ds - 6);
			exif_data_save_data_content (ifd[EXIF_IFD_GPS], d, ds, *ds - 6);
			offset += 12;
		}

		break;
	case EXIF_IFD_EXIF:

		/*
		 * The pointer to IFD_INTEROPERABILITY is in IFD_EXIF.
		 * See note above.
		 */
		if (ifd[EXIF_IFD_INTEROPERABILITY]->entries.size()) {
			exif_set_short (*d + 6 + offset + 0, priv.order,
					EXIF_TAG_INTEROPERABILITY_IFD_POINTER);
			exif_set_short (*d + 6 + offset + 2, priv.order,
					EXIF_FORMAT_LONG);
			exif_set_long  (*d + 6 + offset + 4, priv.order,
					1);
			exif_set_long  (*d + 6 + offset + 8, priv.order,
					*ds - 6);
			exif_data_save_data_content (ifd[EXIF_IFD_INTEROPERABILITY], d, ds,
						     *ds - 6);
			offset += 12;
		}

		break;
	case EXIF_IFD_1:

		/*
		 * Information about the thumbnail (if any) is saved in
		 * IFD_1.
		 */
		if (size) {

			/* EXIF_TAG_JPEG_INTERCHANGE_FORMAT */
			exif_set_short (*d + 6 + offset + 0, priv.order,
					EXIF_TAG_JPEG_INTERCHANGE_FORMAT);
			exif_set_short (*d + 6 + offset + 2, priv.order,
					EXIF_FORMAT_LONG);
			exif_set_long  (*d + 6 + offset + 4, priv.order,
					1);
			exif_set_long  (*d + 6 + offset + 8, priv.order,
					*ds - 6);
			ts = *ds + size;
			t = priv.mem.exif_mem_realloc (d, ts);
			if (!t) {
				EXIF_LOG_NO_MEMORY (priv.log, "ExifData",
						    ts);
			  	return;
			}
			*d = t;
			*ds = ts;
			memcpy (*d + *ds - size, data, size);
			offset += 12;

			/* EXIF_TAG_JPEG_INTERCHANGE_FORMAT_LENGTH */
			exif_set_short (*d + 6 + offset + 0, priv.order,
					EXIF_TAG_JPEG_INTERCHANGE_FORMAT_LENGTH);
			exif_set_short (*d + 6 + offset + 2, priv.order,
					EXIF_FORMAT_LONG);
			exif_set_long  (*d + 6 + offset + 4, priv.order,
					1);
			exif_set_long  (*d + 6 + offset + 8, priv.order,
					size);
			offset += 12;
		}

		break;
	default:
		break;
	}

	/* Sort the directory according to TIFF specification */
	qsort (*d + 6 + offset - (ifd0->entries.size() + n_ptr + n_thumb) * 12,
	       (ifd0->entries.size() + n_ptr + n_thumb), 12,
	       (priv.order == EXIF_BYTE_ORDER_INTEL) ? cmp_func_intel : cmp_func_motorola);

	/* Correctly terminate the directory */
	if (i == EXIF_IFD_0 && (ifd[EXIF_IFD_1]->entries.size() || size))
	{

		/*
		 * We are saving IFD 0. Tell where IFD 1 starts and save
		 * IFD 1.
		 */
		exif_set_long (*d + 6 + offset, priv.order, *ds - 6);
		exif_data_save_data_content (ifd[EXIF_IFD_1], d, ds,*ds - 6);
	} else
		exif_set_long (*d + 6 + offset, priv.order, 0);
}

typedef enum {
	EXIF_DATA_TYPE_MAKER_NOTE_NONE		= 0,
	EXIF_DATA_TYPE_MAKER_NOTE_CANON		= 1,
	EXIF_DATA_TYPE_MAKER_NOTE_OLYMPUS	= 2,
	EXIF_DATA_TYPE_MAKER_NOTE_PENTAX	= 3,
	EXIF_DATA_TYPE_MAKER_NOTE_NIKON		= 4,
	EXIF_DATA_TYPE_MAKER_NOTE_CASIO		= 5,
	EXIF_DATA_TYPE_MAKER_NOTE_FUJI 		= 6
} ExifDataTypeMakerNote;

/*! If MakerNote is recognized, load it.
 *
 * \param[in,out] data #ExifData
 * \param[in] d pointer to raw EXIF data
 * \param[in] ds length of data at d
 */
void ExifData::interpret_maker_note(const unsigned char *d, unsigned int ds)
{
	int mnoteid=0;

	ExifEntry* e = exif_data_get_entry (EXIF_TAG_MAKER_NOTE);
	if (!e)
		return;
	
	priv.data_free();
	if ((mnoteid = exif_mnote_data_olympus_identify (e)) != 0) {
		priv.log.exif_log(EXIF_LOG_CODE_DEBUG,
			"ExifData", "Olympus MakerNote variant type %d", mnoteid);
		
		
		priv.md = new ExifMnoteDataOlympus;

	} else if ((mnoteid = exif_mnote_data_canon_identify (e)) != 0) {
		priv.log.exif_log(EXIF_LOG_CODE_DEBUG,
			"ExifData", "Canon MakerNote variant type %d", mnoteid);
		priv.md = new ExifMnoteDataCanon;

	} else if ((mnoteid = exif_mnote_data_fuji_identify (e)) != 0) {
		priv.log.exif_log(EXIF_LOG_CODE_DEBUG,
			"ExifData", "Fuji MakerNote variant type %d", mnoteid);
		priv.md = new ExifMnoteDataFuji;

	/* NOTE: Must do Pentax detection last because some of the
	 * heuristics are pretty general. */
	} else if ((mnoteid = exif_mnote_data_pentax_identify (e)) != 0) {
		priv.log.exif_log(EXIF_LOG_CODE_DEBUG,
			"ExifData", "Pentax MakerNote variant type %d", mnoteid);
		priv.md = new ExifMnoteDataPentax;
	}

	/* 
	 * If we are able to interpret the maker note, do so.
	 */
	if (priv.md) {
		priv.md->exif_mnote_data_log (&priv.log);
		priv.md->exif_mnote_data_construct (&priv.mem);
		priv.md->set_byte_order (priv.order);
		priv.md->set_offset (priv.offset_mnote);
		priv.md->load (d, ds);
	}
}

#define LOG_TOO_SMALL \
priv.log.exif_log(EXIF_LOG_CODE_CORRUPT_DATA, "ExifData", \
		_("Size of data too small to allow for EXIF data."));

/*! Load the #ExifData structure from the raw JPEG or EXIF data in the given
 * memory buffer. If the EXIF data contains a recognized MakerNote, it is
 * loaded and stored as well for later retrieval by #exif_data_get_mnote_data.
 * If the #EXIF_DATA_OPTION_FOLLOW_SPECIFICATION option has been set on this
 * #ExifData, then the tags are automatically fixed after loading (by calling
 * #exif_data_fix).
 *
 * \param[in] d pointer to raw JPEG or EXIF data
 * \param[in] size number of bytes of data at d
 */
void ExifData::exif_data_load_data (const unsigned char *d_orig,unsigned int ds)
{
	unsigned int l=0;
	ExifLong offset=0;
	ExifShort n=0;
	const unsigned char *d = d_orig;
	unsigned int len=0, fullds=0;

	if (!d || !ds) return;
		

	priv.log.exif_log(EXIF_LOG_CODE_DEBUG, "ExifData", "Parsing %i byte(s) EXIF data...\n", ds);

	/*
	 * It can be that the data starts with the EXIF header. If it does
	 * not, search the EXIF marker.
	 */
	if (ds < 6) 
	{
		LOG_TOO_SMALL;
		return;
	}
	if (!memcmp (d, ExifHeader, 6)) {
		priv.log.exif_log(EXIF_LOG_CODE_DEBUG, "ExifData",
			  "Found EXIF header.");
	} else {
		while (ds >= 3) {
			while (ds && (d[0] == 0xff)) {
				d++;
				ds--;
			}

			/* JPEG_MARKER_SOI */
			if (ds && d[0] == JPEG_MARKER_SOI) {
				d++;
				ds--;
				continue;
			}

			/* JPEG_MARKER_APP0 */
			if (ds >= 3 && d[0] == JPEG_MARKER_APP0) {
				d++;
				ds--;
				l = (d[0] << 8) | d[1];
				if (l > ds)
					return;
				d += l;
				ds -= l;
				continue;
			}

			/* JPEG_MARKER_APP1 */
			if (ds && d[0] == JPEG_MARKER_APP1)
				break;

			/* Unknown marker or data. Give up. */
			priv.log.exif_log(EXIF_LOG_CODE_CORRUPT_DATA,
				  "ExifData", _("EXIF marker not found."));
			return;
		}
		if (ds < 3) {
			LOG_TOO_SMALL;
			return;
		}
		d++;
		ds--;
		len = (d[0] << 8) | d[1];
		priv.log.exif_log(EXIF_LOG_CODE_DEBUG, "ExifData",
			  "We have to deal with %i byte(s) of EXIF data.",
			  len);
		d += 2;
		ds -= 2;
	}

	/*
	 * Verify the exif header
	 * (offset 2, length 6).
	 */
	if (ds < 6) {
		LOG_TOO_SMALL;
		return;
	}
	if (memcmp (d, ExifHeader, 6)) {
		priv.log.exif_log(EXIF_LOG_CODE_CORRUPT_DATA,
			  "ExifData", _("EXIF header not found."));
		return;
	}

	priv.log.exif_log(EXIF_LOG_CODE_DEBUG, "ExifData",
		  "Found EXIF header.");

	/* Sanity check the data length */
	if (ds < 14)
		return;

	/* The JPEG APP1 section can be no longer than 64 KiB (including a
	   16-bit length), so cap the data length to protect against overflow
	   in future offset calculations */
	fullds = ds;
	if (ds > 0xfffe)
		ds = 0xfffe;

	/* Byte order (offset 6, length 2) */
	if (!memcmp (d + 6, "II", 2))
		priv.order = EXIF_BYTE_ORDER_INTEL;
	else if (!memcmp (d + 6, "MM", 2))
		priv.order = EXIF_BYTE_ORDER_MOTOROLA;
	else {
		priv.log.exif_log(EXIF_LOG_CODE_CORRUPT_DATA,
			  "ExifData", _("Unknown encoding."));
		return;
	}

	/* Fixed value */
	if (exif_get_short (d + 8, priv.order) != 0x002a)
		return;

	/* IFD 0 offset */
	offset = exif_get_long (d + 10, priv.order);
	priv.log.exif_log(EXIF_LOG_CODE_DEBUG, "ExifData", 
		  "IFD 0 at %i.", (int) offset);

	/* Sanity check the offset, being careful about overflow */
	if (offset > ds || offset + 6 + 2 > ds)
		return;

	/* Parse the actual exif data (usually offset 14 from start) */
	exif_data_load_data_content (EXIF_IFD_0, d + 6, ds - 6, offset, 0);

	/* IFD 1 offset */
	n = exif_get_short (d + 6 + offset, priv.order);
	if (offset + 6 + 2 + 12 * n + 4 > ds)
		return;

	offset = exif_get_long (d + 6 + offset + 2 + 12 * n, priv.order);
	if (offset) {
		priv.log.exif_log(EXIF_LOG_CODE_DEBUG, "ExifData",
			  "IFD 1 at %i.", (int) offset);

		/* Sanity check. */
		if (offset > ds || offset + 6 > ds) {
			priv.log.exif_log(EXIF_LOG_CODE_CORRUPT_DATA,
				  "ExifData", "Bogus offset of IFD1.");
		} else {
		   exif_data_load_data_content (EXIF_IFD_1, d + 6, ds - 6, offset, 0);
		}
	}

	/*
	 * If we got an EXIF_TAG_MAKER_NOTE, try to interpret it. Some
	 * cameras use pointers in the maker note tag that point to the
	 * space between IFDs. Here is the only place where we have access
	 * to that data.
	 */
	interpret_maker_note(d, fullds);

	/* Fixup tags if requested */
	if (priv.options & EXIF_DATA_OPTION_FOLLOW_SPECIFICATION)
		exif_data_fix ();
}

void ExifData::exif_data_save_data (unsigned char **d, unsigned int *ds)
{
	if (ds)
		*ds = 0;	/* This means something went wrong */

	if (!d || !ds)
		return;

	/* Header */
	*ds = 14;
	*d =priv.exif_data_alloc (*ds);
	if (!*d)  {
		*ds = 0;
		return;
	}
	memcpy (*d, ExifHeader, 6);

	/* Order (offset 6) */
	if (priv.order == EXIF_BYTE_ORDER_INTEL) {
		memcpy (*d + 6, "II", 2);
	} else {
		memcpy (*d + 6, "MM", 2);
	}

	/* Fixed value (2 bytes, offset 8) */
	exif_set_short (*d + 8, priv.order, 0x002a);

	/*
	 * IFD 0 offset (4 bytes, offset 10).
	 * We will start 8 bytes after the
	 * EXIF header (2 bytes for order, another 2 for the test, and
	 * 4 bytes for the IFD 0 offset make 8 bytes together).
	 */
	exif_set_long (*d + 10, priv.order, 8);

	/* Now save IFD 0. IFD 1 will be saved automatically. */
	priv.log.exif_log(EXIF_LOG_CODE_DEBUG, "ExifData",
		  "Saving IFDs...");
	exif_data_save_data_content (ifd[EXIF_IFD_0], d, ds, *ds - 6);
	priv.log.exif_log(EXIF_LOG_CODE_DEBUG, "ExifData",
		  "Saved %i byte(s) EXIF data.", *ds);
}

/*! Allocate a new #ExifData and load EXIF data from a JPEG file.
 * Uses an #ExifLoader internally to do the loading.
 *
 * \param[in] path filename including path
 * \return allocated #ExifData, or NULL on error
 */
void ExifData::exif_data_new_from_file (const char *path)
{
	exif_data_free();

	ExifLoader *loader=new ExifLoader;

	loader->exif_loader_log(&priv.log);
	loader->exif_loader_new (&priv.mem);
	loader->exif_loader_write_file (path);
	loader->exif_loader_get_data (this);
	
	delete loader;
}

/*! Dump all EXIF data to stdout.
 * This is intended for diagnostic purposes only.
 *
 */
void ExifData::exif_data_dump ()
{
	unsigned int i=0;

	for (i = 0; i < EXIF_IFD_COUNT; i++) {
		if (ifd[i] && ifd[i]->entries.size()) {
			printf ("Dumping IFD '%s'...\n",
				exif_ifd_get_name ((ExifIfd)i));
			ifd[i]->exif_content_dump (0);
		}
	}

	if (data) {
		printf ("%i byte(s) thumbnail data available.", size);
		if (size >= 4) {
			printf ("0x%02x 0x%02x ... 0x%02x 0x%02x\n",
				data[0], data[1],
				data[size - 2],
				data[size - 1]);
		}
	}
}

/*! Return the byte order in use by this EXIF structure.
 *
 * \return byte order
 */
ExifByteOrder ExifData::exif_data_get_byte_order ()
{
	return (priv.order);
}

/*! Execute a function on each IFD in turn.
 *
 * \param[in] func function to call for each entry
 * \param[in] user_data data to pass into func on each call
 */
void ExifData::exif_data_foreach_content (ExifDataForeachContentFunc func, void *user_data)
{
	if (!func)
		return;

	for (unsigned int i = 0; i < EXIF_IFD_COUNT; i++)
		func (ifd[i], user_data);
}

typedef struct _ByteOrderChangeData ByteOrderChangeData;
struct _ByteOrderChangeData {
	ExifByteOrder old, newx;

	_ByteOrderChangeData()
	{
		Init();
	}
	void inline Init()
	{
		old = EXIF_BYTE_ORDER_MOTOROLA;
		newx= EXIF_BYTE_ORDER_MOTOROLA;
	}
};

void ExifData::entry_set_byte_order (ExifEntry *e, void *data)
{
	ByteOrderChangeData *d = (ByteOrderChangeData *) data;

	if (!e)
		return;

	exif_array_set_byte_order (e->format, e->data, e->components, d->old, d->newx);
}

void content_set_byte_order (ExifContent *content, void *data)
{
	content->exif_content_foreach_entry ();
}

/*! Set the byte order to use for this EXIF data. If any tags already exist
 * (including MakerNote tags) they are are converted to the specified byte
 * order.
 *
 * \param[in] order byte order
 */
void ExifData::exif_data_set_byte_order (ExifByteOrder order)
{
	ByteOrderChangeData d;

	if ((order == priv.order))
		return;

	d.old = priv.order;
	d.newx = order;
	exif_data_foreach_content (content_set_byte_order, &d);
	priv.order = order;
	if (priv.md)
		priv.md->set_byte_order (order);
}

/*! Set the log message object for all IFDs.
 *
 * \param[in] log #ExifLog
 */
void ExifData::exif_data_log ()
{
	for (unsigned int i = 0; i < EXIF_IFD_COUNT; i++)
		ifd[i]->exif_content_log_mem(&priv.log,&priv.mem);
}

/* Used internally within libexif */
ExifLog *ExifData::exif_data_get_log ()
{
	return &priv.log;
}

static const struct {
	ExifDataOption option;
	const char *name;
	const char *description;
} exif_data_option[] = {
	{EXIF_DATA_OPTION_IGNORE_UNKNOWN_TAGS, N_("Ignore unknown tags"),
	 N_("Ignore unknown tags when loading EXIF data.")},
	{EXIF_DATA_OPTION_FOLLOW_SPECIFICATION, N_("Follow specification"),
	 N_("Add, correct and remove entries to get EXIF data that follows "
	    "the specification.")},
	{EXIF_DATA_OPTION_DONT_CHANGE_MAKER_NOTE, N_("Do not change maker note"),
	 N_("When loading and resaving Exif data, save the maker note unmodified."
	    " Be aware that the maker note can get corrupted.")},
	{EXIF_DATA_OPTION_IGNORE_UNKNOWN, NULL, NULL}
};

/*! Return a short textual description of the given #ExifDataOption.
 *
 * \param[in] o option
 * \return localized textual description of the option
 */
const char *ExifData::exif_data_option_get_name (ExifDataOption o)
{
	unsigned int i;

	for (i = 0; exif_data_option[i].name; i++)
		if (exif_data_option[i].option == o) 
			break;
	return _(exif_data_option[i].name);
}

/*! Return a verbose textual description of the given #ExifDataOption.
 *
 * \param[in] o option
 * \return verbose localized textual description of the option
 */
const char *ExifData::exif_data_option_get_description (ExifDataOption o)
{
	unsigned int i=0;

	for (i = 0; exif_data_option[i].description; i++)
		if (exif_data_option[i].option == o) 
			break;
	return _(exif_data_option[i].description);
}

/*! Clear the given option on the given #ExifData.
 *
 * \param[in] o option
 */
void ExifData::exif_data_unset_option (ExifDataOption o)
{
	
	priv.options = (ExifDataOption)(priv.options & (~o));
}

static void fix_func (ExifContent *c, void *UNUSED(data))
{
	switch (c->exif_content_get_ifd ()) {
	case EXIF_IFD_1:
		if (c->parent->data)
			c->exif_content_fix ();
		else if (c->entries.size()) {
			c->parent->priv.log.exif_log (EXIF_LOG_CODE_DEBUG, "exif-data",
				  "No thumbnail but entries on thumbnail. These entries have been "
				  "removed.");
			while (c->entries.size()) {
				unsigned int cnt = c->entries.size();
				c->exif_content_remove_entry (c->entries.size() - 1);
				if (cnt == c->entries.size()) {
					/* safety net */
					c->parent->priv.log.exif_log (EXIF_LOG_CODE_DEBUG, "exif-data",
					"failed to remove last entry from entries.");
				}
			}
		}
		break;
	default:
		c->exif_content_fix ();
	}
}

/*! Fix the EXIF data to bring it into specification. Call #exif_content_fix
 * on each IFD to fix existing entries, create any new entries that are
 * mandatory but do not yet exist, and remove any entries that are not
 * allowed.
 *
 * \param[in,out] d EXIF data
 */
void ExifData::exif_data_fix ()
{
	exif_data_foreach_content (fix_func, NULL);
}

/*! Return the data type for the given #ExifData.
 *
 * \return data type, or #EXIF_DATA_TYPE_UNKNOWN on error
 */
ExifDataType ExifData::exif_data_get_data_type ()
{
	return priv.data_type;
}

/*! Detect if MakerNote is recognized as one handled by the Pentax module.
 * 
 * \param[in] ed image #ExifData to identify as as a Pentax type
 * \param[in] e #ExifEntry for EXIF_TAG_MAKER_NOTE, from within ed but
 *   duplicated here for convenience
 * \return 0 if not recognized, nonzero if recognized. The specific nonzero 
 *   value returned may identify a subtype unique within this module.
 */

int ExifData::exif_mnote_data_pentax_identify (const ExifEntry *e)
{
	if ((e->size >= 8) && !memcmp (e->data, "AOC", 4)) {
		if (((e->data[4] == 'I') && (e->data[5] == 'I')) ||
			((e->data[4] == 'M') && (e->data[5] == 'M')))
			return pentaxV3;
		else
			/* Uses Casio v2 tags */
			return pentaxV2;
	}

	if ((e->size >= 8) && !memcmp (e->data, "QVC", 4))
		return casioV2;

	/* This isn't a very robust test, so make sure it's done last */
	/* Maybe we should additionally check for a make of Asahi or Pentax */
	if ((e->size >= 2) && (e->data[0] == 0x00) && (e->data[1] == 0x1b))
		return pentaxV1;

	return 0;
}
int ExifData::exif_mnote_data_olympus_identify (const ExifEntry *e)
{
	int variant = exif_mnote_data_olympus_identify_variant(e->data, e->size);

	if (variant == nikonV0) {
		/* This variant needs some extra checking with the Make */
		char value[5];
		ExifEntry *em = exif_data_get_entry (EXIF_TAG_MAKE);
		variant = unrecognized;

		if (em) {
			const char *v = em->exif_entry_get_value (value, sizeof(value));
			if (v && (!strncmp (v, "Nikon", sizeof(value)) || 
					  !strncmp (v, "NIKON", sizeof(value)) ))
				/* When saved, this variant will be written out like the
				 * alternative nikonV2 form above instead
				 */
				variant = nikonV0;
		}
	}

	return variant;
}

int ExifData::exif_mnote_data_fuji_identify (const ExifEntry *e)
{
	return ((e->size >= 12) && !memcmp (e->data, "FUJIFILM", 8));
}

/*! Detect if MakerNote is recognized as one handled by the Canon module.
 * 
 * \param[in] ed image #ExifData to identify as as a Canon type
 * \param[in] e #ExifEntry for EXIF_TAG_MAKER_NOTE, from within ed but
 *   duplicated here for convenience
 * \return 0 if not recognized, nonzero if recognized. The specific nonzero 
 *   value returned may identify a subtype unique within this module.
 */
int  ExifData::exif_mnote_data_canon_identify (const ExifEntry *e)
{
	char value[8];
	ExifEntry *em = exif_data_get_entry (EXIF_TAG_MAKE);
	if (!em) 
		return 0;
	return !strcmp (em->exif_entry_get_value(value, sizeof (value)), "Canon");
}



