/* $Id$ */

/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file scenario_file.cpp
 * This file contains functions for loading and saving scenario tar files.
 */

#include "stdafx.h"
#include "landscape.h"
#include "map_func.h"
#include "settings_type.h"
#include "ini_type.h"
#include "tar_type.h"
#include "fios.h"
#include "screenshot.h"
#include "string_func.h"
#include "strings_func.h"
#include "town.h"
#include "signs_base.h"
#include "tunnelbridge_map.h"

#include "safeguards.h"

static const char *HEIGHT_FILENAME     = "height";          ///< Filename (without extension) of the height image file.
//static const char *WATER_FILENAME  = "water";  ///< Filename of the water image file inside the scenario tar.

static const char *METADATA_FILENAME   = "metadata.txt";    ///< Filename of the meta data INI file.
static const char *TOWN_FILENAME       = "town_data.txt";   ///< Filename of the town layout file.
static const char *ROADBRIDGE_FILENAME = "road_bridge.txt"; ///< Filename of the road bridge layout file.
static const char *ROADTUNNEL_FILENAME = "road_tunnel.txt"; ///< Filename of the road tunnel layout file.
static const char *SIGN_FILENAME       = "sign.txt";        ///< Filename of the sign file.

/** Name of the climates. */
static const char *CLIMATE_NAMES[] = {
	"temperate", // LT_TEMPERATE
	"arctic",    // LT_ARCTIC
	"tropical",  // LT_TROPIC
	"toyland",   // LT_TOYLAND
};

assert_compile(lengthof(CLIMATE_NAMES) == NUM_LANDSCAPE);

/* Names of town layout. */
static const char *TOWN_LAYOUT_NAMES[] = {
	"original",     // TL_ORIGINAL
	"better-roads", // TL_BETTER_ROADS
	"2x2-grid",     // TL_2X2_GRID
	"3x3-grid",     // TL_3X3_GRID
	"random",       // TL_RANDOM
};

assert_compile(lengthof(TOWN_LAYOUT_NAMES) == NUM_TLS);


/**
 * Write the numeric \a value as item value.
 * @param item The INI item to assign.
 * @param value Numeric value that should be used.
 */
static void SetNumericItemValue(IniItem *item, uint value)
{
	char buffer[16];
	seprintf(buffer, lastof(buffer), "%u", value);
	item->SetValue(buffer);
}

/**
 * Construct a new section in the meta data file, with \c width, \c height, and \c file items.
 * @param meta_file INI file to write into.
 * @param section_name Name of the new section.
 * @param filename Value of the \c file item.
 */
static void MakeWidthHeightFileGroup(IniFile *meta_file, const char *section_name, const char *filename)
{
	IniGroup *group = new IniGroup(meta_file, section_name);

	IniItem *item = new IniItem(group, "width");
	SetNumericItemValue(item, MapSizeX());

	item = new IniItem(group, "height");
	SetNumericItemValue(item, MapSizeY());

	item = new IniItem(group, "file");
	item->SetValue(filename);
}

/**
 * Save basic information about the map.
 * @param meta_file INI file to write into.
 */
static bool ConstructBasicInformation(IniFile &meta_file)
{
	/* [extended_heightmap] group. */
	IniGroup *heightmap_group = new IniGroup(&meta_file, "extended_heightmap");

	IniItem *item = new IniItem(heightmap_group, "format_version");
	SetNumericItemValue(item, 1);

	item = new IniItem(heightmap_group, "width");
	SetNumericItemValue(item, MapSizeX());

	item = new IniItem(heightmap_group, "height");
	SetNumericItemValue(item, MapSizeY());

	item = new IniItem(heightmap_group, "orientation");
	item->SetValue("ccw");

	item = new IniItem(heightmap_group, "climate");
	item->SetValue(CLIMATE_NAMES[_settings_game.game_creation.landscape]);

	return true;
}

/**
 * Save the height map in the scenario tar file.
 * @param meta_file INI meta file to write into.
 * @param tar_stream File output stream into the tar file.
 * @return Whether writing the height layer was successful.
 */
static bool ConstructHeightLayer(IniFile &meta_file, WriteTar &tar_stream)
{
	/* [height_layer] */
	const char *height_filename = WriteHeightmapInTar(tar_stream, HEIGHT_FILENAME);
	if (height_filename == nullptr) return false;

	IniGroup *height_group = new IniGroup(&meta_file, "height_layer");

	IniItem *item = new IniItem(height_group, "filename");
	item->SetValue(height_filename);

	item = new IniItem(height_group, "max_height");
	SetNumericItemValue(item, _settings_game.construction.max_heightlevel);

	item = new IniItem(height_group, "snowline_height");
	SetNumericItemValue(item, _settings_game.game_creation.snow_line_height);
	return true;
}

