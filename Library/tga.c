#include "endian.h"
#include "user.h"
#include "mem.h"

int tga_load(unsigned char *buffer, unsigned char **pic, unsigned int *width, unsigned int *height) {
	struct {
		unsigned char 	id_length, colormap_type, image_type;
		unsigned short	colormap_index, colormap_length;
		unsigned char	colormap_size;
		unsigned short	x_origin, y_origin, width, height;
		unsigned char	pixel_size, attributes;
	} targa_header;
	unsigned char *buf_p = buffer, *targa_rgba, *pixbuf;
	unsigned int columns, rows, column, row;

	targa_header.id_length = *buf_p++;
	targa_header.colormap_type = *buf_p++;
	targa_header.image_type = *buf_p++;
	targa_header.colormap_index = endian_little_short(*(short *)buf_p);
	buf_p += 2;
	targa_header.colormap_length = endian_little_short(*(short *)buf_p);
	buf_p += 2;
	targa_header.colormap_size = *buf_p++;
	targa_header.x_origin = endian_little_short(*(short *)buf_p);
	buf_p += 2;
	targa_header.y_origin = endian_little_short(*(short *)buf_p);
	buf_p += 2;
	targa_header.width = endian_little_short(*(short *)buf_p);
	buf_p += 2;
	targa_header.height = endian_little_short(*(short *)buf_p);
	buf_p += 2;
	targa_header.pixel_size = *buf_p++;
	targa_header.attributes = *buf_p++;

	if(targa_header.image_type != 2 && targa_header.image_type != 10) {
		user_err("LoadTGA: Only type 2 and 10 targa RGB images supported\n");
		return 0;
	}

	if(targa_header.colormap_type != 0 || (targa_header.pixel_size != 32 && targa_header.pixel_size != 24)) {
		user_err("LoadTGA: Only 32 or 24 bit images supported (no colormaps)\n");
		return 0;
	}

	columns = targa_header.width;
	rows = targa_header.height;

	if(width) {
		*width = columns;
	}

	if(height) {
		*height = rows;
	}

	targa_rgba = malloc(columns * rows * 4);
	*pic = targa_rgba;

	if(targa_header.id_length != 0) {
		buf_p += targa_header.id_length;
	}
	
	if(targa_header.image_type == 2) {  // Uncompressed, RGB images
		for(row = rows - 1; row >= 0; row--) {
			pixbuf = targa_rgba + row*columns*4;
			for(column = 0; column < columns; column++) {
				unsigned char red,green,blue,alphabyte;
				switch(targa_header.pixel_size) {
				case 24:
					blue = *buf_p++;
					green = *buf_p++;
					red = *buf_p++;
					*pixbuf++ = red;
					*pixbuf++ = green;
					*pixbuf++ = blue;
					*pixbuf++ = 255;
					break;
				case 32:
					blue = *buf_p++;
					green = *buf_p++;
					red = *buf_p++;
					alphabyte = *buf_p++;
					*pixbuf++ = red;
					*pixbuf++ = green;
					*pixbuf++ = blue;
					*pixbuf++ = alphabyte;
					break;
				}
			}
		}
	} else if(targa_header.image_type == 10) {
		// Runlength encoded RGB images
		unsigned char red,green,blue,alphabyte,packetHeader,packetSize,j;
		for(row = rows - 1; row >= 0; row--) {
			pixbuf = targa_rgba + row*columns*4;
			for(column = 0; column < columns; ) {
				packetHeader= *buf_p++;
				packetSize = 1 + (packetHeader & 0x7f);
				if(packetHeader & 0x80) {        // run-length packet
					switch (targa_header.pixel_size) {
					case 24:
						blue = *buf_p++;
						green = *buf_p++;
						red = *buf_p++;
						alphabyte = 255;
						break;
					case 32:
						blue = *buf_p++;
						green = *buf_p++;
						red = *buf_p++;
						alphabyte = *buf_p++;
						break;
					}
	
					for(j=0;j<packetSize;j++) {
						*pixbuf++=red;
						*pixbuf++=green;
						*pixbuf++=blue;
						*pixbuf++=alphabyte;
						column++;
						if(column == columns) {
							// run spans across rows
							column = 0;
							if(row > 0) {
								row--;
							} else {
								goto breakOut;
							}
							pixbuf = targa_rgba + row*columns*4;
						}
					}
				} else {
					// non run-length packet
					for(j=0;j<packetSize;j++) {
						switch (targa_header.pixel_size) {
						case 24:
							blue = *buf_p++;
							green = *buf_p++;
							red = *buf_p++;
							*pixbuf++ = red;
							*pixbuf++ = green;
							*pixbuf++ = blue;
							*pixbuf++ = 255;
							break;
						case 32:
							blue = *buf_p++;
							green = *buf_p++;
							red = *buf_p++;
							alphabyte = *buf_p++;
							*pixbuf++ = red;
							*pixbuf++ = green;
							*pixbuf++ = blue;
							*pixbuf++ = alphabyte;
							break;
						}
						column++;
						if(column == columns) {
							// pixel packet run spans across rows
							column=0;
							if(row > 0) {
								row--;
							} else {
								goto breakOut;
							}
							pixbuf = targa_rgba + row*columns*4;
						}						
					}
				}
			}
			breakOut:;
		}
	}

	return 1;
}
