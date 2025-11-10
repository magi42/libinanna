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

#include <magic/mmath.h>
#include <magic/mgdev-eps.h>
#include <magic/mclass.h>

#include "inanna/annetwork.h"
#include "inanna/patternset.h"
#include "inanna/initializer.h"
#include "inanna/equalization.h"

impl_dynamic (NeuronContainer, {});
impl_dynamic (ANNetwork, {NeuronContainer});


///////////////////////////////////////////////////////////////////////////////////
// |   |                             ___                       o                 //
// |\  |  ___                   _   /   \        _    |   ___      _    ___      //
// | \ | /   ) |   | |/\  __  |/ \  |      __  |/ \  -+-  ___| | |/ \  /   ) |/\ //
// |  \| |---  |   | |   /  \ |   | |     /  \ |   |  |  (   | | |   | |---  |   //
// |   |  \__   \__! |   \__/ |   | \___/ \__/ |   |   \  \__| | |   |  \__  |   //
///////////////////////////////////////////////////////////////////////////////////

NeuronContainer::~NeuronContainer () {
	// Delete all connection objects
	for (int i=0; i<size(); i++)
		for (int j=0; j<(*this)[i].incomings(); j++) {
			(*this)[i].incoming(j).cut();
			delete & ((*this)[i].incoming(j));
		}

	// All connection pointers are now loose!
	// Clean them.
	for (int i=0; i<size(); i++)
		(*this)[i].shallowDisconnectAll ();
}

/** Adds a neuron to the container.
 *
 *  The container takes the ownership of the neuron and will
 *  delete it properly when the time comes.
 **/
void NeuronContainer::add (Neuron* neuron) {
	Array<Neuron>::add (neuron);
	neuron->setId (size()-1);
}

/** Removes the i:th unit and all connections from it and to it.
 **/
void NeuronContainer::removeUnit (int unitID) {
	// Remove connections to and from the unit
	mUnits[unitID].disconnectAll ();

	// Remove the unit
	mUnits.removeFill (unitID);

	// Adjust unit IDs
	for (int i=unitID; i<mUnits.size(); i++)
		mUnits[i].setId (i);
}

/*******************************************************************************
 * Deletes all the neurons in the network.
 ******************************************************************************/
void NeuronContainer::empty ()
{
	Array<Neuron>::empty ();
}

/*******************************************************************************
 * Disconnects all the neurons in the network.
 ******************************************************************************/
void NeuronContainer::disconnectAll ()
{
	for (int i=0; i<mUnits.size(); i++)
		mUnits[i].disconnectAll ();
}

/** Writes the container in XML. */
/*
void NeuronContainer::writeXML (OStream& out) const {
	sout.printf ("<%s>\n", (CONSTR) getclassname());
	for (int i=0; i<size; i++)
		mUnits[i].writeXML (out);
	sout.printf ("</%s>\n", (CONSTR) getclassname());
}
*/


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//         -----                 |   |                           |           //
//         |          ___   ___  |\  |  ___   |                  |           //
//         |---  |/\ /   ) /   ) | \ | /   ) -+- \    /  __  |/\ | /         //
//         |     |   |---  |---  |  \| |---   |   \\//  /  \ |   |/          //
//         |     |    \__   \__  |   |  \__    \   VV   \__/ |   | \         //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
* Constructs a neural network with a topology described by the description
* string.
*
* See @ref ANNetwork::makeUnits() for more information about the
* format of the description string.
*******************************************************************************/
ANNetwork::ANNetwork (const char* desc)
{
	mUnitTemplate = NULL;
	mInitializer  = NULL;
	mTopology     = NULL;
	mpEqualizer   = NULL;
	if (desc)
		failtrace (makeUnits (desc));
}

/*******************************************************************************
* Constructs a neural network.
*******************************************************************************/
ANNetwork::ANNetwork (int size)
{
	mUnitTemplate = NULL;
	mInitializer  = NULL;
	mTopology     = NULL;
	mpEqualizer   = NULL;

	make (size);
}

ANNetwork::~ANNetwork	()
{
	delete mUnitTemplate;
	delete mInitializer;
	delete mTopology;
	delete mpEqualizer;
}

/*******************************************************************************
 * Builds a non-connected network according to the given layer
 * topology (for example: "7-4-2" or "8").
 *
 * See @ref ANNLayering for some more information about the
 * description strings.
 ******************************************************************************/
void ANNetwork::makeUnits (const char* desc)
{
	delete mTopology;
	mTopology = new LayeredTopology (desc);
	mTopology->build (*this);
}

/*******************************************************************************
 * Implementation for @ref LearningMapping.
 *
 * @see #makeUnits
 ******************************************************************************/
