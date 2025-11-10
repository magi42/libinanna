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

#ifndef __INANNA_TOPOLOGY_H__
#define __INANNA_TOPOLOGY_H__

#include "inanna/annetwork.h"

// Predeclarations
class ANNetwork;



///////////////////////////////////////////////////////////////////////////////
//           _   |   | |   | -----                |                          //
//          / \  |\  | |\  |   |         --       |                          //
//         /   \ | \ | | \ |   |    __  |  )  __  |  __   ___  \   |         //
//         |---| |  \| |  \|   |   /  \ |--  /  \ | /  \ (   \  \  |         //
//         |   | |   | |   |   |   \__/ |    \__/ | \__/  ---/   \_/         //
//                                                        __/   \_/          //
///////////////////////////////////////////////////////////////////////////////

/** Baseclass for builder methods for ANNetworks.
 *
 *  Patterns: Builder (builds ANNs), Strategy.
 **/
class ANNTopology {
  public:
	virtual			~ANNTopology ();
	
	virtual void	build	(ANNetwork& network) const=0;
	virtual void	empty	()=0;
  protected:
};



///////////////////////////////////////////////////////////////////////////////
//          _   |   | |   | |                           o                    //
//         / \  |\  | |\  | |      ___         ___          _                //
//        /   \ | \ | | \ | |      ___| \   | /   ) |/\ | |/ \   ___         //
//        |---| |  \| |  \| |     (   |  \  | |---  |   | |   | (   \        //
//        |   | |   | |   | |____  \__|   \_/  \__  |   | |   |  ---/        //
//                                       \_/                     __/         //
///////////////////////////////////////////////////////////////////////////////

/** Older, layered ANN topology. Currently semi-obsolete, but still
 *  used by various parts of Inanna.
 **/
class ANNLayering : public Object {
  public:

				ANNLayering		(const char* description=NULL) {make (description);}
	virtual		~ANNLayering	();
	void		make			(const char* description);
	int			layerIndex		(int layer) const;
	void		getPos			(int i, int& layerno, int& layerpos) const;

	/** Returns the total number of units in the network.
	 **/
	int			totalUnits		() const {
		return layerIndex(mLayers.size()-1)+mLayers[mLayers.size()-1];
	}

	/** Returns the number of layers in the network. */
	int			layers			() const {return mLayers.size();}
	String		toString		() const;

	/** Returns the size of the layer i. Const version.
	 *
	 * If the given layer index is negative, it is substracted from
	 * the number of layers (-1 means the output layer, -2 the layer
	 * below that, etc).
	 **/
	int			layerSize		(int i) const {return mLayers [(i<0)?mLayers.size()+i:i];}

	/** Returns the size of the layer i. Const version.
	 *
	 * If the given layer index is negative, it is substracted from
	 * the number of layers (-1 means the output layer, -2 the layer
	 * below that, etc).
	 **/
	int			operator[]		(int i) const {return mLayers [(i<0)?mLayers.size()+i:i];}

	/** Returns the size of the layer i. Non-const version.
	 *
	 * If the given layer index is negative, it is substracted from
	 * the number of layers (-1 means the output layer, -2 the layer
	 * below that, etc).
	 **/
	int&		operator[]		(int i) {return mLayers [(i<0)?mLayers.size()+i:i];}

	/** Clears the layering; makes the network description empty.
	 **/
	void		empty			() {mLayers.empty();}

	/** Standard copy operator. */
	void		operator=		(const ANNLayering& o) {mLayers=o.mLayers;}

	/** Removes a layer. NOTE: The negative indexing is not allowed
	 *  here.
	 **/
	void		removeLayer		(int i) {mLayers.removeFill(i);}

  protected:
	Array<int>	mLayers;
};



/////////////////////////////////////////////////////////////////////////////////////
// |                                     | -----                |                  //
// |      ___         ___       ___      |   |         --       |                  //
// |      ___| \   | /   ) |/\ /   )  ---|   |    __  |  )  __  |  __   ___  \   | //
// |     (   |  \  | |---  |   |---  (   |   |   /  \ |--  /  \ | /  \ (   \  \  | //
// |____  \__|   \_/  \__  |    \__   ---|   |   \__/ |    \__/ | \__/  ---/   \_/ //
//              \_/                                                     __/   \_/  //
/////////////////////////////////////////////////////////////////////////////////////

/** Multilayer perceptron ANNTopology.
 **/
class LayeredTopology : virtual public ANNTopology, public ANNLayering {
  public:

	/** Standard constructor.
	 *
	 *  See @ref make for info about the parameters.
	 **/
					LayeredTopology		(const char* description=NULL) {make (description);}
	virtual			~LayeredTopology	();

	virtual void	build	(ANNetwork& network) const;

	/** Clears the layering; makes the network description empty.
	 **/
	virtual void	empty			() {ANNLayering::empty();}

  protected:
};


/***************************************************************************
 *  @file		topology.h
 *  @brief		Network topology handling.
 ***************************************************************************/

#endif
