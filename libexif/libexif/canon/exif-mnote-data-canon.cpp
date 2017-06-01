/* exif-mnote-data-canon.c
 *
 * Copyright (c) 2002, 2003 Lutz Mueller <lutz@users.sourceforge.net>
 * Copyright (c) 2003 Matthieu Castet <mat-c@users.sourceforge.net>
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
#include "../exif-mem.h"
#include "exif-mnote-data-canon.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../exif-byte-order.h"
#include "../exif-utils.h"
#include "../exif-data.h"

#define DEBUG

void ExifMnoteDataCanon::exif_mnote_data_canon_clear ()
{
	if (entries) 
	{
		delete entries;
		entries = NULL;
		count = 0;
	}
}

void ExifMnoteDataCanon::free ()
{
	exif_mnote_data_canon_clear ();
}

void ExifMnoteDataCanon::exif_mnote_data_canon_get_tags (unsigned int n,unsigned int *m, unsigned int *s)
{
	unsigned int from = 0, to;

	if (!m) return;
	for (*m = 0; *m < count; (*m)++) {
		to = from + entries[*m].mnote_canon_entry_count_values ();
		if (to > n) {
			if (s) *s = n - from;
			break;
		}
		from = to;
	}
}

char * ExifMnoteDataCanon::get_value (unsigned int n, char *val, unsigned int maxlen)
{
	unsigned int m=0, s=0;

	exif_mnote_data_canon_get_tags (n, &m, &s);
	if (m >= count) return NULL;
	return entries[m].mnote_canon_entry_get_value (s, val, maxlen);
}

void ExifMnoteDataCanon::set_byte_order (ExifByteOrder o)
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

void ExifMnoteDataCanon::set_offset (unsigned int o)
{
	offset = o;
}

void ExifMnoteDataCanon::save(unsigned char **buf, unsigned int *buf_size)
{
	size_t i, o, s, doff;
	unsigned char *t;
	size_t ts;

	if (!buf || !buf_size) return;

	/*
	 * Allocate enough memory for all entries and the number
	 * of entries.
	 */
	*buf_size = 2 + count * 12 + 4;
	if (*buf)
	{
		delete [] *buf;
		*buf=NULL;
	}

	*buf=new unsigned char[*buf_size];
	if (!*buf) {
		EXIF_LOG_NO_MEMORY_PTR(log, "ExifMnoteCanon", *buf_size);
		return;
	}

	/* Save the number of entries */
	exif_set_short (*buf, order, (ExifShort) count);
	
	/* Save each entry */
	for (i = 0; i < count; i++) {
		o = 2 + i * 12;
		exif_set_short (*buf + o + 0, order, (ExifShort) entries[i].tag);
		exif_set_short (*buf + o + 2, order, (ExifShort) entries[i].format);
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
			ts = *buf_size + s;

			/* Ensure even offsets. Set padding bytes to 0. */
			if (s & 1) ts += 1;
			t= new unsigned char[ts];
			if (!t) {
				EXIF_LOG_NO_MEMORY_PTR(log, "ExifMnoteCanon", ts);
				return;
			}
			*buf = t;
			*buf_size = ts;
			doff = *buf_size - s;
			if (s & 1) { doff--; *(*buf + *buf_size - 1) = '\0'; }
			exif_set_long (*buf + o, order, offset + doff);
		} else
			doff = o;

		/*
		 * Write the data. Fill unneeded bytes with 0. Do not
		 * crash if data is NULL.
		 */
		if (!entries[i].data) memset (*buf + doff, 0, s);
		else memcpy (*buf + doff, entries[i].data, s);
		if (s < 4) memset (*buf + doff + s, 0, (4 - s));
	}
}

/* XXX
 * FIXME: exif_mnote_data_canon_load() may fail and there is no
 *        semantics to express that.
 *        See bug #1054323 for details, especially the comment by liblit
 *        after it has supposedly been fixed:
 *
 *        https://sourceforge.net/tracker/?func=detail&aid=1054323&group_id=12272&atid=112272
 *        Unfortunately, the "return" statements aren't commented at
 *        all, so it isn't trivial to find out what is a normal
 *        return, and what is a reaction to an error condition.
 */

