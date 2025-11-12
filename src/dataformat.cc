/***************************************************************************
 *   This file is part of the Inanna library.                              *
 *                                                                         *
 *   Copyright (C) 1997-2002 Marko Gr�nroos <magi@iki.fi>                  *
 *                                                                         *
 ***************************************************************************
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Library General Public            *
 *  License as published by the Free Software Foundation; either           *
 *  version 2 of the License, or (at your option) any later version.       *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Library General Public License for more details.                       *
 *                                                                         *
 *  You should have received a copy of the GNU Library General Public      *
 *  License along with this library; see the file COPYING.LIB.  If         *
 *  not, write to the Free Software Foundation, Inc., 59 Temple Place      *
 *  - Suite 330, Boston, MA 02111-1307, USA.                               *
 *                                                                         *
 ***************************************************************************/

#include <fstream>
#include <magic/mobject.h>
#include <magic/mtextstream.h>
#include "inanna/dataformat.h"
#include "inanna/dataformats.h"



//////////////////////////////////////////////////////////////////////////////
//    ___                   -----                          |     o          //
//    |  \   ___   |   ___  |                     ___   |  |       |        //
//    |   |  ___| -+-  ___| |---   __  |/\ |/|/|  ___| -+- |     | |---     //
//    |   | (   |  |  (   | |     /  \ |   | | | (   |  |  |     | |   )    //
//    |__/   \__|   \  \__| |     \__/ |   | | |  \__|   \ |____ | |__/     //
//////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
 * Tries to load the given file to the given pattern set.
 *
 * @throws file_not_found, invalid_format, assertion_failed
 ******************************************************************************/
void DataFormatLib::load (const String& filename, PatternSet& set)
{
	ASSERTWITH (!isempty(filename), "Filename required (was empty)");

	// Open the file
	TextIStream* in = &stin;
	if (filename != "-" && !filename.isEmpty()) {
		try {
			in = new TextIStream (new File (filename));
		} catch (Exception e) {
			throw file_not_found (e.what());
		}

		/* The success of creation should be checked.
		if (!*in)
			throw file_not_found (format ("Pattern set file '%s' not found", (CONSTR) filename));
		*/
	}

	load (*in, set, filename);

	if (in != &stin)
		delete in;
}

/*******************************************************************************
 * Loads pattern set from the given stream.
 *
 *  @throws file_not_found, invalid_format, assertion_failed
 ******************************************************************************/
void DataFormatLib::load (
	TextIStream&  in,              /**< Input stream                          */
	PatternSet&   set,             /**< Pattern set to load.                  */
	const String& filetype         /**< File name extension; ".raw",".pat" (SNNS), or ".dt" (Proben1) */)
{
	DataFormat* handler = create (filetype);
	try {
		handler->load (in, set);
	} catch (...) {
		delete handler;
		throw; // Rethrow
	}
	delete handler;
}

void DataFormatLib::save (const String& filename, const PatternSet& set) {
	FILE* out=stdout;
	if (filename!="-" && !filename.isEmpty())
		out = fopen (filename, "w");

	ASSERTWITH (out, format ("Pattern save file '%s' couldn't be opened",
							 (CONSTR) filename));

	DataFormat* handler = create (filename);

	try {
		handler->save (out, set);
	} catch (must_overload e) {
		delete handler;
		throw invalid_filename (format ("Save-to-file operation not supported for file type\n%s", (CONSTR) e.what()));
	}
	delete handler;

	if (out != stdout)
		fclose (out);
}

DataFormat* DataFormatLib::create (const String& filename) {
	// Parse the contents according to the file name extension
	if (filename.length()>4 && filename.right(4)==".pat")
		return new SNNSDataFormat ();
	else if (filename.length()>3 && filename.right(3)==".dt")
		return new Proben1DataFormat ();
	else
		return new RawDataFormat ();
}
