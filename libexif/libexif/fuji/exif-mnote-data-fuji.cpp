/* exif-mnote-data-fuji.c
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

#include <stdlib.h>
#include <string.h>


#include "../config.h"
#include "../exif-byte-order.h"
#include "../exif-utils.h"

#include "exif-mnote-data-fuji.h"

struct _MNoteFujiDataPrivate {
	ExifByteOrder order;

	_MNoteFujiDataPrivate()
	{
		Init();
	}
	void inline Init()
	{
		order = EXIF_BYTE_ORDER_MOTOROLA;
	}
};

void ExifMnoteDataFuji::exif_mnote_data_fuji_clear ()
{
	unsigned int i=0;

	if (entries) 
	{
		count>1?delete [] entries:delete entries;
		entries=NULL;
		count = 0;
	}
}

void ExifMnoteDataFuji::free ()
{
	exif_mnote_data_fuji_clear ();
}

char *ExifMnoteDataFuji::get_value (unsigned int i, char *val, unsigned int maxlen)
{
	if (!val) return NULL;
	if (i > count -1) return NULL;
/*
	exif_log (log, EXIF_LOG_CODE_DEBUG, "ExifMnoteDataFuji",
		  "Querying value for tag '%s'...",
		  mnote_fuji_tag_get_name (entries[i].tag));
*/
	return entries[i].mnote_fuji_entry_get_value (val, maxlen);
}

void ExifMnoteDataFuji::save (unsigned char **buf, unsigned int *buf_size)
{
	size_t i, o, s, doff;
	unsigned char *t;
	size_t ts;

	if (!buf || !buf_size) return;

	/*
	 * Allocate enough memory for all entries and the number
	 * of entries.
	 */
	*buf_size = 8 + 4 + 2 + count * 12 + 4;
	mem->exif_mem_alloc (buf, (*buf_size)/sizeof(unsigned char));
	if (!*buf) {
		*buf_size = 0;
		return;
	}

	/*
	 * Header: "FUJIFILM" and 4 bytes offset to the first entry.
	 * As the first entry will start right thereafter, the offset is 12.
	 */
	memcpy (*buf, "FUJIFILM", 8);
	exif_set_long (*buf + 8, order, 12);

	/* Save the number of entries */
	exif_set_short (*buf + 8 + 4, order, (ExifShort) count);
	
	/* Save each entry */
	for (i = 0; i < count; i++) {
		o = 8 + 4 + 2 + i * 12;
		exif_set_short (*buf + o + 0, order, (ExifShort) entries[i].tag);
		exif_set_short (*buf + o + 2, order, (ExifShort) entries[i].format);
		exif_set_long  (*buf + o + 4, order, entries[i].components);
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
			ts = *buf_size + s;

			/* Ensure even offsets. Set padding bytes to 0. */
			if (s & 1) ts += 1;
			t =mem->exif_mem_realloc (buf,ts/sizeof(unsigned char));
			if (!t) {
				return;
			}
			*buf = t;
			*buf_size = ts;
			doff = *buf_size - s;
			if (s & 1) { doff--; *(*buf + *buf_size - 1) = '\0'; }
			exif_set_long (*buf + o, order, doff);
		} else
			doff = o;

		/*
		 * Write the data. Fill unneeded bytes with 0. Do not
		 * crash if data is NULL.
		 */
		if (!entries[i].data) memset (*buf + doff, 0, s);
		else memcpy (*buf + doff, entries[i].data, s);
	}
}

