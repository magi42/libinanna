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

#ifndef __INITIALIZER_H__
#define __INITIALIZER_H__

#include <magic/mobject.h>
#include "annetwork.h"

/** Abstract interface class for @ref Neuron weight initializers.
 **/
class NeuronInitializer : public Object {
  public:
					NeuronInitializer	() {}
					NeuronInitializer	(const NeuronInitializer& orig) {}

	/** The implementor should overload this to initialize the neuron
	 *  and it's weights.
	 **/
	virtual void	initialize			(Neuron& neuron)=0;

	/** Implementor has to overload this if the initializers are to be
	 *  cloned.
	 **/
	virtual NeuronInitializer*	clone	() const {MUST_OVERLOAD; return NULL;}

  private:
	void operator= (const NeuronInitializer& orig) {FORBIDDEN}
	decl_dynamic (NeuronInitializer);
};

/** Standard initializer for @ref Neuron weights.
 **/
class GaussianInitializer : public NeuronInitializer {
  public:

	/** Standard constructor.
	 *
	 *  @param r Standard deviation for weight initialization.
	 **/
					GaussianInitializer	(double r=0.5) : mR (r) {}
					GaussianInitializer (const GaussianInitializer& orig) : mR (orig.mR) {}

	/** Initializes the neuron's weights with standard, normally
	 *  distributed initialization function.
	 **/
	void			initialize			(Neuron& neuron) {
		neuron.init (mR);
	}
	
	/** Implementation for @ref Object. */
	virtual GaussianInitializer* clone	() const {return new GaussianInitializer (*this);}
	
  private:
	double	mR;

	void operator= (const GaussianInitializer& orig) {FORBIDDEN}
	decl_dynamic (GaussianInitializer);
};

/** Dummy @ref NeuronInitializer just leaves the neurons uninitialized.
 **/
class DummyInitializer : public NeuronInitializer {
  public:
					DummyInitializer	() {}
					DummyInitializer	(const DummyInitializer& orig) {}

	/** Dummy. Does not initialize.
	 **/
	void			initialize			(Neuron& neuron) {}

	/** Implementation for @ref Object. */
	virtual DummyInitializer* clone		() const {return new DummyInitializer (*this);}
  private:

	void operator= (const DummyInitializer& orig) {FORBIDDEN}
	decl_dynamic (DummyInitializer);
};

#endif
