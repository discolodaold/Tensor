#include "fs.h"

#include "shell.h"
#include "pack.h"
#include "mem.h"
#include "str.h"
#include "array.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if _WIN32
#include <direct.h>
#include <windows.h>
#define mkdir(P, F) _mkdir(P)
#undef min
#undef max
#endif

#if __linux__
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#endif

struct mount {
	const char   *directory;
	array(struct pack *) packs;
};

struct fs_search {
	unsigned int white;
	unsigned int black;

#if _WIN32
	HANDLE		  find;
	WIN32_FIND_DATA ffd;
	BOOL			last_next;
#endif
#if __linux__
	const char *pattern;
	const char *path;
	DIR *dir;
#endif
};

static array(struct mount *) _mounts;
static struct mount *_write_mount;

static struct mount *_get_or_create(const char *directory) {
	struct mount **mountptr;
	struct mount *mount;

	array_for(_mounts, mountptr) {
		if(string_icmp((*mountptr)->directory, directory) == 0) {
			return *mountptr;
		}
	}

	mount = malloc(sizeof(*mount));
	mount->directory = string_dup(directory);
	array_init(mount->packs);

	array_append(_mounts, mount);

	return mount;
}

static void _mount_free(struct mount *mount) {
	array_free_each(mount->packs, pack_free);
}

void fs_init(void) {
	array_init(_mounts);
	fs_mount(".", MF_WRITE);
}

void fs_mount(const char *directory, unsigned int flags) {
	struct mount *mount = _get_or_create(directory);
	if(flags & MF_WRITE) {
		_write_mount = mount;
	}
	if(!(flags & MF_NO_PACK)) {
		struct fs_search *search;
		const char *filename;
		struct pack *pack;
		FILE *pack_file;
		const char *pattern;

		pattern = string_from_format("%s/*.pak", directory);
		search = new_fs_search(pattern, 0, SF_SUBDIR | SF_HIDDEN | SF_SYSTEM);
		free((void *)pattern);

		if(!search) {
			return;
		}

		while((filename = fs_search_next(search))) {
			pack_file = fopen(filename, "r");
			if(!pack_file) {
				continue;
			}

			pack = load_pack(filename, pack_file);
			if(!pack) {
				fclose(pack_file);
				continue;
			}

			array_append(mount->packs, pack);
		}

		fs_search_free(search);
	}
}

void fs_umount(const char *directory) {
	struct mount *mount = _get_or_create(directory);
	free((char *)mount->directory);
	free(mount);
}

// ============================================================================

static FILE *_join_fopen(const char *directory, const char *filename, const char *mode) {
	char *full_path = alloca(strlen(directory) + 1 + strlen(filename) + 1);

	*full_path = '\0';
	strcat(full_path, directory);
	strcat(full_path, "/");
	strcat(full_path, filename);

	return fopen(full_path, mode);
}

static unsigned int _FILE_length(FILE *f) {
	int pos, end;

	pos = ftell(f);
	fseek(f, 0, SEEK_END);
	end = ftell(f);
	fseek(f, pos, SEEK_SET);

	return end;
}

static FILE *_mount_open(struct mount *mount, const char *filename, const char *mode) {
	struct pack **packptr;
	FILE *result;
	result = _join_fopen(mount->directory, filename, mode);
	if(result) {
		return result;
	}
	array_for(mount->packs, packptr) {
		if(pack_get_file(*packptr, filename, &result) >= 0) {
			return result;
		}
	}
	return result;
}

FILE *fs_open(const char *filename, const char *mode) {
	struct mount **mountptr;
	FILE *result;

again:
	if((result = _mount_open(_write_mount, filename, mode))) {
		return result;
	}

	if(strchr(mode, 'w') || strchr(mode, 'a')) {
		fs_mkdir(filename);
		goto again;
	}

	array_for_reversed(_mounts, mountptr) {
		if(*mountptr == _write_mount) {
			continue;
		}
		if((result = _mount_open(*mountptr, filename, mode))) {
			return result;
		}
	}

	return NULL;
}

int fs_rename(const char *from, const char *to) {
	return fs_copy(from, to) || fs_remove(from);
}

int fs_copy(const char *from, const char *to) {
	FILE *from_file = fs_open(from, "rb");
	FILE *to_file = fs_open(to, "wb");
	char buffer[1024];
	size_t length;
	do {
		if((length = fread(buffer, 1, sizeof(buffer), from_file))) {
			fwrite(buffer, 1, length, to_file);
		}
	} while(length == sizeof(buffer));
	fclose(from_file);
	fclose(to_file);
	return 0;
}

int fs_remove(const char *filename) {
	int result;
	with_string_format(path, "%s/%s", _write_mount->directory, filename) {
		result = unlink(path);
	}
	return result;
}

int fs_mkdir(const char *path) {
	const char *p = path;
	int result;
	while(1) {
		if((p = strchr(p, '/'))) {
			with_string_format(limited_path, "%.*s", path, p - path) {
				if((result = mkdir(path, 0777))) {
					return result;
				}
			}
		}
	}
	return 0;
}

int fs_exists(const char *path, unsigned int white, unsigned int black) {
}

