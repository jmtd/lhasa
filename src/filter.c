/*

Copyright (c) 2012, Simon Howard

Permission to use, copy, modify, and/or distribute this software
for any purpose with or without fee is hereby granted, provided
that the above copyright notice and this permission notice appear
in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

 */

#include <string.h>

#include "filter.h"

void lha_filter_init(LHAFilter *filter, LHAReader *reader,
                     char **filters, unsigned int num_filters)
{
	filter->reader = reader;
	filter->filters = filters;
	filter->num_filters = num_filters;
}

static int match_glob(char *glob, char *str)
{
	// Iterate through the string, matching each character against the
	// equivalent character from the glob.

	while (*str != '\0') {

		// When we reach a '*', cut off the remainder of the glob
		// and shift forward through the string trying to find
		// a point that matches it.

		if (*glob == '*') {
			if (match_glob(glob + 1, str)) {
				return 1;
			}
		} else if (*glob == '?' || *glob == *str) {
			++glob;
		} else {
			return 0;
		}

		++str;
	}

	// We have reached the end of the string to match against.
	// Any '*'s left at the end of the string are superfluous and
	// can be ignored.

	while (*glob == '*') {
		++glob;
	}

	// We now have a successful match only if we have simultaneously
	// matched the end of the glob.

	return *glob == '\0';
}

static int matches_filter(LHAFilter *filter, LHAFileHeader *header)
{
	char *path;
	unsigned int i;

	// Special case: no filters means match all.

	if (filter->num_filters == 0) {
		return 1;
	}

	if (header->path != NULL) {
		path = malloc(strlen(header->path)
		              + strlen(header->filename) + 1);
		strcpy(path, header->path);
		strcat(path, header->filename);
	} else {
		path = header->filename;
	}

	// Check this path with the list of filters. If one matches,
	// we must return true.

	for (i = 0; i < filter->num_filters; ++i) {
		if (match_glob(filter->filters[i], path)) {
			break;
		}
	}

	if (header->path != NULL) {
		free(path);
	}

	return i < filter->num_filters;
}

LHAFileHeader *lha_filter_next_file(LHAFilter *filter)
{
	LHAFileHeader *header;

	// Read through headers until we find one that matches.

	do {
		header = lha_reader_next_file(filter->reader);
	} while (header != NULL && !matches_filter(filter, header));

	return header;
}