void ANNetwork::make (const char* top)
{
	makeUnits(top);
}

/*******************************************************************************
 * Creates a network with unconnected units.
 ******************************************************************************/
void ANNetwork::make (int size)
{
	Array<Neuron>::make (size);
}

/*******************************************************************************
 * Deletes all the neurons in the network.
 ******************************************************************************/
void ANNetwork::empty ()
{
	mTopology->empty ();
	NeuronContainer::empty ();
}

/** Implementation for @ref LearningMapping. */
void ANNetwork::copy (const ANNetwork& fnet, bool onlyWeights)
{
	Learner::copy (fnet, onlyWeights);
	copyFreeNet (fnet, onlyWeights);
}

/*******************************************************************************
 * Builds the network from connection matrix, restricting the topology
 * to only feedforward connections.
 ******************************************************************************/
void ANNetwork::connectFfw (const PackTable<int>& connmat)
{
	ASSERT (connmat.rows == connmat.cols);
	ASSERT (connmat.rows == size());
	ANNLayering& mLayering = dynamic_cast<ANNLayering&>(*mTopology);

	// Disable according to the diagonal
	int outlindex = mLayering.layerIndex(-1);
	for (int i=0; i<outlindex; i++)
		mUnits[i].enable (connmat.get(i,i));

	// Connect forward
	for (int i=0; i<mUnits.size(); i++)
		for (int j=0; j<mUnits.size(); j++) {
			int ilayer, jlayer, dummy;
			mLayering.getPos (i, ilayer, dummy);
			mLayering.getPos (j, jlayer, dummy);
			if (mUnits[i].isEnabled() && mUnits[j].isEnabled() &&
				ilayer==jlayer-1 && connmat.get(i,j))
				connect (i,j);
		}
}

/*******************************************************************************
 * Connects all units in subsequent layers to form feedforward
 * topology.
 ******************************************************************************/
void ANNetwork::connectFullFfw (
	bool shortcuts /**< If true, every layer is connected to all other layers, not only the adjacent ones. */)
{
	ASSERT (mTopology);
	ANNLayering* mpLayering = dynamic_cast<ANNLayering*>(mTopology);
	if (!mpLayering)
		throw runtime_error (i18n("Neural network didn't have a topology when doing connectFullFfw()"));

	// Index of the first unit of previous layer
	for (int l=1; l<mpLayering->layers(); l++) {

		// Index of the first unit of current layer
		int loffset = mpLayering->layerIndex (l);

		// Connect the pair of layers
		for (int j=0; j<(*mpLayering)[l]; j++)
			for (int pl=shortcuts?0:l-1; pl<=l-1; pl++) {
				int ploffset = mpLayering->layerIndex (pl);
				for (int i=0; i<(*mpLayering)[pl]; i++)
					connect (ploffset+i, loffset+j);
			}
	}
}

/*******************************************************************************
 * Connects all units to all units, including self.
 ******************************************************************************/
void ANNetwork::connectFull ()
{
	// This is simple
	for (int i=0; i<mUnits.size(); i++)
		for (int j=0; j<mUnits.size(); j++)
			connect (i,j);
}

/*******************************************************************************
 * @fn void ANNetwork::setUnitPrototype(Neuron* t)
 *
 * Sets the unit template. The object t must be an instance of an
 * inheritor of @ref Neuron.
 *
 * NOTE: The template is destructed along with the ANNetwork, so do
 * not destroy it yourself.
 ******************************************************************************/

/*******************************************************************************
 * Implementation for @ref AnyNetwork. Copies the other FreeNet unto self.
 ******************************************************************************/
void ANNetwork::copyFreeNet (const ANNetwork& other, bool onlyWeights)
{
	ANNLayering& mLayering = dynamic_cast<ANNLayering&>(*mTopology);
	if (onlyWeights) {
		ASSERTWITH (false, "onlyWeights-copy not implemented for ANNetwork");
	} else {
		// Full reconstructive copy
		mLayering = dynamic_cast<ANNLayering&>(*other.mTopology);
		failtrace (copyClone (mUnits, other.mUnits));
		mUnitTemplate = other.mUnitTemplate? other.mUnitTemplate->clone () : (Neuron*)NULL;
	}
}

/** Implementation for @ref AnyNetwork. Initializes the weights and
 *  biases randomly according to the range, or if an initializer
 *  has been given, using that.
 **/
void ANNetwork::init (double r)
{
	for (int i=0; i<mUnits.size(); i++) {
		if (mInitializer)
			mInitializer->initialize (mUnits[i]);
		else
			mUnits[i].init (r);
	}
}

