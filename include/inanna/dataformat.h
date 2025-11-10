/***************************************************************************
    copyright            : (C) 2000 by Marko Grönroos
    email                : magi@iki.fi
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __DATAFORMAT_H__
#define __DATAFORMAT_H__

#include "patternset.h"

class DataFormat; // Internal

EXCEPTIONCLASS (invalid_filename);

//////////////////////////////////////////////////////////////////////////////
//    ___                   -----                          |     o          //
//    |  \   ___   |   ___  |                     ___   |  |       |        //
//    |   |  ___| -+-  ___| |---   __  |/\ |/|/|  ___| -+- |     | |---     //
//    |   | (   |  |  (   | |     /  \ |   | | | (   |  |  |     | |   )    //
//    |__/   \__|   \  \__| |     \__/ |   | | |  \__|   \ |____ | |__/     //
//////////////////////////////////////////////////////////////////////////////

/** PatternSet data file format library.
 *
 *  Patterns:
 *   - Singleton, Abstract Factory.
 **/
class DataFormatLib {
  public:
	static void		load	(const String& filename, PatternSet& set) throw (file_not_found, invalid_format, assertion_failed);
	static void		load	(TextIStream& in, PatternSet& set, const String& filetype=".raw") throw (invalid_format, assertion_failed);
	
	/** Tries to load the given file to the given pattern set.
	 *
	 *  @throws file_not_found, invalid_format, assertion_failed
	 **/
	static void		save	(const String& filename, const PatternSet& set) throw (invalid_filename);

  protected:
	/** Factory creates a data format handler according to
	 *  filename. To make the factory more extensible, there would
	 *  need to be some sort of dynamic registry of Factory Methods.
	 **/
	static DataFormat*	create		(const String& filename);
};

//////////////////////////////////////////////////////////////////////////////
//           ___                   -----                                    //
//           |  \   ___   |   ___  |                     ___   |            //
//           |   |  ___| -+-  ___| |---   __  |/\ |/|/|  ___| -+-           //
//           |   | (   |  |  (   | |     /  \ |   | | | (   |  |            //
//           |__/   \__|   \  \__| |     \__/ |   | | |  \__|   \           //
//////////////////////////////////////////////////////////////////////////////

/** Baseclass for @ref PatternSet file formats.
 *
 *  Patterns:
 *   - Strategy or
 *   - Template Method (subclasses implement stream I/O),
 **/
class DataFormat {
  public:
	virtual ~DataFormat	() {}
	/** Tries to load the given file to the given pattern set.
	 *
	 *  @throws invalid_format, assertion_failed
	 **/
	virtual void	load	(TextIStream& in, PatternSet& set) const=0;
	virtual void	save	(FILE* out, const PatternSet& set) const {MUST_OVERLOAD}

  protected:
};



#endif