static bool ConstructTerrainLayer(IniFile &meta_file, WriteTar &tar_stream)
{
	// [terrain_layer]
	// filename=terrain.png
	// Default: Default terrain.
	// Rough: Rough terrain.
	// Rock: Rocky terrain.
	// Trees
	return true;
}

static bool ConstructClimateLayer(IniFile &meta_file, WriteTar &tar_stream)
{
	// [climate_layer]
	// file = climate.png
	// Default: Default terrain.
	// Desert: Desert terrain.
	// Rainforest: Rainforest terrain.
	return true;
}

/**
 * Write the roads of the scenario.
 */
static bool ConstructRoadMapImageLayers(IniFile &meta_file, WriteTar &tar_stream)
{
	// One image for each bit??
	// Needs handling of roadtypes as well!!
	return true;
}

static bool ConstructWaterLayer(IniFile &meta_file, WriteTar &tar_stream)
{
	// plain water
	// canals
	// locks
	// aqueducts

//	/* [water_layer] contains rivers, locks, and canals. */
//	const char *water_filename = WriteWaterInTar(&fw, WATER_FILENAME);
//	if (water_filename != nullptr) {
//		IniGroup *water_group = new IniGroup(&meta_file, "water_layer");
//		item = new IniItem(water_group, "filename");
//		item->SetValue(water_filename);
//	}
	return true;
}

/**
 * Save the town information
 * @param meta_file parent meta file.
 * @param tar_stream File output stream into the tar file.
 * @return Whether writing the town file information was successful.
 */
static bool ConstructTownLayer(IniFile &meta_file, WriteTar &tar_stream)
{
	IniFile town_file;

	Town *t;
	FOR_ALL_TOWNS(t) {
		IniGroup *town_group = new IniGroup(&town_file, "town");

		char buffer[256];
		SetDParam(0, t->index);
		GetString(buffer, STR_TOWN_NAME, lastof(buffer));
		IniItem *item = new IniItem(town_group, "name");
		item->SetValue(buffer);

		item = new IniItem(town_group, "posx");
		SetNumericItemValue(item, TileX(t->xy));

		item = new IniItem(town_group, "posy");
		SetNumericItemValue(item, TileY(t->xy));

		item = new IniItem(town_group, "buildings");
		SetNumericItemValue(item, t->cache.num_houses);

		item = new IniItem(town_group, "city");
		item->SetValue(t->larger_town ? "true" : "false");

		item = new IniItem(town_group, "layout");
		item->SetValue(TOWN_LAYOUT_NAMES[t->layout]);
	}
	if (town_file.IsEmpty()) return true;

	/* [town_file] group. */
	MakeWidthHeightFileGroup(&meta_file, "town_file", TOWN_FILENAME);

	TarFileWriter writer(tar_stream);
	town_file.WriteFile(writer, TOWN_FILENAME);
	return writer.Success();
}

/**
 * Save the road bridge information
 * @param meta_file parent meta file.
 * @param tar_stream File output stream into the tar file.
 * @return Whether writing the road bridge information was successful.
 */
static bool ConstructRoadBridgeLayer(IniFile &meta_file, WriteTar &tar_stream)
{
	IniFile roadbridge_file;

	for (TileIndex tile = 0; tile != MapSize(); tile++) {
		if (IsBridgeTile(tile) && GetTunnelBridgeTransportType(tile) == TRANSPORT_ROAD) {
			const char *direction = nullptr;
			uint length = 0;
			TileIndex tile2 = GetOtherBridgeEnd(tile);
			if (TileX(tile) < TileX(tile2)) {
				direction = "SW";
				length = TileX(tile2) - TileX(tile);
			} else if (TileY(tile) < TileY(tile2)) {
				direction = "SE";
				length = TileY(tile2) - TileY(tile);
			} else {
				continue;
			}

			IniGroup *roadbridge_group = new IniGroup(&roadbridge_file, "bridge");

			IniItem *item = new IniItem(roadbridge_group, "posx");
			SetNumericItemValue(item, TileX(tile));

			item = new IniItem(roadbridge_group, "posy");
			SetNumericItemValue(item, TileY(tile));

			item = new IniItem(roadbridge_group, "direction");
			item->SetValue(direction);

			item = new IniItem(roadbridge_group, "length");
			SetNumericItemValue(item, length);

			const BridgeSpec *bridge_spec = GetBridgeSpec(GetBridgeType(tile));
			item = new IniItem(roadbridge_group, "max_speed");
			SetNumericItemValue(item, bridge_spec->speed);
		}
	}
	if (roadbridge_file.IsEmpty()) return true;

	/* [road_bridge_file] group. */
	MakeWidthHeightFileGroup(&meta_file, "road_bridge_file", ROADBRIDGE_FILENAME);

	TarFileWriter writer(tar_stream);
	roadbridge_file.WriteFile(writer, ROADBRIDGE_FILENAME);
	return writer.Success();
}

