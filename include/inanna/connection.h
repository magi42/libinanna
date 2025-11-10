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

#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include <magic/mobject.h>

// Predeclarations
class MagiC::TextOStream;

class Neuron;		// In neuron.h
class ANNetwork;	// In freenet.h


///////////////////////////////////////////////////////////////////////////////
//             ___                                   o                       //
//            /   \        _     _    ___   ___   |           _              //
//            |      __  |/ \  |/ \  /   ) |   \ -+- |  __  |/ \             //
//            |     /  \ |   | |   | |---  |      |  | /  \ |   |            //
//            \___/ \__/ |   | |   |  \__   \__/   \ | \__/ |   |            //
///////////////////////////////////////////////////////////////////////////////

/** A weighted connection between two @ref Neuron neurons.
 *
 *  Owner of the connection is always the target neuron, and the
 *  connection points to the source neuron.
 **/
class Connection : public Object {
	decl_dynamic (Connection);

  public:
	
	/** Standard constructor.
	 *
	 *  @param sID Neuron ID of the source neuron.
	 *  @param w   Weight of the connection.
	 **/
							Connection		(const Neuron* source=NULL, const Neuron* target=NULL, double weight=0);
							Connection		(const Connection& orig) {copy (orig);}
	virtual					~Connection		();

	/** Returns the weight of the connection. */
	inline double			weight				() const {return mWeight;}

	/** Sets the weight of the connection. */
	void					setWeight			(double w) {mWeight = w;}

	/** Sets the weight of the connection. */
	void					operator=			(double w) {mWeight = w;}

	/** Initializes the weight randomly to range (-r,r). */
	void					init				(double r=0.0);

	/** Returns the source neuron of the connection. */
	inline const Neuron&	source				() const {return *mpSource;}

	/** Returns the source neuron of the connection. */
	inline Neuron&			source				() {return *mpSource;}

	/** Returns the target neuron of the connection. */
	inline const Neuron&	target				() const {return *mpTarget;}
	
	/** Returns the target neuron of the connection. */
	inline Neuron&			target				() {return *mpTarget;}
	
	/** Reconnects to another source neuron. */
	void					setSource			(const Neuron* newSource) {mpSource=const_cast<Neuron*>(newSource);}

	/** Reconnects to another target neuron. */
	void					setTarget			(const Neuron* newTarget) {mpTarget=const_cast<Neuron*>(newTarget);}

	/** Cuts both source and target references. */
	void					cut					() {mpTarget=NULL; mpSource=NULL;}

	/** Transfers a signal through this connection.
	 *
	 *  @return Output value of the connection.
	 **/
	virtual double			transfer			();

	////////////////////////////////////////
	// Implementations

	/** Standard deep copy operator. */
	virtual void			copy				(const Connection& o);
	
	/** Implementation for @ref Object. */
	virtual Connection*		clone				() const {return new Connection (*this);}

	/** Implementation for @ref Object. */
	virtual void			check				(int netSize) const;

	virtual void			writeXML			(TextOStream& out) const;
	
  private:
	/** Weight of the connection. */
	double		mWeight;

	Neuron*	mpSource;
	Neuron*	mpTarget;
	
  private:
	void operator= (const Connection& other); // Prevent
};

#endif
