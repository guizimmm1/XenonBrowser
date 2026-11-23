/*
 * POSIX odds and ends newlib/xsdk do not implement, plus regex stubs.
 * Everything here is only as correct as NetSurf needs it to be.
 *
 * This file is part of NetSurf, http://www.netsurf-browser.org/
 * Licensed under the GNU General Public License, version 2.
 */

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <regex.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

int fstatat(int fd, const char *path, struct stat *buf, int flags)
{
	(void)fd;
	(void)flags;
	return stat(path, buf);
}

int regcomp(regex_t *preg, const char *regex, int cflags)
{
	(void)preg;
	(void)regex;
	(void)cflags;
	return REG_BADPAT;
}

int regexec(const regex_t *preg, const char *string, size_t nmatch,
	    regmatch_t pmatch[], int eflags)
{
	(void)preg;
	(void)string;
	(void)nmatch;
	(void)pmatch;
	(void)eflags;
	return REG_NOMATCH;
}

size_t regerror(int errcode, const regex_t *preg, char *errbuf, size_t errbuf_size)
{
	(void)errcode;
	(void)preg;
	if (errbuf != NULL && errbuf_size > 0)
		errbuf[0] = '\0';
	return 0;
}

void regfree(regex_t *preg)
{
	(void)preg;
}

int scandir(const char *dirp, struct dirent ***namelist,
	    int (*filter)(const struct dirent *),
	    int (*compar)(const struct dirent **, const struct dirent **))
{
	DIR *dir;
	struct dirent *entry;
	struct dirent **list;
	size_t count = 0;
	size_t capacity = 16;

	dir = opendir(dirp);
	if (dir == NULL)
		return -1;

	list = malloc(capacity * sizeof(*list));
	if (list == NULL) {
		closedir(dir);
		errno = ENOMEM;
		return -1;
	}

	while ((entry = readdir(dir)) != NULL) {
		struct dirent *copy;
		if (filter != NULL && filter(entry) == 0)
			continue;
		copy = malloc(sizeof(*copy));
		if (copy == NULL)
			break;
		memcpy(copy, entry, sizeof(*copy));
		if (count == capacity) {
			struct dirent **grown;
			capacity *= 2;
			grown = realloc(list, capacity * sizeof(*list));
			if (grown == NULL) {
				free(copy);
				break;
			}
			list = grown;
		}
		list[count++] = copy;
	}
	closedir(dir);

	if (compar != NULL && count > 1) {
		qsort(list, count, sizeof(*list),
		      (int (*)(const void *, const void *))compar);
	}
	*namelist = list;
	return (int)count;
}
