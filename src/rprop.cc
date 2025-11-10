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

#include "inanna/rprop.h"
#include "inanna/patternset.h"

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//        ----  ----                -----           o                        //
//        |   ) |   )           --    |        ___      _    ___             //
//        |---  |---  |/\  __  |  )   |   |/\  ___| | |/ \  /   ) |/\        //
//        | \   |     |   /  \ |--    |   |   (   | | |   | |---  |          //
//        |  \  |     |   \__/ |      |   |    \__| | |   |  \__  |          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

/*virtual*/ void RPropTrainer::init (const StringMap& params)
{
	Trainer::init (params);
	
	INITPARAMS(params, 
			   mDelta0			= params["RPropTrainer.delta0"].toDouble();
			   mDeltaMax		= params["RPropTrainer.deltamax"].toDouble();
			   mDecay			= params["BackpropTrainer.decay"].toDouble();
			   mBatchLearning	= params["BackpropTrainer.batchLearning"].toInt();
		);
}

/*virtual*/ Array<DynParameter>* RPropTrainer::parameters () const
{
	Array<DynParameter>* result = new Array<DynParameter>;
	result->add (new DoubleParameter	("delta0", i18n("Initial learning rate"), 15, 0.0, 100.0, 0.1));
	result->add (new DoubleParameter	("deltamax", i18n("Maximum learning rate"), 15, 0.0, 100.0, 50.0));
	result->add (new DoubleParameter	("decay", i18n("Weight decay multiplier"), 15, 0.5, 1.0, 1.0));
	result->add (new IntParameter		("maxCycles", i18n("Max training cycles"), 1, 100000, 100));
	result->add (new BoolParameter		("batchLearning", i18n("Update weights in batch")));

	return result;
}

/** Implementation for BackpropTrainer. Initializes training. */
/*virtual*/ void RPropTrainer::initTrain (ANNetwork& network) const
{
	BackpropTrainer::initTrain (network);
	
	// Set initial deltas
	mDelta.make (mWeightDeltas.size());
	for (int i=0; i<mDelta.size(); i++)
		mDelta[i] = mDelta0;

	// Initialize "previous neuron error"
	mGradient.make (mWeightDeltas.size());
	for (int i=0; i<mGradient.size(); i++)
		mGradient[i] = 0.0;
}

inline double sign (double x) {return (x>=0)? 1:-1;}
inline double min (double x, double y) {return (x>y)? y:x;}

// This algorithm is largely copied from the SNNS implementation,
// found in SNNS/kernel/sources/learn_f.c
//
// SNNS variable key:
//  value_a = mDelta[ji] = "update value" = error_ji = 
//  value_b = mOldDeltaW[ji] = delta_w(t-1)
//  value_c = gradient_ji = sum(dEdw)


/*******************************************************************************
 * Implementation for BackpropTrainer.
 ******************************************************************************/
/*virtual*/ void RPropTrainer::backpropagate (ANNetwork& network,
											  const PatternSource& set,
											  int p) const
{
	BackpropTrainer::backpropagate (network, set, p);
	
	// Calculate per-weight errors
	for (register int j=network.size()-1, ji=0; j>=0; j--)
		for (register int i=-1; i<network[j].incomings(); i++, ji++)
			if (i==-1) // Bias
				mGradient[ji] -= mError[j];
			else // Weight
				mGradient[ji] -= mError[j] * network[j].incoming(i).source().activation();
}

Connection nullconn;

/** Updates weights after backpropagation phase. */
/*virtual*/ void RPropTrainer::updateWeights (ANNetwork& network) const
{
	for (int j=network.size()-1, ji=0; j>=0; j--) {
		// Update weights for the neuron j
		for (int i=-1; i<network[j].incomings(); i++, ji++) {
			Connection& conn = (i==-1)? network[j].getBiasObj() : network[j].incoming(i);
			double& delta = mDelta[ji];

			// Weight decay
			double gradient_ji = mGradient[ji] + (1-mDecay)*conn.weight();

			// Calculate dw * dEdw
			double direction = gradient_ji * mWeightDeltas[ji];

			if (direction < 0.0) {			// Same direction as before: dw * dEdw < 0
				delta *= 1.2;
				if (delta > mDeltaMax)
					delta = mDeltaMax;
				if (gradient_ji < 0.0)
					mWeightDeltas[ji] =  delta;
				else
					mWeightDeltas[ji] = -delta;
			} else if (direction > 0.0) {	// Direction changed
				mWeightDeltas[ji] = 0.0;
				delta *= 0.5;
				if (delta < 1E-6)
					delta = 1E-6;
			} else {						// RProp learning process has just started
				if (gradient_ji<0.0)
					mWeightDeltas[ji] = delta;
				else
					mWeightDeltas[ji] = -delta;
			}
			
			// Update weight or bias
			if (i==-1)
				network[j].setBias (network[j].bias() + mWeightDeltas[ji]);
			else
				conn.setWeight (conn.weight() + mWeightDeltas[ji]);
			mGradient[ji] = 0.0;
		}
	}
}
