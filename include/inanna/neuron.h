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

#ifndef __NEURON_H__
#define __NEURON_H__

#include <magic/mobject.h>
#include <magic/mcoord.h>
#include <magic/mpararr.h>

#include "connection.h"

// Externals
class NeuronInitializer;
class NeuronContainer;		// In freenet.h


// We can not be truly free before we can freely connect ourself in a
// global free network where we can freely control each other!


//////////////////////////////////////////////////////////////////////////////
//                 ___          o                  __  ___                  //
//                |   | |          ___   ___   |  /  \ |  \                 //
//                |   | |---    | /   ) |   \ -+-  __/ |   |                //
//                |   | |   )   | |---  |      |     \ |   |                //
//                `___´ |__/  \_|  \__   \__/   \ \__/ |__/                 //
//////////////////////////////////////////////////////////////////////////////

class Object3D {
  public:

	void 					copy			(const Object3D& other);
	
	/** Fetches the 3-dimensional coordinates of the neuron in a
	 *  neuron space.
	 **/
	const Coord3D&			getPlace		() const {return mCoord;}

	/** Fetches the 3-dimensional coordinates of the neuron in a
	 *  neuron space. The result is returnes in the reference
	 *  parameters.
	 **/
	void					getPlace		(double& x, double& y, double& z) const;

	/** Move the neuron to the given coordinates in a 3-dimensional
	 *  neuron space.
	 **/
	void					moveTo			(const Coord3D& o) {mCoord = o;}

	/** Move the neuron to the given coordinates in a 3-dimensional
	 *  neuron space. The third dimension is not required.
	 **/
	void					moveTo			(double x, double y, double z=0.0) {mCoord.moveTo (x,y,z);}

  protected:
	/** Coordinates */
	Coord3D			mCoord;
};



//////////////////////////////////////////////////////////////////////////////
//                      ----  o |   |          |                            //
//                      |   )   |\  |          |  ___                       //
//                      |---  | | \ |  __   ---| /   )                      //
//                      |   ) | |  \| /  \ (   | |---                       //
//                      |___  | |   | \__/  ---|  \__                       //
//////////////////////////////////////////////////////////////////////////////

/** Bidirectionally linked graph node.
 **/
class BiNode : public Object {
	decl_dynamic (BiNode);
  public:
							BiNode			() {}
							BiNode			(const BiNode& orig) {copy (orig);}
	virtual					~BiNode			();
	
	/** Adds an incoming connection. It is assumed that the source of
     *  the connection is a valid neuron.
	 **/
	void					addIncoming		(Connection* conn) {mIncoming.add (conn);}

	/** Adds an outgoing connection. It is assumed that the target of
     *  the connection is a valid neuron.
	 **/
	void					addOutgoing		(Connection* conn) {mOutgoing.add (conn);}

	/** Adds a connection to the neuron from neuron with id s. */
	void					connectFrom		(const BiNode& source);

	/** Returns whether or net the neuron has an incoming connection
	 *  from given neuron.
	 **/
	bool					connectedFrom	(const BiNode& source) const;

	// Disconnecting

	/** Disconnects given incoming connection.
	 **/
	void					disconnectFrom	(const Connection& incoming);
	
	/** Disconnects given outgoing connection.
	 **/
	void					disconnectTo	(const Connection& outgoing);

	/** Disconnects any incoming connection from the given neuron.
	 **/
	void					disconnectFrom	(const BiNode& source);

	/** Disconnects any outgoing connection to the given neuron.
	 **/
	void					disconnectTo	(const BiNode& target);
	
	/** Removes ALL incoming and outgoing connections to the neuron. */
	void					disconnectAll	();

	// Connection access
	
	Connection&				incoming		(int i) {return mIncoming[i];}
	const Connection&		incoming		(int i) const {return mIncoming[i];}

	Connection&				outgoing		(int i) {return mOutgoing[i];}
	const Connection&		outgoing		(int i) const {return mOutgoing[i];}

	/** Returns the number of incoming connections to the neuron. */
	int						incomings		() const {return mIncoming.size();}

	/** Returns the number of outgoing connections from the neuron. */
	int						outgoings		() const {return mOutgoing.size();}

	int						id				() const {return mId;}

	/** Standard copy operator. */
	virtual void			copy			(const BiNode& orig);

	/** Implementation for @ref Object */
	virtual void			check			(int netSize) const;

	//virtual void			writeXML		(OStream& out) const;

  private:
	int					mId;

	/** Incoming connections to the neuron */
	Array<Connection>	mIncoming;

	/** Outgoing connections from the unit */
	Array<Connection>	mOutgoing;
	
	/** Lets the NeuronContainer set the ID. */
	void				setId			(int id) {mId = id;}

	/** Disconnects all connections, but doesn't destroy the
	 *  Connection objects.
	 **/
	void				shallowDisconnectAll	();
	
	friend class NeuronContainer;
};

//////////////////////////////////////////////////////////////////////////////
//                     |   |                                                //
//                     |\  |  ___                   _                       //
//                     | \ | /   ) |   | |/\  __  |/ \                      //
//                     |  \| |---  |   | |   /  \ |   |                     //
//                     |   |  \__   \__! |   \__/ |   |                     //
//////////////////////////////////////////////////////////////////////////////

/** The baseclass of neurons in @ref ANNetwork.
 *
 *  Somewhat bloated.
 *
 *  It handles signal transmission in reading manner (connections
 *  point to source units).
 **/
