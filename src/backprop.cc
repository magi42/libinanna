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

#include "inanna/backprop.h"
#include "inanna/patternset.h"


////////////////////////////////////////////////////////////////////////////////
// ----              |                      -----           o                 //
// |   )  ___   ___  |    --            --    |        ___      _    ___      //
// |---   ___| |   \ | / |  ) |/\  __  |  )   |   |/\  ___| | |/ \  /   ) |/\ //
// |   ) (   | |     |/  |--  |   /  \ |--    |   |   (   | | |   | |---  |   //
// |___   \__|  \__/ | \ |    |   \__/ |      |   |    \__| | |   |  \__  |   //
////////////////////////////////////////////////////////////////////////////////

/*virtual*/ void BackpropTrainer::init (const StringMap& params) {
	Trainer::init (params);
	INITPARAMS(params, 
			   mEta				= params["BackpropTrainer.eta"].toDouble();
			   mMomentum		= params["BackpropTrainer.momentum"].toDouble();
			   mDecay			= params["BackpropTrainer.decay"].toDouble();
			   mBatchLearning	= params["BackpropTrainer.batchLearning"].toInt();
		);
}

/*virtual*/ Array<DynParameter>* BackpropTrainer::parameters () const {
	Array<DynParameter>* result = new Array<DynParameter>;
	result->add (new DoubleParameter	("eta", i18n("Learning rate"), 15, 0.0, 1.0, 0.25));
	result->add (new DoubleParameter	("momentum", i18n("Weight momentum"), 15, 0.0, 1.0, 0.9));
	result->add (new DoubleParameter	("decay", i18n("Weight decay multiplier"), 15, 0.5, 1.0, 1.0));
	result->add (new BoolParameter		("batchLearning", i18n("Update weights in batch")));

	return result;
}

/*******************************************************************************
 * Initializes training.
 ******************************************************************************/
/*virtual*/ void BackpropTrainer::initTrain (ANNetwork& network) const
{
	Trainer::initTrain (network);
	
	// Count the total number of connections in the entire network
	int connections=0;
	for (int i=0; i<network.size(); i++)
		connections += network[i].incomings();

	// Create weight delta data for connections and biases
	mWeightDeltas.make (connections + network.size());
	for (int i=0; i<mWeightDeltas.size(); i++)
		mWeightDeltas[i] = 0.0;
}

/*******************************************************************************
 * Implementation for Trainer.
 ******************************************************************************/
/*virtual*/ double BackpropTrainer::trainOnce (ANNetwork& network, const PatternSource& set) const
{
#ifdef CMP_WARNINGS
#warning "TODO: Batch learning is disabled right now."
#endif
	/*	  
	if (mBatchLearning)
		for (int i=0; i<mWeightDeltas.size(); i++)
			mWeightDeltas[i] = 0.0;
	*/
	
	// Train each pattern once
	double sse=0.0;
	for (int p=0; p<set.patterns; p++)
		sse += trainPattern (network, set, p);

	if (true || mBatchLearning)
		updateWeights (network);

	// Actualize weight adjustments
	/*
	if (mBatchLearning)
		for (int n=0, oc=0; n<network.size(); n++)
			for (int c=0; c<network[n].incomings(); c++, oc++)
				network[n].incoming(c).setWeight (mWeightDeltas[oc]);
	*/
	return sse/set.patterns; // Return MSE
}

/*******************************************************************************
 * Trains one pattern.
 ******************************************************************************/
/*virtual*/ double BackpropTrainer::trainPattern (ANNetwork& network, const PatternSource& set, int p) const
{
	// Feed the pattern to the network
	for (int inp=0; inp<set.inputs; inp++)
		network[inp].setActivation (set.input (p, inp));

	// Forward pass
	network.update ();

	// Backward pass
	backpropagate (network, set, p);

	// Calculate error
	double sse=0.0;
	for (int outp=0; outp<set.outputs; outp++)
		sse += sqr(set.output(p,outp) - network[network.size() - set.outputs + outp].activation());

	return sse / set.outputs; // Return MSE
}

/*******************************************************************************
 * Propagates an error signal backwards in the network. Does not
 * modify the network in any way, but stores the per-neuron error in
 * mError.
 ******************************************************************************/
void BackpropTrainer::backpropagate (register ANNetwork& network,
									 register const PatternSource& set,
									 int p) const
{
	mError.make (network.size());
	int outLayerBase = network.size() - set.outputs;
	//register Connection* conn;
	register double sum_k;
	register Neuron* neuron_j;
	register double delta_j;
	register int j;
	//register int k;
	
	// Iterate backwards
	for (j=network.size()-1; j>=0; j--) {
		neuron_j = &network[j];
		// Calculate error at a neuron
		if (j >= outLayerBase) { // Output neuron
			delta_j = (set.output(p,j-outLayerBase) - neuron_j->activation())
				* neuron_j->activation() * (1.0 - neuron_j->activation());
		}
		else { // A hidden or input neuron
			sum_k=0.0;
			for (int k=0; k<neuron_j->outgoings(); k++)
				sum_k += mError [neuron_j->outgoing(k).target().id()] * neuron_j->outgoing(k).weight();
			
			delta_j = neuron_j->activation()*(1.0-neuron_j->activation()) * sum_k;
		}
		mError[j] = delta_j;
	}
}

/*******************************************************************************
 * Updates weights after backpropagation phase.
 ******************************************************************************/
void BackpropTrainer::updateWeights (register ANNetwork& network) const {
	register int j, i, ji;
	register double deltaw_ji;
	
	for (j=network.size()-1, ji=0; j>=0; j--) {
		// Update bias
		deltaw_ji = mEta * mError[j];
		network[j].setBias (network[j].bias() + deltaw_ji + mMomentum*mWeightDeltas[ji]);
		mWeightDeltas[ji++] = deltaw_ji;

		// Update weights
		for (i=network[j].incomings()-1; i>=0; i--, ji++) {
			register Connection& conn = network[j].incoming(i);
			deltaw_ji = mEta * mError[j] * conn.source().activation();
			conn.setWeight (conn.weight() + deltaw_ji + mMomentum*mWeightDeltas[ji]);
			mWeightDeltas[ji] = deltaw_ji;
		}
	}
}
