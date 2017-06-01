/* exif-mnote-data-olympus.c
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
#include "exif-mnote-data-olympus.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../exif-utils.h"
#include "../exif-data.h"

#define DEBUG


/* Uncomment this to fix a problem with Sanyo MakerNotes. It's probably best
 * not to in most cases because it seems to only affect the thumbnail tag
 * which is duplicated in IFD 1, and fixing the offset could actually cause
 * problems with other software that expects the broken form.
 */
/*#define EXIF_OVERCOME_SANYO_OFFSET_BUG */

void ExifMnoteDataOlympus::exif_mnote_data_olympus_clear ()
{
	unsigned int i=0;

	if (entries) 
	{
		count>1?delete [] entries:delete entries;
		entries=NULL;
	}

	entries=NULL;
	count=0;
}

void ExifMnoteDataOlympus::free ()
{
	exif_mnote_data_olympus_clear ();
	
}

char *ExifMnoteDataOlympus::get_value (unsigned int i, char *val, unsigned int maxlen)
{
	if (!val) return NULL;
	if (i > count -1) return NULL;
/*
	exif_log (log, EXIF_LOG_CODE_DEBUG, "ExifMnoteDataOlympus",
		  "Querying value for tag '%s'...",
		  mnote_olympus_tag_get_name (entries[i].tag));
*/
	return entries[i].mnote_olympus_entry_get_value (val, maxlen);
}

/** 
 * @brief save the MnoteData from ne to buf
 * 
 * @param ne extract the data from this structure 
 * @param *buf write the mnoteData to this buffer (buffer will be allocated)
 * @param buf_size the size of the buffer
 */