class Neuron : public BiNode, public Object3D {
	decl_dynamic (Neuron);
  public:

							Neuron		();
							Neuron		(const Neuron& orig) : mBias (0,0) {copy(orig);}
							~Neuron		();

	/** Initialize weights and bias to random value in given range.
	 **/
	virtual void			init			(double r=0.0);

	/** Resets activation (and other possible dynamic states) to 0.0.
	 **/
	void					reset			() {mActivation=0.0;}

	/** Updates the unit by transferring signals through the
	 *  connections.
	 *
	 *  @param net The owner of the neuron, because it doesn't know it otherwise.
	 **/
	virtual void			transfer		(ANNetwork& net);

	// Manipulation

	/** Returns the activation value.
	 *
	 *  Unless overloaded, this value is identical with the @ref Neuron::output() value.
	 *
	 *  @see Neuron::output
	 **/
	inline double			activation		() const {return mActivation;}

	/** Sets the activation value of the neuron. */
	virtual void			setActivation	(double a) {mActivation = a;}
	
	/** Sets the activation value of the neuron. */
	void					operator=		(double a) {mActivation=a;}
	
	/** Returns the output value of the neuron.
	 *
	 *  Unless overloaded, this value is identical with the @ref Neuron::activation() value.
	 *
	 *  @see Neuron::activation
	 */
	virtual double			output			() const {return mActivation;}
	
	/** Returns the output value of the neuron.
	 *
	 *  @see Neuron::output
	 */
							operator const double	() const {return mActivation;}

	/** Returns the current bias (threshold) value of the neuron. */
	double					bias			() const {return mBias.weight();}

	/** Sets the bias (threshold value of the neuron to the given value. */
	void					setBias			(double b) {mBias = b;}

	Connection&				getBiasObj		() {return mBias;}

	/** Returns the type of the neuron; is it an input, hidden or output
	 *  unit. See @ref Neuron::unitTypes for information on the possible
	 *  values.
	 **/
	int						getType			() const {return mType;}
	
	/** Sets the type of the neuron; is it an input, hidden or output
	 *  unit. See @ref Neuron::unitTypes for information on the possible
	 *  values. This information is typically not very important.
	 **/
	void					setType			(int t) {mType = t;}

	/** Returns the transfer function ID of the neuron. See @ref
	 *  Neuron::tfuncs for information about the different
	 *  functions.
	 **/
	int						transferFunc	() const {return mTransferFunc;}

	/** Sets the transfer function ID of the neuron. See @ref
	 *  Neuron::tfuncs for information about the different
	 *  functions.
	 **/
	void					setTFunc		(int f) {mTransferFunc = f;}

	/** Enables (true) or disables (false) the neuron. If the neuron
	 *  is disabled, it's @ref Neuron::output() value will always
	 *  be 0.0. It will also (practically) not use any computational
	 *  resources.
	 *
	 *  @see Neuron::isEnabled()
	 **/
	void					enable			(bool s=true) {mExists=s;}

	/** Returns the enabledness state of the neuron.
	 *
	 *  @see Neuron::enable()
	 **/
	bool					isEnabled		() const {return mExists;}

	// Connection manipulation

	/** Standard copy operator. */
	void					copy			(const Neuron& orig);

	/** Implementation for @ref Object */
	virtual Neuron*		clone			() const {return new Neuron(*this);}

	/** Implementation for @ref Object */
	virtual void			check			(int netSize) const;

	/** Implementation (temporary). */
	//virtual void			writeXML		(OStream& out) const;

	/** Neuron types: is neuron input, hidden or output unit. */
	enum unitTypes {INPUTUNIT=0, HIDDENUNIT=1, OUTPUTUNIT=2};

	/** Transfer functions. */
	enum tfuncs {LOGISTIC_TF=0, LINEAR_TF=1, ELLIOTT_TF=2};

  protected:
	
	/** Activation value of the unit. */
	double			mActivation;

	/** Bias (threshold). */
	Connection		mBias;

	/** Unit type (see unitTypes) */
	int				mType;

	/** Transfer function type (or -1 if default) */
	int				mTransferFunc;
	
	/** Does this unit really exist or not (is it active or dormant) */
	bool			mExists;
	
  private:
	/** Lower all the target indexes above the given index by one
	 *  (removal of the unit by that index).
	**/
	//void			adjustAbove		(int targetIndex);

	void operator= (const Neuron& other); // Prevent

	friend class NeuronContainer;
	friend class ANNetwork;
};



///////////////////////////////////////////////////////////////////////////////
//       |   |                            |   | o       o                    //
//       |\  |  ___                   _   |   |    ____    |                 //
//       | \ | /   ) |   | |/\  __  |/ \  |   | | (     | -+-  __  |/\       //
//       |  \| |---  |   | |   /  \ |   |  \ /  |  \__  |  |  /  \ |         //
//       |   |  \__   \__! |   \__/ |   |   V   | ____) |   \ \__/ |         //
///////////////////////////////////////////////////////////////////////////////

/** For making operations for neurons.
 *
 *  Design Patterns: Visitor, Strategy (different visitors are strategies).
 **/
class NeuronVisitor {
  public:
	virtual ~NeuronVisitor () {}
	virtual void	visit	(Neuron& neuron) const=0;
};

#endif