int fs_read(const char *filename, const unsigned char **dataptr) {
	struct mount **mountptr;
	int result;
	FILE *f;
	struct pack **packptr;
	unsigned char *data;

	array_for_reversed(_mounts, mountptr) {
		struct mount *mount = *mountptr;
		f = _join_fopen(mount->directory, filename, "r");
		if(f) {
			result = _FILE_length(f);
			data = malloc(result + 1);
			fread(data, result, 1, f);
			data[result] = '\0';
			*dataptr = data;
			fclose(f);
			return result;
		}
		array_for(mount->packs, packptr) {
			result = pack_get(*packptr, filename, dataptr);
			if(result >= 0) {
				return result;
			}
		}
	}

	*dataptr = NULL;
	return -1;
}

void fs_write(const char *filename, unsigned int size, const unsigned char *data) {
	FILE *f = fs_open(filename, "w");
	if(f) {
		fwrite(data, size, 1, f);
		fclose(f);
	}
}

// ============================================================================

#define min(X, Y) ({typeof(X) x = X; typeof(Y) y = Y; x < y ? x : y;})
#define max(X, Y) ({typeof(X) x = X; typeof(Y) y = Y; x > y ? x : y;})

#if __linux__
static int glob_match(const char *pattern, const char *text);
static int glob_match_after_star(const char *pattern, const char *text) {
	const char *p = pattern, *t = text;
	char c, c1;
	while((c = *p++) == '?' || c == '*')
		if(c == '?' && *t++ == '\0')
			return 0;
	if(c == '\0')
		return 1;
	if(c == '\\')
		c1 = *p;
	else
		c1 = c;
	while(1) {
		if ((c == '[' || *t == c1) && glob_match(p - 1, t))
			return 1;
		if (*t++ == '\0')
			return 0;
	}
}
static int glob_match(const char *pattern, const char *text) {
	const char *p = pattern, *t = text;
	char c, c1, cstart, cend;
	int invert;

	while((c = *p++) != '\0') {
		switch(c) {
		case '?':
			if(*t == '\0')
				return 0;
			++t;
			break;
		case '\\':
			if (*p++ != *t++)
				return 0;
			break;
		case '*':
			return glob_match_after_star(p, t);
		case '[':
			c1 = *t++;
			if(!c1)
				return 0;
			invert = ((*p == '!') || (*p == '^'));
			if(invert)
				p++;
			c = *p++;
			while(1) {
				cstart = cend = c;
				if(c == '\\') {
					cstart = *p++;
					cend = cstart;
				}
				if(c == '\0')
					return 0;
				c = *p++;
				if(c == '-' && *p != ']') {
					cend = *p++;
					if(cend == '\\')
						cend = *p++;
					if(cend == '\0')
						return 0;
					c = *p++;
				}
				if(c1 >= cstart && c1 <= cend)
					goto match;
				if(c == ']')
					break;
			}
			if(!invert)
				return 0;
			break;
match:
			while (c != ']') {
				if (c == '\0')
					return 0;
				c = *p++;
				if (c == '\0')
					return 0;
				else if (c == '\\')
					++p;
			}
			if(invert)
				return 0;
			break;
		default:
			if(c != *t++)
				return 0;
		}
	}
	return *t == '\0';
}
#endif

struct fs_search *new_fs_search(const char *pattern, unsigned int white, unsigned int black) {
	struct fs_search *result = malloc(sizeof(*result));
#if _WIN32
	result->find = FindFirstFile(pattern, &result->ffd);
	if(result->find == INVALID_HANDLE_VALUE) {
		free(result);
		return NULL;
	}
	result->last_next = TRUE;
#endif
#if __linux__
	size_t filename_length = strlen(min(strrchr(pattern, '/'), strrchr(pattern, '*')));
	result->pattern = string_from_format("%s/%s", _write_mount->directory, pattern);
	result->path = string_sub(result->pattern, 0, strlen(result->pattern) - filename_length);
	result->dir = opendir(result->path);
#endif
	result->white = white;
	result->black = black;
	return result;
}

const char *fs_search_next(struct fs_search *search) {
	const char *result = NULL;
	unsigned int has;
#if _WIN32
	if(search->last_next == FALSE) {
		return NULL;
	}
	do {
		has = 0;
		has |= (search->ffd.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)   ? SF_ARCH   : 0;
		has |= (search->ffd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)	? SF_HIDDEN : 0;
		has |= (search->ffd.dwFileAttributes & FILE_ATTRIBUTE_READONLY)  ? SF_RDONLY : 0;
		has |= (search->ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? SF_SUBDIR : 0;
		has |= (search->ffd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)	? SF_SYSTEM : 0;
		if(!(has & search->black) && (has & search->white) == search->white) {
			result = string_dup(search->ffd.cAlternateFileName);
			break;
		}
		search->last_next = FindNextFile(search->find, &search->ffd);
	} while(search->last_next == TRUE);
#endif
#if __linux__
	struct dirent *d;
	struct stat st;
	while((d = readdir(search->dir)) != NULL) {
		with_string_format(path, "%s/%s", search->path, d->d_name) {
			if(!glob_match(search->pattern, path)) {
				continue;
			}
			if(strcmp(d->d_name, ".") == 0 || strcmp(d->d_name, "..") == 0) {
				continue;
			}
			stat(path, &st);
			has = 0;
			has |= (st.st_mode & S_IFDIR) ? SF_SUBDIR : 0;
			if(!(has & search->black) && (has & search->white) == search->white) {
				result = string_dup(path);
				mem_free(path);
				break;
			}
		}
	}
#endif
	return result;
}

void fs_search_free(struct fs_search *search) {
#if _WIN32
	FindClose(search->find);
#endif
#if __linux__
	mem_free(search->path);
	mem_free(search->pattern);
	closedir(search->dir);
#endif
	mem_free(search);
}