void ExifMnoteDataCanon::load (const unsigned char *buf, unsigned int buf_size)
{
	ExifShort c;
	size_t i, tcount, o, datao;

	if (!buf || !buf_size) {
		log->exif_log(EXIF_LOG_CODE_CORRUPT_DATA,
			  "ExifMnoteCanon", "Short MakerNote");
		return;
	}
	datao = 6 + offset;
	if ((datao + 2 < datao) || (datao + 2 < 2) || (datao + 2 > buf_size)) {
		log->exif_log(EXIF_LOG_CODE_CORRUPT_DATA,
			  "ExifMnoteCanon", "Short MakerNote");
		return;
	}

	/* Read the number of tags */
	c = exif_get_short (buf + datao, order);
	datao += 2;

	/* Remove any old entries */
	exif_mnote_data_canon_clear ();

	/* Reserve enough space for all the possible MakerNote tags */
	if (entries)
	{
		delete entries;
		entries=NULL;
	}
	entries=new MnoteCanonEntry;
	if (!entries) {
		EXIF_LOG_NO_MEMORY_PTR(log, "ExifMnoteCanon", sizeof (MnoteCanonEntry) * c);
		return;
	}

	/* Parse the entries */
	tcount = 0;
	for (i = c, o = datao; i; --i, o += 12) {
		size_t s;
		if ((o + 12 < o) || (o + 12 < 12) || (o + 12 > buf_size)) {
			log->exif_log(EXIF_LOG_CODE_CORRUPT_DATA,
				"ExifMnoteCanon", "Short MakerNote");
			break;
	        }

		entries[tcount].tag        = static_cast<MnoteCanonTag>(exif_get_short (buf + o, order));
		entries[tcount].format     = static_cast<ExifFormat>(exif_get_short (buf + o + 2, order));
		entries[tcount].components = exif_get_long (buf + o + 4, order);
		entries[tcount].order      = order;

		log->exif_log(EXIF_LOG_CODE_DEBUG, "ExifMnoteCanon",
			"Loading entry 0x%x ('%s')...", entries[tcount].tag,
			 mnote_canon_tag_get_name (entries[tcount].tag));

		/*
		 * Size? If bigger than 4 bytes, the actual data is not
		 * in the entry but somewhere else (offset).
		 */
		s = exif_format_get_size (entries[tcount].format) * 
								  entries[tcount].components;
		entries[tcount].size = s;
		if (!s) {
			log->exif_log(EXIF_LOG_CODE_CORRUPT_DATA,
				  "ExifMnoteCanon",
				  "Invalid zero-length tag size");
			continue;

		} else {
			size_t dataofs = o + 8;
			if (s > 4) dataofs = exif_get_long (buf + dataofs, order) + 6;
			if ((dataofs + s < s) || (dataofs + s < dataofs) || (dataofs + s > buf_size)) {
				log->exif_log(EXIF_LOG_CODE_DEBUG,
					"ExifMnoteCanon",
					"Tag data past end of buffer (%u > %u)",
					dataofs + s, buf_size);
				continue;
			}

			entries[tcount].data_new(s);
			if (!entries[tcount].data) {
				EXIF_LOG_NO_MEMORY_PTR(log, "ExifMnoteCanon", s);
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

unsigned int ExifMnoteDataCanon::get_count ()
{
	unsigned int c=0;

	for (unsigned int i = 0; i < count; i++)
		c += entries[i].mnote_canon_entry_count_values ();
	return c;
}

unsigned int ExifMnoteDataCanon::get_id (unsigned int i)
{
	unsigned int m=0;

	exif_mnote_data_canon_get_tags (i, &m, NULL);
	if (m >= count) return 0;
	return entries[m].tag;
}

const char * ExifMnoteDataCanon::get_name (unsigned int i)
{
	unsigned int m, s;

	exif_mnote_data_canon_get_tags (i, &m, &s);
	if (m >= count) return NULL;
	return mnote_canon_tag_get_name_sub (entries[m].tag, s, options);
}

const char * ExifMnoteDataCanon::get_title (unsigned int i)
{
	unsigned int m, s;
	exif_mnote_data_canon_get_tags (i, &m, &s);
	if (m >= count) return NULL;
	return mnote_canon_tag_get_title_sub (entries[m].tag, s, options);
}

const char * ExifMnoteDataCanon::get_description (unsigned int i)
{
	unsigned int m=0;

	exif_mnote_data_canon_get_tags (i, &m, NULL);
	if (m >= count) return NULL;
	return mnote_canon_tag_get_description (entries[m].tag);
}
