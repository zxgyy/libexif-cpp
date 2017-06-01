/* exif-mnote-data-priv.h
 *
 * Copyright (c) 2003 Lutz Mueller <lutz@users.sourceforge.net>
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

#ifndef __EXIF_MNOTE_DATA_PRIV_H__
#define __EXIF_MNOTE_DATA_PRIV_H__



#include "exif-mnote-data.h"
#include "exif-byte-order.h"
#include "exif-log.h"
#include <stdio.h>



/*! \internal */
class ExifMnoteData 
{
public:
	ExifMnoteData()
	{
		Init();
	}

	void inline Init()
	{
		log=NULL;
		mem=NULL;
	}
	virtual ~ExifMnoteData ()
	{
		exif_mnote_data_free ();
	}
public:
	void exif_mnote_data_construct (ExifMem *mem);
	void exif_mnote_data_free ();
	void exif_mnote_data_load (const unsigned char *buf,	unsigned int buf_size);
	void exif_mnote_data_save (unsigned char **buf,unsigned int *buf_size);
	void exif_mnote_data_set_byte_order (ExifByteOrder o);
	void exif_mnote_data_set_offset (unsigned int o);
	unsigned int exif_mnote_data_count ();
	unsigned int exif_mnote_data_get_id (unsigned int n);
	const char *exif_mnote_data_get_name (unsigned int n);
	const char *exif_mnote_data_get_title (unsigned int n);;
	const char *exif_mnote_data_get_description (unsigned int n);
	char *exif_mnote_data_get_value (unsigned int n, char *val, unsigned int maxlen);
	void exif_mnote_data_log (ExifLog *log);

public:
	/* Life cycle */
	virtual void free()=0;

	/* Modification */
	virtual void save(unsigned char **, unsigned int *)=0;
	virtual void load(const unsigned char *, unsigned int)=0;
	virtual void set_offset(unsigned int)=0;
	virtual void set_byte_order(ExifByteOrder)=0;

	/* Query */
	virtual unsigned int get_count()=0;
	virtual unsigned int get_id(unsigned int)=0;
	virtual const char * get_name(unsigned int)=0;
	virtual const char * get_title(unsigned int)=0;
	virtual const char * get_description(unsigned int)=0;
	virtual char * get_value(unsigned int, char *val, unsigned int maxlen)=0;
public:
	/* Logging */
	ExifLog *log;

	/* Memory management */
	ExifMem *mem;
};

#endif /* __EXIF_MNOTE_PRIV_H__ */