void ExifMnoteDataFuji::load (const unsigned char *buf, unsigned int buf_size)
{
	ExifLong c;
	size_t i, tcount, o, datao;

	if (!buf || !buf_size) {
		log->exif_log(EXIF_LOG_CODE_CORRUPT_DATA,
			  "ExifMnoteDataFuji", "Short MakerNote");
		return;
	}
	datao = 6 + offset;
	if ((datao + 12 < datao) || (datao + 12 < 12) || (datao + 12 > buf_size)) {
		log->exif_log(EXIF_LOG_CODE_CORRUPT_DATA,
			  "ExifMnoteDataFuji", "Short MakerNote");
		return;
	}

	order = EXIF_BYTE_ORDER_INTEL;
	datao += exif_get_long (buf + datao + 8, EXIF_BYTE_ORDER_INTEL);
	if ((datao + 2 < datao) || (datao + 2 < 2) ||
	    (datao + 2 > buf_size)) {
		log->exif_log(EXIF_LOG_CODE_CORRUPT_DATA,
			  "ExifMnoteDataFuji", "Short MakerNote");
		return;
	}

	/* Read the number of tags */
	c = exif_get_short (buf + datao, EXIF_BYTE_ORDER_INTEL);
	datao += 2;

	/* Remove any old entries */
	exif_mnote_data_fuji_clear ();

	/* Reserve enough space for all the possible MakerNote tags */
	mem->exif_mem_alloc (&entries, c);
	if (!entries) {
		EXIF_LOG_NO_MEMORY_PTR(log, "ExifMnoteDataFuji", sizeof (MnoteFujiEntry) * c);
		return;
	}

	/* Parse all c entries, storing ones that are successfully parsed */
	tcount = 0;
	for (i = c, o = datao; i; --i, o += 12) {
		size_t s;
		if ((o + 12 < o) || (o + 12 < 12) || (o + 12 > buf_size)) {
			log->exif_log(EXIF_LOG_CODE_CORRUPT_DATA,
				  "ExifMnoteDataFuji", "Short MakerNote");
			break;
		}

		entries[tcount].tag        = static_cast<MnoteFujiTag>(exif_get_short (buf + o, order));
		entries[tcount].format     = static_cast<ExifFormat>(exif_get_short (buf + o + 2, order));
		entries[tcount].components = exif_get_long (buf + o + 4, order);
		entries[tcount].order      = order;

		log->exif_log(EXIF_LOG_CODE_DEBUG, "ExifMnoteDataFuji",
			  "Loading entry 0x%x ('%s')...", entries[tcount].tag,
			  mnote_fuji_tag_get_name (entries[tcount].tag));

		/*
		 * Size? If bigger than 4 bytes, the actual data is not
		 * in the entry but somewhere else (offset).
		 */
		s = exif_format_get_size (entries[tcount].format) * entries[tcount].components;
		entries[tcount].size = s;
		if (s) {
			size_t dataofs = o + 8;
			if (s > 4)
				/* The data in this case is merely a pointer */
				dataofs = exif_get_long (buf + dataofs, order) + 6 + offset;
			if ((dataofs + s < dataofs) || (dataofs + s < s) ||
				(dataofs + s >= buf_size)) {
				log->exif_log(EXIF_LOG_CODE_CORRUPT_DATA,
						  "ExifMnoteDataFuji", "Tag data past end of "
					  "buffer (%u >= %u)", dataofs + s, buf_size);
				continue;
			}

			mem->exif_mem_alloc (&entries[tcount].data, s/sizeof(unsigned char));
			if (!entries[tcount].data) {
				EXIF_LOG_NO_MEMORY_PTR(log, "ExifMnoteDataFuji", s);
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

unsigned int ExifMnoteDataFuji::get_count ()
{
	return count;
}

unsigned int ExifMnoteDataFuji::get_id (unsigned int n)
{
	if (count <= n) return 0;
	return entries[n].tag;
}

const char *ExifMnoteDataFuji::get_name (unsigned int i)
{
	if (i >= count) return NULL;
	return mnote_fuji_tag_get_name (entries[i].tag);
}

const char *ExifMnoteDataFuji::get_title (unsigned int i)
{
	if (i >= count) return NULL;
	return mnote_fuji_tag_get_title (entries[i].tag);
}

const char *ExifMnoteDataFuji::get_description (unsigned int i)
{
	if (i >= count) return NULL;
	return mnote_fuji_tag_get_description (entries[i].tag);
}

void ExifMnoteDataFuji::set_byte_order (ExifByteOrder o)
{
	ExifByteOrder o_orig;
	unsigned int i;

	o_orig = order;
	order = o;
	for (i = 0; i < count; i++) {
		entries[i].order = o;
		exif_array_set_byte_order (entries[i].format, entries[i].data,
				entries[i].components, o_orig, o);
	}
}

void ExifMnoteDataFuji::set_offset (unsigned int o)
{
	offset = o;
}