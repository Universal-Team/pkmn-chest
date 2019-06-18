/*
 * savegame_manager: a tool to backup and restore savegames from Nintendo
 *  DS cartridges. Nintendo DS and all derivative names are trademarks
 *  by Nintendo. EZFlash 3-in-1 is a trademark by EZFlash.
 *
 * globals.h: global varibles, defines etc.
 *
 * Copyright (C) Pokedoc (2011)
 */
/* 
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2 of the License, or 
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License 
 * for more details.
 * 
 * You should have received a copy of the GNU General Public License along 
 * with this program; if not, write to the Free Software Foundation, Inc., 
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef GLOBALS_H
#define GLOBALS_H

#include <nds.h>
#include "auxspi.h"

extern uint8 data[0x8000];

extern int ir_delay;

// all libfat access will be using this device. default value = "/", i.e. "default" DLDI device
extern char device[16];

// text buffer for composing various messages
extern char txt[256];

// this should be enough for the forseeable future
#define EXTRA_ARRAY_SIZE 16

extern u32 extra_id[EXTRA_ARRAY_SIZE];
extern u8 extra_size[EXTRA_ARRAY_SIZE];

#endif // GLOBALS_H