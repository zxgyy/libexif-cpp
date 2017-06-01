# libexif C++ version

Provides basic support for reading EXIF tags on files or memory image using libexif. 

## How to install

Get libexif for your OS:

# Note


```
When I use the c version libexif to read Nikon Camera's image, i found there's 
some memory leak, when i use the exif_data_unref(), the lib can free some 
data, but still there's 188 BYTE, there's also crash found in the capturing 
process. So i change the C Version libexif to C++ version of libexif.
```