/** Sets the given initializer. NOTE: Takes the ownership of the
 *  initializer.
 **/
void ANNetwork::setInitializer (NeuronInitializer* initer)
{
	delete mInitializer;
	mInitializer = initer;
}

/** Sets equalization handler for the network. Gives ownership. */
void ANNetwork::setEqualizer (Equalizer* eq)
{
	delete mpEqualizer;
	mpEqualizer = eq;
}

/** Creates a connection from neuron i to neuron j. */
Connection* ANNetwork::connect (int i, int j) {
	ASSERTWITH (i>=0 && i<mUnits.size(),
				format ("Invalid source index from(i)=%d, to(j)=%d", i, j));

	//newComment (format("Connection from %d to %d", i, j));
	Connection* conn = new Connection (&mUnits[i], &mUnits[j]);
	failtrace (mUnits[j].addIncoming (conn));
	failtrace (mUnits[i].addOutgoing (conn));
	return conn;
}

/** Resets activations to 0.0. */
void ANNetwork::reset () {
	for (int i=0; i<mUnits.size(); i++)
		mUnits[i].reset ();
}

/** Transfers signals once for all units. */
void ANNetwork::update () {
	for (int i=0; i<mUnits.size(); i++)
		if (mUnits[i].incomings()>0) // Do not transfer if there are no incoming connections
			mUnits[i].transfer (*this);
}

/*******************************************************************************
 * Implementation for Learner.
 ******************************************************************************/
Vector ANNetwork::testPattern (const PatternSource& set, int pattern) const
{
	Vector result;

	// Feed the pattern into the input layer
	for (int inp=0; inp<set.inputs; inp++)
		(*const_cast<ANNetwork*>(this))[inp].setActivation (set.input (pattern, inp));

	const_cast<ANNetwork*>(this)->update ();

	// Read results
	result.make (set.outputs);
	for (int outp=0; outp<set.outputs; outp++)
		result[outp] = (*this)[size() - set.outputs + outp].activation();

	return result;
}

/*******************************************************************************
 * Implementation for @ref Object.  Prints a human readable
 * representation of the network to given output stream.
 ******************************************************************************/
OStream& ANNetwork::operator>> (OStream& out) const
{
	for (int i=0; i<mUnits.size(); i++) {
		char ttype = "SLE"[mUnits[i].transferFunc()];
		out.printf ("%3d: A=%2.2f b=%+2.2f %c (%d): ",
					mUnits[i].id(), mUnits[i].activation(), mUnits[i].bias(), ttype, mUnits[i].incomings());
		for (int j=0; j<mUnits[i].incomings(); j++)
			out.printf ("%2d:%+2.2f ",
						mUnits[i].incoming(j).source().id(),
						mUnits[i].incoming(j).weight());
		out << "  \n";
	}
	return out;
}

/* Utility function to calculate the angle of a point from origin. */
double quadatan (double x, double y) {
	if (y==0)
		return (x>=0)? 0 : M_PI;

	double at=atan (y/x);

	// Tuodaan oikeaan kvadranttiin
	if (y>0 && x<0) at+=M_PI;
	if (y<0 && x<0) at+=M_PI;
	if (y<0 && x>0)	at+=2*M_PI;

	return at;
}

/*******************************************************************************
 *  Draws a picture of the network in EPS (Encapsulated
 *  PostScript). If no size is given, the image is drawn using
 *  default scaling.
 *
 *  @return Picture as EPS code.
 ******************************************************************************/
