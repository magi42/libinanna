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

#include "inanna/annetwork.h"

ANNTopology::~ANNTopology () {
}



///////////////////////////////////////////////////////////////////////////////
//          _   |   | |   | |                           o                    //
//         / \  |\  | |\  | |      ___         ___          _                //
//        /   \ | \ | | \ | |      ___| \   | /   ) |/\ | |/ \   ___         //
//        |---| |  \| |  \| |     (   |  \  | |---  |   | |   | (   \        //
//        |   | |   | |   | |____  \__|   \_/  \__  |   | |   |  ---/        //
//                                       \_/                     __/         //
///////////////////////////////////////////////////////////////////////////////

/** Standard constructor.
 *
 *  See @ref make for info about the parameters.
 **/
ANNLayering::~ANNLayering () {
}

/** Sets the layer structure according to the given description string.
 *
 *  @param description Textual layer description. Consists of
 *  layer sizes separated by '-'-marks. For example: "2-10-10-1".
 *  NOTE: The layer sizes may currently NOT have additional
 *  qualifiers that are not numbers, for example:
 *  "S2-10-10-L10". These qualifiers are usually intended for the
 *  @ref AnyNetwork inheritor, and should ignored here, but, as
 *  said, are currently not.
 **/
void ANNLayering::make (const char* description) {
	if (!description)
		return;
	String desc = description;
	
	Array<String> sizes_tmp;
	desc.split (sizes_tmp, '-');
	ASSERTWITH (sizes_tmp.size()>=1, "Layering description must have at least 1 layer");

	// Move the string array to self
	mLayers.make (sizes_tmp.size());
	for (int i=0; i<sizes_tmp.size(); i++)
		mLayers[i] = sizes_tmp[i].toInt();
}

/** Calculates the absolute neuron index offset for the given
 *  layer index; index of the first neuron on that layer. If the
 *  given layer index is negative, it is substracted from the
 *  number of layers (-1 means the output layer, -2 the layer
 *  below that, etc).
 **/
int ANNLayering::layerIndex (int layer) const {
	if (layer<0)
		layer = mLayers.size()+layer;

	ASSERT (layer>=0 && layer<mLayers.size());
	
	int index = 0;
	for (int k=0; k<layer; k++)
		index += mLayers[k];

	return index;
}

/** Calculates the relative layer and offset for given absolute
 *  unit index.
 *
 *  @param i Neuron index.
 *
 *  @param layerno Reference-returned result: layer index where
 *  the neuron resides.
 *
 *  @param layerpos Reference-returned result: position of the
 *  neuron on the layer indicated by the layerno result parameter.
 **/
void ANNLayering::getPos (int i, int& layerno, int& layerpos) const {
	layerpos=i;
	for (layerno=0; layerno<mLayers.size() && layerpos>=mLayers[layerno];
		 layerpos-=mLayers[layerno++]);
	ASSERTWITH (layerno<mLayers.size(), format ("Index %d doesn't map to layering", i));
} 

/** Converts the layering back to layering description string.
 **/
String ANNLayering::toString () const {
	// Make an array containing the layer sizes
	Array<String> layers_arr (layers());
	for (int i=0; i<layers(); i++)
		layers_arr[i] = String ((*this)[i]);

	// Join the layer sizes into a layered network description string
	String layers_str;
	layers_str.join (layers_arr, '-');

	return layers_str;
}



/////////////////////////////////////////////////////////////////////////////////////
// |                                     | -----                |                  //
// |      ___         ___       ___      |   |         --       |                  //
// |      ___| \   | /   ) |/\ /   )  ---|   |    __  |  )  __  |  __   ___  \   | //
// |     (   |  \  | |---  |   |---  (   |   |   /  \ |--  /  \ | /  \ (   \  \  | //
// |____  \__|   \_/  \__  |    \__   ---|   |   \__/ |    \__/ | \__/  ---/   \_/ //
//              \_/                                                     __/   \_/  //
/////////////////////////////////////////////////////////////////////////////////////

LayeredTopology::~LayeredTopology () {
}

void LayeredTopology::build (ANNetwork& network) const {
	const ANNLayering& mLayering = dynamic_cast<const ANNLayering&>(*this);
	int inputs=mLayering[0];
	int outputlimit=mLayering.layerIndex (-1);

	// network.makeNeurons (mLayering.totalUnits());

	// Make the unit array
	for (int i=0; i<mLayering.totalUnits(); i++)
		// If unit template is given, use it
		if (network.getUnitPrototype())
			network.add (network.getUnitPrototype()->clone());
		else // Ordinary neuron
			network.add (new Neuron());

	// Set the unit attributes according to their position in the network
	for (int i=0; i<network.size(); i++) {
		// Set the unit's type
		if (i<inputs)
			network[i].setType (Neuron::INPUTUNIT);
		else
			if (i<outputlimit)
				network[i].setType (Neuron::HIDDENUNIT);
		else
			network[i].setType (Neuron::OUTPUTUNIT);

		// Set unit's coordinates

		// Calculate unit's layer and offset
		int layerno, layerpos;
		mLayering.getPos (i, layerno, layerpos);

		// Move the unit to it's proper location
		network[i].moveTo (layerno*8, layerpos, 0);
	}
}

/***************************************************************************
 *  @file topology.cc
 *  @brief Network topology handling.
 ***************************************************************************/