/**
 * Save road tunnels as file into the scenario tar file.
 * @param meta_file parent meta file.
 * @param tar_stream File output stream into the tar file.
 * @return Whether writing the road tunnel information was successful.
 */
static bool ConstructRoadTunnelLayer(IniFile &meta_file, WriteTar &tar_stream)
{
	IniFile roadtunnel_file;

	for (TileIndex tile = 0; tile != MapSize(); tile++) {
		if (IsTunnelTile(tile) && GetTunnelBridgeTransportType(tile) == TRANSPORT_ROAD) {
			TileIndex tile2 = GetOtherTunnelEnd(tile);
			if (TileX(tile) < TileX(tile2) || TileY(tile) < TileY(tile2)) {
				IniGroup *roadtunnel_group = new IniGroup(&roadtunnel_file, "tunnel");

				IniItem *item = new IniItem(roadtunnel_group, "posx");
				SetNumericItemValue(item, TileX(tile));

				item = new IniItem(roadtunnel_group, "posy");
				SetNumericItemValue(item, TileY(tile));
			}
		}
	}
	if (roadtunnel_file.IsEmpty()) return true;

	/* [road_tunnel_file] group. */
	MakeWidthHeightFileGroup(&meta_file, "road_tunnel_file", ROADTUNNEL_FILENAME);

	TarFileWriter writer(tar_stream);
	roadtunnel_file.WriteFile(writer, ROADTUNNEL_FILENAME);
	return writer.Success();
}

/**
 * Save the signs of the scenario.
 * @param meta_file parent meta file.
 * @param tar_stream File output stream into the tar file.
 * @return Whether writing the sign information was successful.
 */
static bool ConstructSignLayer(IniFile &meta_file, WriteTar &tar_stream)
{
	IniFile sign_file;

	Sign *s;
	FOR_ALL_SIGNS(s) {
		IniGroup *sign_group = new IniGroup(&sign_file, "sign");

		IniItem *item = new IniItem(sign_group, "posx");
		SetNumericItemValue(item, s->x);

		item = new IniItem(sign_group, "posy");
		SetNumericItemValue(item, s->y);

		item = new IniItem(sign_group, "text");
		item->SetValue(s->name);
	}
	if (sign_file.IsEmpty()) return true;

	/* [sign_file] group. */
	IniGroup *sign_group = new IniGroup(&meta_file, "sign_file");
	IniItem *item = new IniItem(sign_group, "file");
	item->SetValue(SIGN_FILENAME);

	TarFileWriter writer(tar_stream);
	sign_file.WriteFile(writer, SIGN_FILENAME);
	return writer.Success();
}

/**
 * Construct a meta data INI file, and write it to the tar file.
 * @param tar_writer Tarfile storage destination.
 * @return Whether generation of the meta data file was successful.
 */
static bool SaveMetaData(WriteTar &tar_stream)
{
	IniFile meta_file;
	bool success = true;

	success &= ConstructBasicInformation(meta_file);
	success &= ConstructHeightLayer(meta_file, tar_stream);
	success &= ConstructTerrainLayer(meta_file, tar_stream);
	success &= ConstructClimateLayer(meta_file, tar_stream);
	success &= ConstructRoadMapImageLayers(meta_file, tar_stream);
	success &= ConstructWaterLayer(meta_file, tar_stream);
	success &= ConstructTownLayer(meta_file, tar_stream);
	success &= ConstructRoadBridgeLayer(meta_file, tar_stream);
	success &= ConstructRoadTunnelLayer(meta_file, tar_stream);
	success &= ConstructSignLayer(meta_file, tar_stream);

	/* Write the metadata file. */
	TarFileWriter writer(tar_stream);
	meta_file.WriteFile(writer, METADATA_FILENAME);
	success &= writer.Success();

	return success;
}

/**
 * Save the current map as a scenario tar file.
 * @param pathname Path of the file to save to.
 * @return Whether saving the data was successful.
 */
bool SaveScenarioTarfile(const char *pathname)
{
	WriteTar tar_stream;
	if (!tar_stream.StartWriteTar(pathname, "scenario")) return false;
	bool b = SaveMetaData(tar_stream);
	b &= tar_stream.StopWriteTar();
	return b;
}
