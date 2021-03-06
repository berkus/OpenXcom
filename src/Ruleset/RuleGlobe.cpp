/*
 * Copyright 2010-2014 OpenXcom Developers.
 *
 * This file is part of OpenXcom.
 *
 * OpenXcom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenXcom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */
#define _USE_MATH_DEFINES
#include "RuleGlobe.h"
#include <SDL_endian.h>
#include <cmath>
#include <fstream>
#include "../Engine/Exception.h"
#include "Polygon.h"
#include "Polyline.h"
#include "../Engine/CrossPlatform.h"

namespace OpenXcom
{

/**
 * Creates a blank ruleset for globe contents.
 */
RuleGlobe::RuleGlobe() : _polygons(), _polylines()
{
}

/**
 *
 */
RuleGlobe::~RuleGlobe()
{
	for (std::list<Polygon*>::iterator i = _polygons.begin(); i != _polygons.end(); ++i)
	{
		delete *i;
	}
	for (std::list<Polyline*>::iterator i = _polylines.begin(); i != _polylines.end(); ++i)
	{
		delete *i;
	}
}

/**
 * Loads the globe from a YAML file.
 * @param node YAML node.
 */
void RuleGlobe::load(const YAML::Node &node)
{
	if (node["data"])
	{
		loadDat(CrossPlatform::getDataFile(node["data"].as<std::string>()));
	}
	for (YAML::const_iterator i = node["polygons"].begin(); i != node["polygons"].end(); ++i)
	{
		Polygon *polygon = new Polygon(3);
		polygon->load(*i);
		_polygons.push_back(polygon);
	}
	for (YAML::const_iterator i = node["polylines"].begin(); i != node["polylines"].end(); ++i)
	{
		Polyline *polyline = new Polyline(3);
		polyline->load(*i);
		_polylines.push_back(polyline);
	}
}

/**
 * Returns the list of polygons in the globe.
 * @return Pointer to the list of polygons.
 */
std::list<Polygon*> *RuleGlobe::getPolygons()
{
	return &_polygons;
}

/**
 * Returns the list of polylines in the globe.
 * @return Pointer to the list of polylines.
 */
std::list<Polyline*> *RuleGlobe::getPolylines()
{
	return &_polylines;
}

/**
 * Loads a series of map polar coordinates in X-Com format,
 * converts them and stores them in a set of polygons.
 * @param filename Filename of the DAT file.
 * @sa http://www.ufopaedia.org/index.php?title=WORLD.DAT
 */
void RuleGlobe::loadDat(const std::string &filename)
{
	// Load file
	std::ifstream mapFile (filename.c_str(), std::ios::in | std::ios::binary);
	if (!mapFile)
	{
		throw Exception(filename + " not found");
	}

	short value[10];

	while (mapFile.read((char*)&value, sizeof(value)))
	{
		Polygon* poly;
		int points;
		
		for (int i = 0; i < 10; ++i)
		{
			value[i] = SDL_SwapLE16(value[i]);
		}

		if (value[6] != -1)
		{
			points = 4;
		}
		else
		{
			points = 3;
		}
		poly = new Polygon(points);

		for (int i = 0, j = 0; i < points; ++i)
		{
			// Correct X-Com degrees and convert to radians
			double lonRad = value[j++] * 0.125f * M_PI / 180;
			double latRad = value[j++] * 0.125f * M_PI / 180;

			poly->setLongitude(i, lonRad);
			poly->setLatitude(i, latRad);
		}
		poly->setTexture(value[8]);

		_polygons.push_back(poly);
	}

	if (!mapFile.eof())
	{
		throw Exception("Invalid globe map");
	}

	mapFile.close();
}

}