void ExifMnoteDataOlympus::save (unsigned char **buf, unsigned int *buf_size)
{
	size_t i, o, s, doff, base = 0, o2 = 6 + 2;
	size_t datao = 0;
	unsigned char *t;
	size_t ts;

	if (!buf || !buf_size) return;

	/*
	 * Allocate enough memory for all entries and the number of entries.
	 */
	*buf_size = 6 + 2 + 2 + count * 12;
	switch (version) {
	case olympusV1:
	case sanyoV1:
	case epsonV1:
		if (*buf)
		{
			(*buf_size)>1?delete [] *buf:delete *buf;
			*buf=NULL;
		}
		
		mem->exif_mem_alloc (buf,(*buf_size)/sizeof(unsigned char));
		if (!*buf) {
			EXIF_LOG_NO_MEMORY_PTR(log, "ExifMnoteDataOlympus", *buf_size);
			return;
		}

		/* Write the header and the number of entries. */
		strcpy ((char *)*buf, version==sanyoV1?"SANYO":
					(version==epsonV1?"EPSON":"OLYMP"));
		exif_set_short (*buf + 6, order, (ExifShort) 1);
		datao = offset;
		break;

	case olympusV2:
		if (*buf)
		{
			(*buf_size)>1?delete [] *buf:delete *buf;
			*buf=NULL;
		}
		*buf_size += 8-6 + 4;
		mem->exif_mem_alloc (buf,(*buf_size)/sizeof(unsigned char));
		if (!*buf) {
			EXIF_LOG_NO_MEMORY_PTR(log, "ExifMnoteDataOlympus", *buf_size);
			return;
		}

		/* Write the header and the number of entries. */
		strcpy ((char *)*buf, "OLYMPUS");
		exif_set_short (*buf + 8, order, (ExifShort) (
			(order == EXIF_BYTE_ORDER_INTEL) ?
			('I' << 8) | 'I' :
			('M' << 8) | 'M'));
		exif_set_short (*buf + 10, order, (ExifShort) 3);
		o2 += 4;
		break;

	case nikonV1: 
		base = MNOTE_NIKON1_TAG_BASE;

		/* v1 has offsets based to main IFD, not makernote IFD */
		datao += offset + 10;
		/* subtract the size here, so the increment in the next case will not harm us */
		*buf_size -= 8 + 2;
	/* Fall through to nikonV2 handler */
	case nikonV2: 
	/* Write out V0 files in V2 format */
	case nikonV0:
		if (*buf)
		{
			(*buf_size)>1?delete [] *buf:delete *buf;
			*buf=NULL;
		}
		*buf_size += 8 + 2;
		*buf_size += 4; /* Next IFD pointer */
		mem->exif_mem_alloc (buf,(*buf_size)/sizeof(unsigned char));
		if (!*buf) {
			EXIF_LOG_NO_MEMORY_PTR(log, "ExifMnoteDataOlympus", *buf_size);
			return;
		}

		/* Write the header and the number of entries. */
		strcpy ((char *)*buf, "Nikon");
		(*buf)[6] = version;

		if (version != nikonV1) {
			exif_set_short (*buf + 10, order, (ExifShort) (
				(order == EXIF_BYTE_ORDER_INTEL) ?
				('I' << 8) | 'I' :
				('M' << 8) | 'M'));
			exif_set_short (*buf + 12, order, (ExifShort) 0x2A);
			exif_set_long (*buf + 14, order, (ExifShort) 8);
			o2 += 2 + 8;
		}
		datao -= 10;
		/* Reset next IFD pointer */
		exif_set_long (*buf + o2 + 2 + count * 12, order, 0);
		break;

	default:
		return;
	}

	exif_set_short (*buf + o2, order, (ExifShort) count);
	o2 += 2;

	/* Save each entry */
	for (i = 0; i < count; i++) {
		o = o2 + i * 12;
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
			doff = *buf_size;
			ts = *buf_size + s;
			t =mem->exif_mem_realloc (buf,ts/sizeof(unsigned char));
			if (!t) {
				EXIF_LOG_NO_MEMORY_PTR(log, "ExifMnoteDataOlympus", ts);
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
}

void ExifMnoteDataOlympus::load (const unsigned char *buf, unsigned int buf_size)
{
	ExifShort c;
	size_t i, tcount, o, o2, datao = 6, base = 0;

	if (!buf || !buf_size) {
		log->exif_log(EXIF_LOG_CODE_CORRUPT_DATA,
			  "ExifMnoteDataOlympus", "Short MakerNote");
		return;
	}
	o2 = 6 + offset; /* Start of interesting data */
	if ((o2 + 10 < o2) || (o2 + 10 < 10) || (o2 + 10 > buf_size)) {
		log->exif_log(EXIF_LOG_CODE_CORRUPT_DATA,
			  "ExifMnoteDataOlympus", "Short MakerNote");
		return;
	}

	/*
	 * Olympus headers start with "OLYMP" and need to have at least
	 * a size of 22 bytes (6 for 'OLYMP', 2 other bytes, 2 for the
	 * number of entries, and 12 for one entry.
	 *
	 * Sanyo format is identical and uses identical tags except that
	 * header starts with "SANYO".
	 *
	 * Epson format is identical and uses identical tags except that
	 * header starts with "EPSON".
	 *
	 * Nikon headers start with "Nikon" (6 bytes including '\0'), 
	 * version number (1 or 2).
	 * 
	 * Version 1 continues with 0, 1, 0, number_of_tags,
	 * or just with number_of_tags (models D1H, D1X...).
	 * 
	 * Version 2 continues with an unknown byte (0 or 10),
	 * two unknown bytes (0), "MM" or "II", another byte 0 and 
	 * lastly 0x2A.
	 */
	version = exif_mnote_data_olympus_identify_variant(buf+o2, buf_size-o2);
	switch (version) {
	case olympusV1:
	case sanyoV1:
	case epsonV1:
		log->exif_log(EXIF_LOG_CODE_DEBUG, "ExifMnoteDataOlympus",
			"Parsing Olympus/Sanyo/Epson maker note v1...");

		/* The number of entries is at position 8. */
		if (buf[o2 + 6] == 1)
			order = EXIF_BYTE_ORDER_INTEL;
		else if (buf[o2 + 6 + 1] == 1)
			order = EXIF_BYTE_ORDER_MOTOROLA;
		o2 += 8;
		if (o2 + 2 > buf_size) return;
		c = exif_get_short (buf + o2, order);
		if ((!(c & 0xFF)) && (c > 0x500)) {
			if (order == EXIF_BYTE_ORDER_INTEL) {
				order = EXIF_BYTE_ORDER_MOTOROLA;
			} else {
				order = EXIF_BYTE_ORDER_INTEL;
			}
		}
		break;

	case olympusV2:
		/* Olympus S760, S770 */
		datao = o2;
		o2 += 8;
		log->exif_log(EXIF_LOG_CODE_DEBUG, "ExifMnoteDataOlympus",
			"Parsing Olympus maker note v2 (0x%02x, %02x, %02x, %02x)...",
			buf[o2], buf[o2 + 1], buf[o2 + 2], buf[o2 + 3]);

		if ((buf[o2] == 'I') && (buf[o2 + 1] == 'I'))
			order = EXIF_BYTE_ORDER_INTEL;
		else if ((buf[o2] == 'M') && (buf[o2 + 1] == 'M'))
			order = EXIF_BYTE_ORDER_MOTOROLA;

		/* The number of entries is at position 8+4. */
		o2 += 4;
		break;

	case nikonV1:
		o2 += 6;
		if (o2 >= buf_size) return;
		log->exif_log(EXIF_LOG_CODE_DEBUG, "ExifMnoteDataOlympus",
			"Parsing Nikon maker note v1 (0x%02x, %02x, %02x, "
			"%02x, %02x, %02x, %02x, %02x)...",
			buf[o2 + 0], buf[o2 + 1], buf[o2 + 2], buf[o2 + 3], 
			buf[o2 + 4], buf[o2 + 5], buf[o2 + 6], buf[o2 + 7]);

		/* Skip version number */
		o2 += 1;

		/* Skip an unknown byte (00 or 0A). */
		o2 += 1;

		base = MNOTE_NIKON1_TAG_BASE;
		/* Fix endianness, if needed */
		if (o2 + 2 > buf_size) return;
		c = exif_get_short (buf + o2, order);
		if ((!(c & 0xFF)) && (c > 0x500)) {
			if (order == EXIF_BYTE_ORDER_INTEL) {
				order = EXIF_BYTE_ORDER_MOTOROLA;
			} else {
				order = EXIF_BYTE_ORDER_INTEL;
			}
		}
		break;

	case nikonV2:
		o2 += 6;
		if (o2 >= buf_size) return;
		log->exif_log(EXIF_LOG_CODE_DEBUG, "ExifMnoteDataOlympus",
			"Parsing Nikon maker note v2 (0x%02x, %02x, %02x, "
			"%02x, %02x, %02x, %02x, %02x)...",
			buf[o2 + 0], buf[o2 + 1], buf[o2 + 2], buf[o2 + 3], 
			buf[o2 + 4], buf[o2 + 5], buf[o2 + 6], buf[o2 + 7]);

		/* Skip version number */
		o2 += 1;

		/* Skip an unknown byte (00 or 0A). */
		o2 += 1;

		/* Skip 2 unknown bytes (00 00). */
		o2 += 2;

		/*
		 * Byte order. From here the data offset
		 * gets calculated.
		 */
		datao = o2;
		if (o2 >= buf_size) return;
		if (!strncmp ((char *)&buf[o2], "II", 2))
			order = EXIF_BYTE_ORDER_INTEL;
		else if (!strncmp ((char *)&buf[o2], "MM", 2))
			order = EXIF_BYTE_ORDER_MOTOROLA;
		else {
			log->exif_log(EXIF_LOG_CODE_DEBUG,
				"ExifMnoteDataOlympus", "Unknown "
				"byte order '%c%c'", buf[o2],
				buf[o2 + 1]);
			return;
		}
		o2 += 2;

		/* Skip 2 unknown bytes (00 2A). */
		o2 += 2;

		/* Go to where the number of entries is. */
		if (o2 + 4 > buf_size) return;
		o2 = datao + exif_get_long (buf + o2, order);
		break;

	case nikonV0:
		log->exif_log(EXIF_LOG_CODE_DEBUG, "ExifMnoteDataOlympus",
			"Parsing Nikon maker note v0 (0x%02x, %02x, %02x, "
			"%02x, %02x, %02x, %02x, %02x)...",
			buf[o2 + 0], buf[o2 + 1], buf[o2 + 2], buf[o2 + 3], 
			buf[o2 + 4], buf[o2 + 5], buf[o2 + 6], buf[o2 + 7]);
		/* 00 1b is # of entries in Motorola order - the rest should also be in MM order */
		order = EXIF_BYTE_ORDER_MOTOROLA;
		break;
	
	default:
		log->exif_log(EXIF_LOG_CODE_DEBUG, "ExifMnoteDataOlympus",
			"Unknown Olympus variant %i.", version);
		return;
	}

	/* Sanity check the offset */
	if ((o2 + 2 < o2) || (o2 + 2 < 2) || (o2 + 2 > buf_size)) {
		log->exif_log(EXIF_LOG_CODE_CORRUPT_DATA,
			  "ExifMnoteOlympus", "Short MakerNote");
		return;
	}

	/* Read the number of tags */
	c = exif_get_short (buf + o2, order);
	o2 += 2;

	/* Remove any old entries */
	exif_mnote_data_olympus_clear ();

	mem->exif_mem_alloc (&entries,c);
	if (!entries) {
		EXIF_LOG_NO_MEMORY_PTR(log, "ExifMnoteOlympus", sizeof (MnoteOlympusEntry) * c);
		return;
	}

	/* Parse all c entries, storing ones that are successfully parsed */
	tcount = 0;
	for (i = c, o = o2; i; --i, o += 12) {
		size_t s;
		if ((o + 12 < o) || (o + 12 < 12) || (o + 12 > buf_size)) {
			log->exif_log(EXIF_LOG_CODE_CORRUPT_DATA,
				  "ExifMnoteOlympus", "Short MakerNote");
			break;
		}

	    entries[tcount].tag        = static_cast<MnoteOlympusTag>(exif_get_short (buf + o, order) + base);
	    entries[tcount].format     = static_cast<ExifFormat>(exif_get_short (buf + o + 2, order));
	    entries[tcount].components = exif_get_long (buf + o + 4, order);
	    entries[tcount].order      = order;

	    log->exif_log(EXIF_LOG_CODE_DEBUG, "ExifMnoteOlympus",
		      "Loading entry 0x%x ('%s')...", entries[tcount].tag,
		      mnote_olympus_tag_get_name (entries[tcount].tag));
/*	    log->exif_log(EXIF_LOG_CODE_DEBUG, "ExifMnoteOlympus",
			    "0x%x %d %ld*(%d)",
		    entries[tcount].tag,
		    entries[tcount].format,
		    entries[tcount].components,
		    (int)exif_format_get_size(entries[tcount].format)); */

	    /*
	     * Size? If bigger than 4 bytes, the actual data is not
	     * in the entry but somewhere else (offset).
	     */
	    s = exif_format_get_size (entries[tcount].format) *
		   			 entries[tcount].components;
		entries[tcount].size = s;
		if (s) {
			size_t dataofs = o + 8;
			if (s > 4) {
				/* The data in this case is merely a pointer */
				dataofs = exif_get_long (buf + dataofs, order) + datao;
#ifdef EXIF_OVERCOME_SANYO_OFFSET_BUG
				/* Some Sanyo models (e.g. VPC-C5, C40) suffer from a bug when
				 * writing the offset for the MNOTE_OLYMPUS_TAG_THUMBNAILIMAGE
				 * tag in its MakerNote. The offset is actually the absolute
				 * position in the file instead of the position within the IFD.
				 */
			    if (dataofs + s > buf_size && version == sanyoV1) {
					/* fix pointer */
					dataofs -= datao + 6;
					log->exif_log(EXIF_LOG_CODE_DEBUG,
						  "ExifMnoteOlympus",
						  "Inconsistent thumbnail tag offset; attempting to recover");
			    }
#endif
			}
			if ((dataofs + s < dataofs) || (dataofs + s < s) || 
			    (dataofs + s > buf_size)) {
				log->exif_log(EXIF_LOG_CODE_DEBUG,
					  "ExifMnoteOlympus",
					  "Tag data past end of buffer (%u > %u)",
					  dataofs + s, buf_size);
				continue;
			}

			entries[tcount].data_free();
			mem->exif_mem_alloc (&entries[tcount].data,s/sizeof(unsigned char));
			if (!entries[tcount].data) 
			{
				EXIF_LOG_NO_MEMORY_PTR(log, "ExifMnoteOlympus", s);
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

unsigned int ExifMnoteDataOlympus::get_count ()
{
	return count;
}

unsigned int ExifMnoteDataOlympus::get_id (unsigned int n)
{
	if (count <= n) return 0;
	return entries[n].tag;
}

const char * ExifMnoteDataOlympus::get_name (unsigned int i)
{
	if (i >= count) return NULL;
	return mnote_olympus_tag_get_name (entries[i].tag);
}

const char * ExifMnoteDataOlympus::get_title (unsigned int i)
{
	if (i >= count) return NULL;
    return mnote_olympus_tag_get_title (entries[i].tag);
}

const char *ExifMnoteDataOlympus::get_description (unsigned int i)
{
	if (i >= count) return NULL;
    return mnote_olympus_tag_get_description (entries[i].tag);
}

void ExifMnoteDataOlympus::set_byte_order (ExifByteOrder o)
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

void ExifMnoteDataOlympus::set_offset (unsigned int o)
{
	offset = o;
}

enum OlympusVersion exif_mnote_data_olympus_identify_variant (const unsigned char *buf,unsigned int buf_size)
{
	/* Olympus, Nikon, Sanyo, Epson */
	if (buf_size >= 8) {
		/* Match the terminating NUL character, too */
		if (!memcmp (buf, "OLYMPUS", 8))
			return olympusV2;
		else if (!memcmp (buf, "OLYMP", 6))
			return olympusV1;
		else if (!memcmp (buf, "SANYO", 6))
			return sanyoV1;
		else if (!memcmp (buf, "EPSON", 6))
			return epsonV1;
		else if (!memcmp (buf, "Nikon", 6)) {
			switch (buf[6]) {
			case 1:  return nikonV1;
			case 2:  return nikonV2;
			default: return unrecognized; /* Unrecognized Nikon variant */
			}
		}
	}

	/* Another variant of Nikon */
	if ((buf_size >= 2) && (buf[0] == 0x00) && (buf[1] == 0x1b)) {
		return nikonV0;
	}

	return unrecognized;
}
