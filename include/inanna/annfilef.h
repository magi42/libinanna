/***************************************************************************
 *   This file is part of the Inanna library.                              *
 *                                                                         *
 *   Copyright (C) 1997-2002 Marko Grönroos <magi@iki.fi>                  *
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

#ifndef __FILEFORMAT_H__
#define __FILEFORMAT_H__

#include "annetwork.h"

class ANNFileFormat; // Internal

EXCEPTIONCLASS (invalid_filename);



////////////////////////////////////////////////////////////////////////////////////
//   _   |   | |   | ----- o |       -----                          |     o       //
//  / \  |\  | |\  | |       |  ___  |                     ___   |  |       |     //
// /   \ | \ | | \ | |---  | | /   ) |---   __  |/\ |/|/|  ___| -+- |     | |---  //
// |---| |  \| |  \| |     | | |---  |     /  \ |   | | | (   |  |  |     | |   ) //
// |   | |   | |   | |     | |  \__  |     \__/ |   | | |  \__|   \ |____ | |__/  //
////////////////////////////////////////////////////////////////////////////////////

/** ANNetwork data file format library.
 *
 *  Patterns:
 *   - Singleton, Abstract Factory.
 **/
class ANNFileFormatLib {
  public:

	static ANNetwork*	load	(const String& filename) throw (file_not_found, invalid_format, assertion_failed, stream_failure);
	static void			load	(const String& filename, ANNetwork& net) throw (file_not_found, invalid_format, assertion_failed, stream_failure, open_failure);

	/** Loads network from the given stream.
	 *
	 *  @param in Input stream
	 *
	 *  @param filetype File name extension; ".raw",".pat" (SNNS), or ".dt" (Proben1)
	 *
	 *  @throws file_not_found, invalid_format, assertion_failed
	 **/
	static void		load	(TextIStream& in, ANNetwork& net, const String& filetype) throw (stream_failure, invalid_format, assertion_failed);
	
	static void		save	(const String& filename, const ANNetwork& net, const char* fileformat=NULL) throw (invalid_filename, invalid_format, assertion_failed, stream_failure, open_failure);
	static void		save	(TextOStream& out, const ANNetwork& net, const String& filetype) throw (assertion_failed, invalid_filename, stream_failure, invalid_format);

  protected:
	static ANNFileFormat*	create	(const String& fileformat) throw (invalid_format);
};



///////////////////////////////////////////////////////////////////////
//   _   |   | |   | ----- o |       -----                           //
//  / \  |\  | |\  | |       |  ___  |                     ___   |   //
// /   \ | \ | | \ | |---  | | /   ) |---   __  |/\ |/|/|  ___| -+-  //
// |---| |  \| |  \| |     | | |---  |     /  \ |   | | | (   |  |   //
// |   | |   | |   | |     | |  \__  |     \__/ |   | | |  \__|   \  //
///////////////////////////////////////////////////////////////////////

/** Baseclass for @ref ANNetwork file formats.
 *
 *  Patterns:
 *   - Strategy or
 *   - Template Method (subclasses implement stream I/O),
 **/
class ANNFileFormat {
  public:
	virtual			~ANNFileFormat	()	{}

	/** Tries to load the given file to the given pattern set.
	 *
	 *  @throws invalid_format, assertion_failed
	 **/
	virtual void	load	(TextIStream& in, ANNetwork& net) const throw (stream_failure, invalid_format) =0;
	virtual void	save	(TextOStream& out, const ANNetwork& net) const throw (stream_failure) {MUST_OVERLOAD}

  protected:
};

class SNNS_ANNFormat : public ANNFileFormat {
  public:
	virtual void	load	(TextIStream& in, ANNetwork& set) const throw (stream_failure, invalid_format);
	virtual void	save	(TextOStream& out, const ANNetwork& set) const throw (stream_failure);
};


#endif
