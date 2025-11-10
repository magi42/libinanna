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

#ifndef __DATAFORMATS_H__
#define __DATAFORMATS_H__

#include "dataformat.h"


//////////////////////////////////////////////////////////////////////////////////
//  ---- |   | |   |  ---- ___                   -----                          //
// (     |\  | |\  | (     |  \   ___   |   ___  |                     ___   |  //
//  ---  | \ | | \ |  ---  |   |  ___| -+-  ___| |---   __  |/\ |/|/|  ___| -+- //
//     ) |  \| |  \|     ) |   | (   |  |  (   | |     /  \ |   | | | (   |  |  //
// ___/  |   | |   | ___/  |__/   \__|   \  \__| |     \__/ |   | | |  \__|   \ //
//////////////////////////////////////////////////////////////////////////////////

class SNNSDataFormat : public DataFormat {
  public:
	virtual void	load	(TextIStream& in, PatternSet& set) const;
	virtual void	save	(FILE* out, const PatternSet& set) const;
};



///////////////////////////////////////////////////////////////////////////////////////////
// ----                             ___                   -----                          //
// |   )          |      ___    _   |  \   ___   |   ___  |                     ___   |  //
// |---  |/\  __  |---  /   ) |/ \  |   |  ___| -+-  ___| |---   __  |/\ |/|/|  ___| -+- //
// |     |   /  \ |   ) |---  |   | |   | (   |  |  (   | |     /  \ |   | | | (   |  |  //
// |     |   \__/ |__/   \__  |   | |__/   \__|   \  \__| |     \__/ |   | | |  \__|   \ //
///////////////////////////////////////////////////////////////////////////////////////////

class Proben1DataFormat : public DataFormat {
  public:
	virtual void	load	(TextIStream& in, PatternSet& set) const;
};



///////////////////////////////////////////////////////////////////////////////
//  ----               ___                   -----                           //
//  |   )  ___         |  \   ___   |   ___  |                     ___   |   //
//  |---   ___| \    / |   |  ___| -+-  ___| |---   __  |/\ |/|/|  ___| -+-  //
//  | \   (   |  \\//  |   | (   |  |  (   | |     /  \ |   | | | (   |  |   //
//  |  \   \__|   VV   |__/   \__|   \  \__| |     \__/ |   | | |  \__|   \  //
///////////////////////////////////////////////////////////////////////////////

class RawDataFormat : public DataFormat {
  public:
	virtual void	load	(TextIStream& in, PatternSet& set) const;
	virtual void	save	(FILE* out, const PatternSet& set) const;
};



#endif
