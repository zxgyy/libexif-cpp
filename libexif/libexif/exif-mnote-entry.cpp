#include <stdio.h>

#include "exif-mnote-entry.h"

void ExifMnoteEntry::data_free()
{
	if (data)
	{
		size>1?delete [] data:delete data;
		data=NULL;
		size=0;
	}
}
void ExifMnoteEntry::data_new(unsigned int s)
{
	data_free();
	size=s;
	data=new unsigned char[s];
}
