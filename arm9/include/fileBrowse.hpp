/*-----------------------------------------------------------------
 Copyright (C) 2005 - 2017
	Michael "Chishm" Chisholm
	Dave "WinterMute" Murphy

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

------------------------------------------------------------------*/

#ifndef FILE_BROWSE_HPP
#define FILE_BROWSE_HPP
#include <string>
#include <vector>

struct DirEntry {
	std::string name;
	bool isDirectory;
};

/*
 * Gets the contents of the current directory
 * std::vector<DirEntry>& dirContents is where the contents will be stored
 * const std::vector<std::string> extensionList is the extensions to include
 */
void getDirectoryContents(std::vector<DirEntry> &dirContents, const std::vector<std::string> &extensionList);

/*
 * Browse for a file
 * const std::vector<std::string>& extensionList is the extensions to show
 * bool directoryNavigation is whether to allow changing folders
 * bool canChooseDirs is whether to allow selecting directories as the path (default: false)
 * Returns the selected file
 */
std::string browseForFile(const std::vector<std::string> &extensionList, bool directoryNavigation, bool canChooseDirs = false);

/*
 * Browse for a save file
 * Returns the selected file
 */
std::string browseForSave(void);


/*
 * Copies a file to another location
 * const char *sourcePath is the path to the source file
 * const char *destinationPath is the path to copy to
 * returns 1 if successful, -2 if a directory was given, or -1 for an error while copying
 */
int fcopy(const char *sourcePath, const char *destinationPath);

#endif //FILE_BROWSE_HPP