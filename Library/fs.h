#ifndef CORE_FS_H
#define CORE_FS_H

#include <stdio.h>

enum mount_flag {
	MF_WRITE   = (1 << 0), // sets this mount as the path to use for writing new files
	MF_NO_PACK = (1 << 1)  // do not scan for packs
};

enum search_flag {
	SF_ARCH   = (1 << 0),
	SF_HIDDEN = (1 << 1),
	SF_RDONLY = (1 << 2),
	SF_SUBDIR = (1 << 3),
	SF_SYSTEM = (1 << 4)
};

void fs_init(void);
void fs_mount(const char *, unsigned int);
void fs_umount(const char *);

FILE *fs_open(const char *, const char *);
int fs_rename(const char *, const char *);
int fs_copy(const char *, const char *);
int fs_remove(const char *);
int fs_mkdir(const char *);
int fs_exists(const char *, unsigned int white, unsigned int black);

int fs_read(const char *, const unsigned char **);
void fs_write(const char *, unsigned int, const unsigned char *);

struct fs_search *new_fs_search(const char *pattern, unsigned int white, unsigned int black);
const char *fs_search_next(struct fs_search *);
void fs_search_free(struct fs_search *);

#define with_fs_search(PATTERN, WHITE, BLACK, NEEDLE) for(struct fs_search *_search = new_fs_search(PATTERN, WHITE, BLACK); NEEDLE || (fs_search_free(_search), 0); NEEDLE = fs_search_next(_search))

#endif
