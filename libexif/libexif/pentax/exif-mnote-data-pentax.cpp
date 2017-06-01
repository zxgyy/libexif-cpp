/* exif-mnote-data-pentax.c
 *
 * Copyright (c) 2002, 2003 Lutz Mueller <lutz@users.sourceforge.net>
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

#include "../config.h"
#include "exif-mnote-data-pentax.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../exif-byte-order.h"
#include "../exif-utils.h"

void ExifMnoteDataPentax::exif_mnote_data_pentax_clear ()
{
	unsigned int i=0;

	if (entries) 
	{
			count>1?delete [] entries:delete entries;
			entries=NULL;
			count = 0;
	}
}

void ExifMnoteDataPentax::free ()
{
	exif_mnote_data_pentax_clear ();
}

char *ExifMnoteDataPentax::get_value (unsigned int i, char *val, unsigned int maxlen)
{
	if (count <= i) return NULL;
	return entries[i].mnote_pentax_entry_get_value (val, maxlen);
}

/** 
 * @brief save the MnoteData from ne to buf
 * 
 * @param ne extract the data from this structure 
 * @param *buf write the mnoteData to this buffer (buffer will be allocated)
 * @param buf_size the final size of the buffer
 */
void ExifMnoteDataPentax::save (unsigned char **buf, unsigned int *buf_size)
{
	size_t i,
	   base = 0,		/* internal MakerNote tag number offset */
	   o2 = 4 + 2;  	/* offset to first tag entry, past header */
        size_t datao = offset; /* this MakerNote style uses offsets
        			  based on main IFD, not makernote IFD */

	if (!buf || !buf_size) return;

	/*
	 * Allocate enough memory for header, the number of entries, entries,
	 * and next IFD pointer
	 */
	*buf_size = o2 + 2 + count * 12 + 4;
	switch (version) {
	case casioV2:
		base = MNOTE_PENTAX2_TAG_BASE;
		mem->exif_mem_alloc (buf,(*buf_size)/sizeof(unsigned char));
		if (!*buf) {
			EXIF_LOG_NO_MEMORY_PTR(log, "ExifMnoteDataPentax", *buf_size);
			return;
		}
		/* Write the magic header */
		strcpy ((char *)*buf, "QVC");
		exif_set_short (*buf + 4, order, (ExifShort) 0);

		break;

	case pentaxV3:
		base = MNOTE_PENTAX2_TAG_BASE;
		mem->exif_mem_alloc (buf,(*buf_size)/sizeof(unsigned char));
		if (!*buf) {
			EXIF_LOG_NO_MEMORY_PTR(log, "ExifMnoteDataPentax", *buf_size);
			return;
		}

		/* Write the magic header */
		strcpy ((char *)*buf, "AOC");
		exif_set_short (*buf + 4, order, (ExifShort) (
			(order == EXIF_BYTE_ORDER_INTEL) ?
			('I' << 8) | 'I' :
			('M' << 8) | 'M'));
		break;

	case pentaxV2:
		base = MNOTE_PENTAX2_TAG_BASE;
		mem->exif_mem_alloc (buf, (*buf_size)/sizeof(unsigned char));
		if (!*buf) {
			EXIF_LOG_NO_MEMORY_PTR(log, "ExifMnoteDataPentax", *buf_size);
			return;
		}

		/* Write the magic header */
		strcpy ((char *)*buf, "AOC");
		exif_set_short (*buf + 4, order, (ExifShort) 0);
		break;

	case pentaxV1:
		/* It looks like this format doesn't have a magic header as
		 * such, just has a fixed number of entries equal to 0x001b */
		*buf_size -= 6;
		o2 -= 6;
		mem->exif_mem_alloc (buf, (*buf_size)/sizeof(unsigned char));
		if (!*buf) {
			EXIF_LOG_NO_MEMORY_PTR(log, "ExifMnoteDataPentax", *buf_size);
			return;
		}
		break;

	default:
		/* internal error */
		return;
	}

	/* Write the number of entries. */
	exif_set_short (*buf + o2, order, (ExifShort) count);
	o2 += 2;

	/* Save each entry */
	for (i = 0; i < count; i++) {
		size_t doff;	/* offset to current data portion of tag */
		size_t s;
		unsigned char *t;
		size_t o = o2 + i * 12;   /* current offset into output buffer */
		exif_set_short (*buf + o + 0, order,
				(ExifShort) (entries[i].tag - base));
		exif_set_short (*buf + o + 2, order,
				(ExifShort) entries[i].format);
		exif_set_long  (*buf + o + 4, order,
				entries[i].components);
		o += 8;
		s = exif_format_get_size (entries[i].format) *
						entries[i].components;
		if (s > 65536) {
			/* Corrupt data: EXIF data size is limited to the
			 * maximum size of a JPEG segment (64 kb).
			 */
			continue;
		}
		if (s > 4) {
			size_t ts = *buf_size + s;
			doff = *buf_size;
			t = mem->exif_mem_realloc (buf,(unsigned int)ts);
			if (!t) {
				EXIF_LOG_NO_MEMORY_PTR(log, "ExifMnoteDataPentax", ts);
				return;
			}
			*buf = t;
			*buf_size = ts;
			exif_set_long (*buf + o, order, datao + doff);
		} else
			doff = o;

		/* Write the data. */
		if (entries[i].data) {
			memcpy (*buf + doff, entries[i].data, s);
		} else {
			/* Most certainly damaged input file */
			memset (*buf + doff, 0, s);
		}
	}

	/* Sanity check the buffer size */
	if (*buf_size < (o2 + count * 12 + 4)) {
		log->exif_log(EXIF_LOG_CODE_CORRUPT_DATA, "ExifMnoteDataPentax",
			"Buffer overflow");
	}

	/* Reset next IFD pointer */
	exif_set_long (*buf + o2 + count * 12, order, 0);
}

