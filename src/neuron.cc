/***************************************************************************
 *   This file is part of the Inanna library.                              *
 *                                                                         *
 *   Copyright (C) 1998-2002 Marko Grönroos <magi@iki.fi>                  *
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

#include "inanna/initializer.h"


// Implementations for initializer.h
impl_abstract (NeuronInitializer, {Object});
impl_dynamic (DummyInitializer, {NeuronInitializer});
impl_dynamic (GaussianInitializer, {NeuronInitializer});

// Local implementations
impl_dynamic (BiNode, {Object});
impl_dynamic (Neuron, {BiNode});



//////////////////////////////////////////////////////////////////////////////
//                 ___          o                  __  ___                  //
//                |   | |          ___   ___   |  /  \ |  \                 //
//                |   | |---    | /   ) |   \ -+-  __/ |   |                //
//                |   | |   )   | |---  |      |     \ |   |                //
//                `___´ |__/  \_|  \__   \__/   \ \__/ |__/                 //
//////////////////////////////////////////////////////////////////////////////

void Object3D::copy (const Object3D& other) {
	mCoord = other.mCoord;
}

void Object3D::getPlace (double& x, double& y, double& z) const {
	x = mCoord.x;
	y = mCoord.y;
	z = mCoord.z;
}



//////////////////////////////////////////////////////////////////////////////
//                      ----  o |   |          |                            //
//                      |   )   |\  |          |  ___                       //
//                      |---  | | \ |  __   ---| /   )                      //
//                      |   ) | |  \| /  \ (   | |---                       //
//                      |___  | |   | \__/  ---|  \__                       //
//////////////////////////////////////////////////////////////////////////////

BiNode::~BiNode () {
	disconnectAll ();
}

void BiNode::copy (const BiNode& other) {
	mIncoming = other.mIncoming;
	mOutgoing = other.mOutgoing;

#ifdef CMP_WARNINGS
#warning "TODO: BiNode::copy()"
#endif
	/*
	for (int j=0; j<mIncoming.size; j++) {
		ASSERTWITH (other.mIncoming.getp (j), format("%d, %d, %d", c0, j, other.mIncoming.size));
		ASSERTWITH (mIncoming.getp (j), format("%d, %d", j, other.mIncoming.size));
		mIncoming[j];
	}
	*/
}

void BiNode::connectFrom (const BiNode& source) {
	Connection* newconn = new Connection (dynamic_cast<Neuron*>(&const_cast<BiNode&>(source)),
										  dynamic_cast<Neuron*>(this));
	mIncoming.add (newconn);
	const_cast<BiNode&>(source).addOutgoing (newconn);
}

bool BiNode::connectedFrom (const BiNode& other) const {
	for (int i=0; i<mIncoming.size(); i++)
		if (&mIncoming[i].source() == &other)
			return true;
	return false;
}

// Disconnecting

void BiNode::disconnectFrom (const Connection& connection) {
	for (int i=0; i<mIncoming.size(); i++)
		if (&mIncoming[i] == &connection) {
			mIncoming[i].setTarget (NULL);

			// Order the source unit to cut its link
			if (!isnull(connection.source()))
				mIncoming[i].source().disconnectTo (connection);
			
			// We are the target, so we delete it.
			mIncoming.removeFill (i);
			i--;
		}
}

void BiNode::disconnectTo (const Connection& connection) {
	for (int i=0; i<mOutgoing.size(); i++)
		if (&mOutgoing[i] == &connection) {
			mOutgoing[i].setSource (NULL);

			// We are the source, so we let the target to delete the
			// connection
			if (!isnull(mOutgoing[i].target()))
				mOutgoing[i].target().disconnectFrom (connection);

			// We just cut our reference to it
			mOutgoing.cut (i);
			mOutgoing.removeFill (i);
			i--;
		}
}

void BiNode::disconnectFrom (const BiNode& node) {
	for (int i=0; i<mIncoming.size(); i++)
		if (&mIncoming[i].source() == &node) {
			mIncoming[i].setTarget (NULL);

			// Order the source unit to cut its link
			if (!isnull(mIncoming[i].source()))
				mIncoming[i].source().disconnectTo (mIncoming[i]);
			
			// We are the target, so we delete it.
			mIncoming.removeFill (i);
			i--;
		}
}

