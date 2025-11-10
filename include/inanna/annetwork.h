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

#ifndef __INANNA_ANNETWORK_H__
#define __INANNA_ANNETWORK_H__

#include <magic/mobject.h>
#include <magic/mcoord.h>
#include <magic/mtable.h>
#include <magic/mattribute.h>
#include "inanna/learning.h"
#include "inanna/topology.h"
#include "inanna/neuron.h"

// External predeclarations
class NeuronInitializer;
class Equalizer;

// Internal predeclarations
class FreeWeight;
class Connection;
class Neuron;
class ANNetwork;



///////////////////////////////////////////////////////////////////////////////////
// |   |                             ___                       o                 //
// |\  |  ___                   _   /   \        _    |   ___      _    ___      //
// | \ | /   ) |   | |/\  __  |/ \  |      __  |/ \  -+-  ___| | |/ \  /   ) |/\ //
// |  \| |---  |   | |   /  \ |   | |     /  \ |   |  |  (   | | |   | |---  |   //
// |   |  \__   \__! |   \__/ |   | \___/ \__/ |   |   \  \__| | |   |  \__  |   //
///////////////////////////////////////////////////////////////////////////////////


// We are so f*cking free that all this f*cking freedom restricts our
// f*cking lives so f*cking much

/*******************************************************************************
 * Neuron container.
 *
 * This class abstracts the handling of neurons in a neural network.
 ******************************************************************************/
class NeuronContainer : public Array<Neuron> {
 	decl_dynamic (NeuronContainer);
  public:
						NeuronContainer		() : mUnits (*this) {}
						NeuronContainer		(int size) : Array<Neuron> (size), mUnits (*this) {}
						~NeuronContainer	();

	//void				makeNeurons		(int size) {make (size);}
	void				add				    (Neuron* neuron);
	void				removeUnit		(int i);
	//void				writeXML		(OStream& out) const;
	virtual void		empty			();

  private:
	void				disconnectAll	();

  protected:
	/** Neurons in the container. */
	Array<Neuron>&	mUnits;
};



///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                _   |   | |   |                           |                //
//               / \  |\  | |\  |  ___   |                  |                //
//              /   \ | \ | | \ | /   ) -+- \    /  __  |/\ | /              //
//              |---| |  \| |  \| |---   |   \\//  /  \ |   |/               //
//              |   | |   | |   |  \__    \   VV   \__/ |   | \              //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
 * A generic and extensible neural network.
 ******************************************************************************/
class ANNetwork : public NeuronContainer, virtual public Learner, public Attributed {
	decl_dynamic (ANNetwork);
  public:

						ANNetwork		(const char* description=NULL);
						ANNetwork		(int size);
						ANNetwork		(const ANNetwork& orig) {copy (orig);}
	virtual				~ANNetwork	();

	void				makeUnits		(const char* topology);
	virtual void		make			(const char* top);
	virtual void		make			(int size);
	virtual void		empty			();
	virtual void		copy			(const ANNetwork& fnet, bool onlyWeights=false);
	void				connectFfw		(const PackTable<int>& conns);
	void				connectFullFfw	(bool shortcuts);
	void				connectFull		();

	void				setUnitPrototype	(Neuron* t) {delete mUnitTemplate; mUnitTemplate=t;}
	const Neuron*		getUnitPrototype	() const {return mUnitTemplate;}

	virtual void		copyFreeNet		(const ANNetwork& orig, bool onlyWeights=false);
	virtual void		init			(double r=0.0);
	void				reset			();
	virtual void		update	 		();
	virtual Vector		testPattern		(const PatternSource& set, int pattern) const;

	/** Returns current layering. */
	const ANNTopology&	getTopology		() const {return *mTopology;}
	//const ANNLayering&	getLayering		() const {return *mTopology;}

	void				setInitializer	(NeuronInitializer* initer);

	/** Returns pointer (or NULL) to the equalization handler for the
	 *  network. Non-const version.
	 **/
	Equalizer*			getEqualizer	() {return mpEqualizer;}

	/** Returns pointer (or NULL) to the equalization handler for the
	 *  network. Const version.
	 **/
	const Equalizer*	getEqualizer	() const {return mpEqualizer;}

	void				setEqualizer	(Equalizer* eq=NULL);

	// Tools

	Connection*			connect			(int i, int j);
	virtual OStream&	operator>>		(OStream& out) const;
	void				cleanup			(bool removeDisableds=false,
										 bool removePassthroughs=false);
	String				drawEPS			(double xsize=-1, double ysize=-1) const;
	void				drawFeedForward ();
	void				check			() const;

  protected:

	ANNTopology*		mTopology;		/**< Layering information. */
	Neuron*				mUnitTemplate;  /**< Neuron template. */
	NeuronInitializer*	mInitializer;	/**< Neuron initializer method. */
	Equalizer*			mpEqualizer;	/**< Equalization object. */

  private:
	/** Used by drawFeedForward() */
	void				orderColumn		(int colStart, int colEnd);

 	void operator= (const ANNetwork& other) {FORBIDDEN} // Prevent
};

#endif
