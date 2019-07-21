/* $Id$ */

/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file ini.cpp Definition of the IniItem class, related to reading/writing '*.ini' files. */

#include "stdafx.h"
#include "debug.h"
#include "ini_type.h"
#include "string_func.h"
#include "fileio_func.h"
#include "fios.h"

#if (defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 199309L) || (defined(_XOPEN_SOURCE) && _XOPEN_SOURCE >= 500)
# include <unistd.h>
#endif

#ifdef _WIN32
# include <windows.h>
# include <shellapi.h>
# include "core/mem_func.hpp"
#endif

#include "safeguards.h"

/**
 * Create a new ini file with given group names.
 * @param list_group_names A \c nullptr terminated list with group names that should be loaded as lists instead of variables. @see IGT_LIST
 */
IniFile::IniFile(const char * const *list_group_names) : IniLoadFile(list_group_names)
{
}

/**
 * Write the data of the object to the provided file.
 * @param filename Name of the file to write to.
 * @return Whether writing was successful.
 */
bool IniFile::WriteFile(BaseFileWriter &fw, const char *filename)
{
	if (!fw.Open(filename, "w")) return false;

	for (const IniGroup *group = this->group; group != nullptr; group = group->next) {
		/* Write section header. */
		if (group->comment) fw.PutString(group->comment);
		fw.PutByte('[');
		fw.PutString(group->name);
		fw.PutByte(']');
		fw.PutByte('\n');

		for (const IniItem *item = group->item; item != nullptr; item = item->next) {
			if (item->comment != nullptr) fw.PutString(item->comment);

			/* protect item->name with quotes if needed */
			if (strchr(item->name, ' ') != nullptr || item->name[0] == '[') {
				fw.PutByte('"');
				fw.PutString(item->name);
				fw.PutByte('"');
			} else {
				fw.PutString(item->name);
			}

			fw.PutString(" = ");
			if (item->value != nullptr) fw.PutString(item->value);
			fw.PutByte('\n');
		}
	}
	if (this->comment) fw.PutString(this->comment);

	fw.Close(true);
	return fw.Success();
}

/**
 * Save the Ini file's data to the disk.
 * @param filename the file to save to.
 * @return true if saving succeeded.
 */
bool IniFile::SaveToDisk(const char *filename)
{
	/*
	 * First write the configuration to a (temporary) file and then rename
	 * that file. This to prevent that when OpenTTD crashes during the save
	 * you end up with a truncated configuration file.
	 */
	char file_new[MAX_PATH];

	strecpy(file_new, filename, lastof(file_new));
	strecat(file_new, ".new", lastof(file_new));

	FileSystemWriter fsw;
	if (!this->WriteFile(fsw, file_new)) return false;

#if defined(_WIN32)
	/* _tcsncpy = strcpy is TCHAR is char, but isn't when TCHAR is wchar. */
	#undef strncpy
	/* Allocate space for one more \0 character. */
	TCHAR tfilename[MAX_PATH + 1], tfile_new[MAX_PATH + 1];
	_tcsncpy(tfilename, OTTD2FS(filename), MAX_PATH);
	_tcsncpy(tfile_new, OTTD2FS(file_new), MAX_PATH);
	/* SHFileOperation wants a double '\0' terminated string. */
	tfilename[MAX_PATH - 1] = '\0';
	tfile_new[MAX_PATH - 1] = '\0';
	tfilename[_tcslen(tfilename) + 1] = '\0';
	tfile_new[_tcslen(tfile_new) + 1] = '\0';

	/* Rename file without any user confirmation. */
	SHFILEOPSTRUCT shfopt;
	MemSetT(&shfopt, 0);
	shfopt.wFunc  = FO_MOVE;
	shfopt.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_NOERRORUI | FOF_SILENT;
	shfopt.pFrom  = tfile_new;
	shfopt.pTo    = tfilename;
	SHFileOperation(&shfopt);
#else
	if (rename(file_new, filename) < 0) {
		DEBUG(misc, 0, "Renaming %s to %s failed; configuration not saved", file_new, filename);
	}
#endif

	return true;
}

/* virtual */ FILE *IniFile::OpenFile(const char *filename, Subdirectory subdir, size_t *size)
{
	/* Open the text file in binary mode to prevent end-of-line translations
	 * done by ftell() and friends, as defined by K&R. */
	return FioFOpenFile(filename, "rb", subdir, size);
}

/* virtual */ void IniFile::ReportFileError(const char * const pre, const char * const buffer, const char * const post)
{
	ShowInfoF("%s%s%s", pre, buffer, post);
}