void BiNode::disconnectTo (const BiNode& node) {
	for (int i=0; i<mOutgoing.size(); i++)
		if (&mOutgoing[i].target() == &node) {
			mOutgoing[i].setSource (NULL);

			// We are the source, so we let the target to delete the
			// connection
			if (!isnull(mOutgoing[i].target()))
				mOutgoing[i].target().disconnectFrom (mOutgoing[i]);

			// We just cut our reference to it
			mOutgoing.cut (i);
			mOutgoing.removeFill (i);
			i--;
		}
}

void BiNode::disconnectAll () {
	for (int i=0; i<mIncoming.size(); i++) {
		mIncoming[i].setTarget (NULL);
		
		// Order the source unit to cut its link
		if (!isnull(mIncoming[i].source()))
			mIncoming[i].source().disconnectTo (mIncoming[i]);
		
		// We are the target, so we delete it.
		mIncoming.remove (i);
	}
	mIncoming.make (0);

	for (int i=0; i<mOutgoing.size(); i++) {
		mOutgoing[i].setSource (NULL);
		
		// We are the source, so we let the target to delete the
		// connection
		if (!isnull(mOutgoing[i].target()))
			mOutgoing[i].target().disconnectFrom (mOutgoing[i]);
		
		// We just cut our reference to it
		mOutgoing.cut (i);
	}
	mOutgoing.make (0);
}

void BiNode::shallowDisconnectAll () {
	for (int i=0; i<mIncoming.size(); i++)
		mIncoming.cut (i);
	mIncoming.make (0);
	
	for (int i=0; i<mOutgoing.size(); i++)
		mOutgoing.cut (i);
	mOutgoing.make (0);
}

/*virtual*/ void BiNode::check (int netSize) const {
	for (int i=0; i<incomings(); i++)
		incoming(i).check (netSize);
}

/*
void BiNode::writeXML (OStream& out) const {
	sout.printf ("\t<%s ID=%d>\n", (CONSTR) getclassname(), id());
	for (int i=0; i<mIncoming.size(); i++)
		mIncoming[i].writeXML (out);
	sout.printf ("\t</%s>\n", (CONSTR) getclassname());
}
*/


//////////////////////////////////////////////////////////////////////////////
//                     |   |                                                //
//                     |\  |  ___                   _                       //
//                     | \ | /   ) |   | |/\  __  |/ \                      //
//                     |  \| |---  |   | |   /  \ |   |                     //
//                     |   |  \__   \__! |   \__/ |   |                     //
//////////////////////////////////////////////////////////////////////////////

Neuron::Neuron () : BiNode(), Object3D()
{
	mActivation = 0.0;
	mBias = 0.0;
	mType = HIDDENUNIT;
	mTransferFunc = LOGISTIC_TF;
	mExists = true;
}

Neuron::~Neuron () {
	;
}

void Neuron::copy (const Neuron& other) {
	Object3D::copy (other);
	BiNode::copy (other);

	// int j,c0=-666;

	mActivation = other.mActivation;
	mBias.copy (other.mBias);
	mType = other.mType;
	mTransferFunc = other.mTransferFunc;
	mExists = other.mExists;
}

void Neuron::init (double r)
{
	if (mExists) {
		for (int i=0; i<incomings(); i++)
			incoming(i).init (r);
	}
	mBias.init (r);
}

void Neuron::transfer (ANNetwork& net)
{
	if (mExists) {
		register double sum = mBias.weight ();
		for (register int i=0; i<incomings(); i++)
			sum += incoming(i).weight()*incoming(i).source().activation();

		// NOTE: The sigmoid function is currently hard-coded, as this
		// makes backprop A LOT faster
		if (mTransferFunc == LOGISTIC_TF)
			  mActivation = sigmoid (sum);
		else
			  mActivation = sum;
		// TODO: Implement other TFs
	} else
		mActivation = 0.0;
}

void Neuron::check (int netSize) const
{
	ASSERT (mType>=0 && mType<=2);
	ASSERT (mTransferFunc>=0 && mTransferFunc<=2);
	mBias.check (1);
	ASSERT (mActivation>=-10000 && mActivation<=10000); // Sensible range

	BiNode::check (netSize);
}

void Neuron::operator= (const Neuron& other) {
	copy (other);
}