void ExifMnoteDataPentax::load (const unsigned char *buf, unsigned int buf_size)
{
	size_t i, tcount, o, datao, base = 0;
	ExifShort c;

	if (!buf || !buf_size) {
		log->exif_log(EXIF_LOG_CODE_CORRUPT_DATA,
			  "ExifMnoteDataPentax", "Short MakerNote");
		return;
	}
	datao = 6 + offset;
	if ((datao + 8 < datao) || (datao + 8 < 8) || (datao + 8 > buf_size)) {
		log->exif_log(EXIF_LOG_CODE_CORRUPT_DATA,
			  "ExifMnoteDataPentax", "Short MakerNote");
		return;
	}

	/* Detect variant of Pentax/Casio MakerNote found */
	if (!memcmp(buf + datao, "AOC", 4)) {
		if ((buf[datao + 4] == 'I') && (buf[datao + 5] == 'I')) {
			version = pentaxV3;
			order = EXIF_BYTE_ORDER_INTEL;
		} else if ((buf[datao + 4] == 'M') && (buf[datao + 5] == 'M')) {
			version = pentaxV3;
			order = EXIF_BYTE_ORDER_MOTOROLA;
		} else {
			/* Uses Casio v2 tags */
			version = pentaxV2;
		}
		log->exif_log(EXIF_LOG_CODE_DEBUG, "ExifMnoteDataPentax",
			"Parsing Pentax maker note v%d...", (int)version);
		datao += 4 + 2;
		base = MNOTE_PENTAX2_TAG_BASE;
	} else if (!memcmp(buf + datao, "QVC", 4)) {
		log->exif_log(EXIF_LOG_CODE_DEBUG, "ExifMnoteDataPentax",
			"Parsing Casio maker note v2...");
		version = casioV2;
		base = MNOTE_CASIO2_TAG_BASE;
		datao += 4 + 2;
	} else {
		/* probably assert(!memcmp(buf + datao, "\x00\x1b", 2)) */
		log->exif_log(EXIF_LOG_CODE_DEBUG, "ExifMnoteDataPentax",
			"Parsing Pentax maker note v1...");
		version = pentaxV1;
	}

	/* Read the number of tags */
	c = exif_get_short (buf + datao, order);
	datao += 2;

	/* Remove any old entries */
	exif_mnote_data_pentax_clear ();

	/* Reserve enough space for all the possible MakerNote tags */
	mem->exif_mem_alloc (&entries, c);
	if (!entries) {
		EXIF_LOG_NO_MEMORY_PTR(log, "ExifMnoteDataPentax", sizeof (MnotePentaxEntry) * c);
		return;
	}

	/* Parse all c entries, storing ones that are successfully parsed */
	tcount = 0;
	for (i = c, o = datao; i; --i, o += 12) {
		size_t s;
		if ((o + 12 < o) || (o + 12 < 12) || (o + 12 > buf_size)) {
			log->exif_log(EXIF_LOG_CODE_CORRUPT_DATA,
				  "ExifMnoteDataPentax", "Short MakerNote");
			break;
		}

		entries[tcount].tag        =static_cast<MnotePentaxTag> (exif_get_short (buf + o + 0, order) + base);
		entries[tcount].format     = static_cast<ExifFormat>(exif_get_short (buf + o + 2, order));
		entries[tcount].components = exif_get_long  (buf + o + 4, order);
		entries[tcount].order      = order;

		log->exif_log(EXIF_LOG_CODE_DEBUG, "ExifMnotePentax",
			  "Loading entry 0x%x ('%s')...", entries[tcount].tag,
			  mnote_pentax_tag_get_name (entries[tcount].tag));

		/*
		 * Size? If bigger than 4 bytes, the actual data is not
		 * in the entry but somewhere else (offset).
		 */
		s = exif_format_get_size (entries[tcount].format) *
                                      entries[tcount].components;
		entries[tcount].size = s;
		if (s) {
			size_t dataofs = o + 8;
			if (s > 4)
				/* The data in this case is merely a pointer */
			   	dataofs = exif_get_long (buf + dataofs, order) + 6;
			if ((dataofs + s < dataofs) || (dataofs + s < s) ||
				(dataofs + s > buf_size)) {
				log->exif_log(EXIF_LOG_CODE_DEBUG,
						  "ExifMnoteDataPentax", "Tag data past end "
					  "of buffer (%u > %u)", dataofs + s, buf_size);
				continue;
			}

			mem->exif_mem_alloc (&entries[tcount].data, s/sizeof(unsigned char));
			if (!entries[tcount].data) {
				EXIF_LOG_NO_MEMORY_PTR(log, "ExifMnoteDataPentax", s);
				continue;
			}
			memcpy (entries[tcount].data, buf + dataofs, s);
		}

		/* Tag was successfully parsed */
		++tcount;
	}
	/* Store the count of successfully parsed tags */
	count = tcount;
}

unsigned int ExifMnoteDataPentax::get_count ()
{
	return count;
}

unsigned int ExifMnoteDataPentax::get_id (unsigned int n)
{
	if (count <= n) return 0;
	return entries[n].tag;
}

const char *ExifMnoteDataPentax::get_name (unsigned int n)
{
	if (count <= n) return NULL;
	return mnote_pentax_tag_get_name (entries[n].tag);
}

const char *ExifMnoteDataPentax::get_title (unsigned int n)
{
	if (count <= n) return NULL;
	return mnote_pentax_tag_get_title (entries[n].tag);
}

const char *ExifMnoteDataPentax::get_description (unsigned int n)
{
	if (count <= n) return NULL;
	return mnote_pentax_tag_get_description (entries[n].tag);
}

void ExifMnoteDataPentax::set_offset (unsigned int o)
{
	offset = o;
}

void ExifMnoteDataPentax::set_byte_order (ExifByteOrder o)
{
	ExifByteOrder o_orig;
	unsigned int i=0;

	o_orig = order;
	order = o;
	for (i = 0; i < count; i++) 
	{
		entries[i].order = o;
		exif_array_set_byte_order (entries[i].format, entries[i].data,
				entries[i].components, o_orig, o);
	}
}
