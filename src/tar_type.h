/* $Id$ */

/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file tar_type.h Structs, typedefs and macros used for TAR file handling. */

#ifndef TAR_TYPE_H
#define TAR_TYPE_H

#include <map>
#include <string>

#include "fileio_type.h"

/** The define of a TarList. */
struct TarListEntry {
	const char *filename; ///< Path of the tarfile.
	const char *dirname;  ///< Directory in the tarfile, or \c NULL.

	/* MSVC goes copying around this struct after initialisation, so it tries
	 * to free filename, which isn't set at that moment... but because it
	 * initializes the variable with garbage, it's going to segfault. */
	TarListEntry() : filename(nullptr), dirname(nullptr) {}
	~TarListEntry() { free(this->filename); free(this->dirname); }
};

/** Meta-data of a file in a tarfile. */
struct TarFileListEntry {
	const char *tar_filename; ///< Filename of the tarfile (not owned by this struct).
	size_t size;              ///< Size of the file.
	size_t position;          ///< Offset in the tarfile where this file begins.
};

typedef std::map<std::string, TarListEntry> TarList; ///< Collection of found tarfiles, ordered by name.
typedef std::map<std::string, TarFileListEntry> TarFileList; ///< Collection of found files in tarfiles, ordered by name in its tarfile.
extern TarList _tar_list[NUM_SUBDIRS]; ///< Collection of found tarfiles, for each OpenTTD subdirectory.
extern TarFileList _tar_filelist[NUM_SUBDIRS]; ///< Collection of found files in tarfiles, for each OpenTTD subdirectory.

/**
 * Macro to iterate over all files in tarfiles of an OpenTTD subdirectory.
 * @param tar Iterator variable iterating over #TarFileList.
 * @param sd OpenTTD subdirectory to search.
 */
#define FOR_ALL_TARS(tar, sd) for (tar = _tar_filelist[sd].begin(); tar != _tar_filelist[sd].end(); tar++)

#endif /* TAR_TYPE_H */
