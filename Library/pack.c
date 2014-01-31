#define LITTLE_ENDIAN

#include "str2ptr.h"
#include "endian.h"
#include "fs.h"
#include "mem.h"
#include "user.h"
#include "str.h"

struct disk_pack_file {
	char name[54];
	int  fileofs;
	int  filelen;
};

struct disk_pack_header {
	char id[4]; // == IDPAKHEADER
	int  dirofs;
	int  dirlen;
};

struct pack_entry {
	unsigned int		size;
	unsigned int		offset;
	const unsigned char *data; // NULL if the data is in the store
};

struct pack {
	const char	     *filename;
	struct str2ptr   *lookup;
	FILE			 *store;
};

struct pack *load_pack(const char *filename, FILE *file) {
	struct disk_pack_header disk_header;
	struct disk_pack_file *in_file;
	struct pack_entry *entry, *entries;
	unsigned int num_files;
	struct pack *result;

	fread(&disk_header, sizeof(disk_header), 1, file);
	disk_header.dirofs = endian_little_long(disk_header.dirofs);
	disk_header.dirlen = endian_little_long(disk_header.dirlen);

	if(disk_header.id[0] != 'P' || disk_header.id[1] != 'A' || disk_header.id[2] != 'C' || disk_header.id[3] != 'K') {
		user_err("attempted to load pack file %s with invalid ident", filename);
		return NULL;
	}

	num_files = disk_header.dirlen / sizeof(*in_file);
	in_file = alloca(sizeof(*in_file) * num_files);
	fseek(file, disk_header.dirofs, SEEK_SET);
	fread(in_file, 1, disk_header.dirlen, file);

	result = malloc(sizeof(*result) + sizeof(*entry) * num_files);
	result->filename = string_dup(filename);
	result->lookup = new_str2ptr();
	result->store = file;

    entries = (struct pack_entry *)(result + 1);

	while(num_files--) {
		entry = entries++;
		entry->size = endian_little_long(in_file->filelen);
		entry->offset = endian_little_long(in_file->fileofs);
		entry->data = NULL;
		str2ptr_set(result->lookup, in_file->name, entry);
		++in_file;
	}

	return result;
}

const char *pack_filename(struct pack *pack) {
	return pack->filename;
}

unsigned int pack_num_entries(struct pack *pack) {
	return str2ptr_size(pack->lookup);
}

int pack_get(struct pack *pack, const char *filename, const unsigned char **data) {
	struct pack_entry *entry = str2ptr_get(pack->lookup, filename);
	if(entry) {
		*data = malloc(entry->size + 1);
		if(entry->data) {
			memcpy((char *)*data, entry->data, entry->size);
		} else {
			fseek(pack->store, entry->offset, SEEK_SET);
			fread((char *)*data, entry->size, 1, pack->store);
		}
		((char *)(*data))[entry->size] = '\0';
		return entry->size;
	}
	return -1;
}

int pack_get_file(struct pack *pack, const char *filename, FILE **file) {
	struct pack_entry *entry = str2ptr_get(pack->lookup, filename);
	*file = NULL;
	if(entry) {
        *file = fopen(pack->filename, "rb");
		fseek(*file, entry->offset, SEEK_SET);
		return entry->size;
	}
	return -1;
}

void pack_set(struct pack *pack, const char *filename, unsigned int size, const unsigned char *data) {
}

void pack_del(struct pack *pack, const char *filename) {
}

void pack_save(struct pack *pack, FILE *file) {
}

void pack_free(struct pack *pack) {
	free((void *)pack->filename);
	str2ptr_free(pack->lookup);
	fclose(pack->store);
	free(pack);
}