String ANNetwork::drawEPS (double xsize, double ysize) const {
	ANNLayering& mLayering = dynamic_cast<ANNLayering&>(*mTopology);
	if (xsize<0 || ysize<0)
		xsize=175, ysize=175;

	// Calculate logical size
	int maxSize=0;
	for (int i=0; i<mLayering.layers(); i++)
		if (mLayering[i]>maxSize)
			maxSize = mLayering[i];
	double size = (mLayering.layers()>maxSize)? mLayering.layers():maxSize;

	Coord2D picSize (175, 175);
	EPSDevice dc (picSize);									// Graphics device
	dc.framedStyle (size, size);							// Clipping with frame

	// Draw cells
	for (int i=0; i<mUnits.size(); i++) {
		const Neuron& unit = mUnits[i];
		const Coord2D& pos = Coord2D (unit.getPlace ());

		bool inOrOutUnit = (i<mLayering[0]) || (i>=mUnits.size()-mLayering[-1])
			|| unit.transferFunc()==Neuron::LINEAR_TF;

		// Draw unit body
		double unitRadius = inOrOutUnit? 0.15 : 0.25;
		dc.saveState ();
		dc.lineWidth (0);
		dc.circle (pos, unitRadius, unit.isEnabled());
		dc.restoreState ();

		// Draw connections
		dc.lineStyle ("->", 0.2);
		if (unit.isEnabled())
			for (int c=0; c<unit.incomings(); c++) {
				const Neuron& source = unit.incoming(c).source();
				const Coord2D& tpos = Coord2D(source.getPlace ());

				// Draw the arrow line to a short distance from source
				double angle = quadatan (tpos.x-pos.x, tpos.y-pos.y);
				double trgRadius = (unit.incoming(c).source().id() >= mUnits.size() - mLayering[-1]
									|| source.transferFunc()==Neuron::LINEAR_TF)? 0.15:0.25;
				Coord2D tmpos (tpos.x-trgRadius*cos(angle), tpos.y-trgRadius*sin(angle));
				dc.lineWidth (0.5);
				dc.saveState ();
				if (unit.transferFunc()==Neuron::LINEAR_TF)
					dc.lineStyle ("dashed", 0.02);
				dc.line (pos, tmpos);
				dc.restoreState ();
			}

		// For output units
		if (i >= mUnits.size() - mLayering[-1]) {
			dc.lineWidth (0.5);
			dc.saveState ();
			if (unit.transferFunc()==Neuron::LINEAR_TF)
				dc.lineStyle ("dashed", 0.02);
			dc.line (pos, pos+Coord2D(0.5,0));
			dc.restoreState ();
		}
	}

	dc.printFooter ();
	return dc.getBuffer ();
}

/*******************************************************************************
 *  Removes unnecessary connections and disables lonely units.
 *
 *  @param removeDisableds Should disabled units be purged?
 *  Default: false (not purged).
 *
 *  @param removePassthroughs Remove units that have only one
 *  input and one output connection. Default: false (not purged).
 ******************************************************************************/
void ANNetwork::cleanup (bool removeDisableds, bool removePassthroughs)
{
	ANNLayering& mLayering = dynamic_cast<ANNLayering&>(*mTopology);
	int inputIndex = mLayering[0];
	int outputIndex = mUnits.size() - mLayering[-1];

	int disconnects;
	do {
		disconnects=0;	// Count the number of disabled units in this iteration
		for (int i=inputIndex; i<outputIndex; i++) {
			// Count incoming connections
			int ins=0;
			for (int j=0; j < mUnits.size(); j++)
				if (mUnits[j].connectedFrom (mUnits[i]))
					ins++;

			// If this unit is unnecessarily connected
			if ((ins>0 && mUnits[i].incomings()==0) || (ins==0 && mUnits[i].incomings()>0)) {
				// Remove any connections to this node
				for (int j=0; j < mUnits.size(); j++)
					if (mUnits[j].connectedFrom (mUnits[i]))
						mUnits[j].disconnectFrom (mUnits[i]);

				// Remove any connections from this node
				mUnits[i].disconnectAll ();

				// Disable it
				mUnits[i].enable (false);

				disconnects++;
				continue;
			}

			// If this unit is a passthrough unit (one input, one or more outputs)
			if (removePassthroughs && (ins==1 && mUnits[i].incomings()>=1)) {
				// Find the input connection
				for (int j=0; j<mUnits.size(); j++)
					if (mUnits[j].connectedFrom (mUnits[i])) {
						// Found. Connect the source unit to all the source units
						mUnits[j].disconnectFrom (mUnits[i]);
						for (int trg=0; trg<mUnits[i].incomings(); trg++)
							mUnits[j].connectFrom (mUnits[i].incoming(trg).source());
						break;
					}
				mUnits[i].disconnectAll ();
				mUnits[i].enable (false); // The unit will be removed
				disconnects++;
				continue;
			}

			// If this unit is a passthrough unit with one output, one or more inputs
			if (removePassthroughs && (ins>=1 && mUnits[i].incomings()==1)) {
				// Find the input connections
				for (int j=0; j<mUnits.size(); j++)
					if (mUnits[j].connectedFrom (mUnits[i])) {
						// Found. Connect the source unit to all the source units
						mUnits[j].disconnectFrom (mUnits[i]);
						mUnits[j].connectFrom (mUnits[i].incoming(0).source());
					}
				mUnits[i].disconnectAll ();
				mUnits[i].enable (false); // The unit will be removed
				disconnects++;
				continue;
			}
		}
	} while (disconnects>0);

	// Disable units that do not have any connections
	for (int i=0; i<outputIndex; i++)
		mUnits[i].enable (mUnits[i].incomings()>0);

	// Remove all disabled hidden units
	if (removeDisableds)
		for (int i=inputIndex; i<mUnits.size()-mLayering[-1]; i++)
			if (!mUnits[i].isEnabled()) {
				removeUnit (i);

				// Huh, now we have to change the layering info.. argh..
				int layer, pos;
				mLayering.getPos (i, layer, pos);
				if (mLayering[layer]>1)
					mLayering[layer]--;
				else {
					// We have to remove the layer because someone doesn't
					// like 0-sized layers. Oh well...
					mLayering.removeLayer (layer);
				}

				// Stay on this index...
				i--;
			}
}

