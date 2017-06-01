/*! \file exif-mem.h
 *  \brief Define the ExifMem data type and the associated functions.
 *  ExifMem defines the memory management functions used within libexif.
 */
/* exif-mem.h
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

#ifndef __EXIF_MEM_H__
#define __EXIF_MEM_H__

#include "exif-utils.h"
#include <stdio.h>


class ExifMem 
{
public:
	template<typename T> inline void exif_mem_alloc(T **ReturnData, unsigned int ds)
	{
		exif_mem_free(ReturnData);

		if (ds == 1)
		{
			*ReturnData=new T;
		}
		if (ds > 1)
		{
			*ReturnData=new T[ds];
		}
	}

	template<typename T> inline void exif_mem_free(T **InputData)
	{
		if (*InputData)
		{
			delete [] *InputData;
		}
		*InputData=NULL;
	}

	template<typename T> inline T *exif_mem_realloc(T **InputData,unsigned int ds)
	{
		if (*InputData)
		{
			unsigned int length=sizeof(*InputData)/sizeof((*InputData)[0]);

			if (length < ds)
			{
				T *pData=new T[ds];
				memset(pData,0,ds*sizeof(T));
				memcpy(pData,*InputData, (ds-1)*sizeof(T));
				if (ds==2 || (length==1 && sizeof(T) ==1))
				{
					delete *InputData;
				}
				else
				{
					delete [] *InputData;
				}
				
				*InputData = pData;
			}
				
			return *InputData;

		}
		else
		{
			if (ds == 1)
			{
				*InputData=new T;
			}
			if (ds > 1)
			{
				*InputData=new T[ds];
			}
			return *InputData;
		}

		return NULL;

	}
public:
	ExifMem()
	{
	}
	~ExifMem()
	{
	}
};


#endif /* __EXIF_MEM_H__ */