struct ValueIndex : public Comparable {
	double	value;
	int		index;

	int operator== (const Comparable& other) const {return 0;}
	int compare (const Comparable& other) const {
		const ValueIndex& o = dynamic_cast<const ValueIndex&> (other);
		return (value<o.value)? -1 : (value>o.value)? 1 : (index<o.index)? -1 : 1;
	}
};

/*******************************************************************************
 *  Order the coordinates of the @ref Neuron neurons (which do have
 *  coordinates) in the network in a pretty way in two-dimensional
 *  space.
 ******************************************************************************/
void ANNetwork::drawFeedForward () {
	ANNLayering& mLayering = dynamic_cast<ANNLayering&>(*mTopology);
	int inputs = mLayering[0];
	int outputs = mLayering[-1];
	int hiddens = mUnits.size()-inputs-outputs;

	// Collect layer sizes to this string
	String layeringDesc = String(inputs);

	// Position hidden and output units
	if (hiddens) {
		int column = 0;
		int firstOnColumn = inputs;
		for (int i=inputs; i<mUnits.size(); i++) {
			if (i==inputs+hiddens) {
				// Always move to the next column when coming to the output layer
				layeringDesc += String("-") + String(i-firstOnColumn);
				column++;
				firstOnColumn = i;
			} else
				for (int j=firstOnColumn; j<i; j++) {
					if (mUnits[j].isEnabled() && mUnits[j].connectedFrom(mUnits[i]) &&
						i<inputs+hiddens) { // Inside-column connection found.
						// -> Forced to move to the next column
						layeringDesc += String("-") + String(i-firstOnColumn);

						// Move on to the next column
						column++;
						firstOnColumn = i;
						break;
					} // if (connected)
				} // for j
			mUnits[i].mCoord.x = 3*(column+1);
		} // for i
	}

	// Add the last layer column
	layeringDesc += String("-") + String(outputs);
	// TRACE1("%s", (CONSTR) layeringDesc);

	// Change our layering according to this description (if this
	// fails, we are in deep shit)
	mLayering.make (layeringDesc);

	// Calculate the size of the biggest layer
	int maxSize=0;
	for (int i=0; i<mLayering.layers(); i++)
		if (mLayering[i]>maxSize)
			maxSize = mLayering[i];

	// Order the hidden units by their ideal y-positions
	double gridSize = (mLayering.layers()>maxSize)? mLayering.layers():maxSize;
	double layerStep = gridSize/double(mLayering.layers()+0.5);
	// One layer at a time (not the last layer).
	for (int l=0; l<mLayering.layers(); l++) {
		// Handle units on this layer
		Array<ValueIndex> idealY (mLayering[l]);
		int layerBase = mLayering.layerIndex(l);
		for (int i=0; i<mLayering[l]; i++) {
			// Calculate average position of source units
			double sum=0;
			int conns=0;
			for (int j=0; j<layerBase+i; j++)
				if (mUnits[j].connectedFrom(mUnits[layerBase+i])) {
					sum += mUnits[j].mCoord.y;
					conns++;
				}
			if (conns)
				idealY[i].value = sum/conns;
			else
				idealY[i].value = double(i)/mLayering[l];
			idealY[i].index = i+layerBase;
		}

		// Sort it (for all but the output layer)
		if (l<mLayering.layers()-1)
			idealY.quicksort ();

		// Now reorder them
		double unitStep = gridSize/double(mLayering[l]+1);
		for (int i=0; i<mLayering[l]; i++) {
			mUnits[idealY[i].index].moveTo (double(l)*layerStep+0.5,
											double(i+1)*unitStep);
		}
	}
}

inline int sgn (double x) {return (x==0)?0 : ((x<0)? -1:1);}

/** Implementation for @ref Object. */
void ANNetwork::check () const {
	ANNLayering& mLayering = dynamic_cast<ANNLayering&>(*mTopology);
	mLayering.check ();
	for (int i=0; i<mUnits.size(); i++)
		mUnits[i].check (mUnits.size());
	if (mUnitTemplate)
		mUnitTemplate->check (1);
}
