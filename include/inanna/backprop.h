/***************************************************************************
 *   This file is part of the Inanna library.                              *
 *                                                                         *
 *   Copyright (C) 1997-2005 Marko Grönroos <magi@iki.fi>                  *
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

#ifndef __BACKPROPTRAINER_H__
#define __BACKPROPTRAINER_H__

#include "trainer.h"


////////////////////////////////////////////////////////////////////////////////
// ----              |                      -----           o                 //
// |   )  ___   ___  |    --            --    |        ___      _    ___      //
// |---   ___| |   \ | / |  ) |/\  __  |  )   |   |/\  ___| | |/ \  /   ) |/\ //
// |   ) (   | |     |/  |--  |   /  \ |--    |   |   (   | | |   | |---  |   //
// |___   \__|  \__/ | \ |    |   \__/ |      |   |    \__| | |   |  \__  |   //
////////////////////////////////////////////////////////////////////////////////

/** Error back propagation neural learning algorithm.
 *
 *  Design Patterns: Template Method (various parts of the algorithm
 *  can be overloaded).
 **/
class BackpropTrainer : public Trainer {
  public:
	virtual Array<DynParameter>*	parameters	() const;
	virtual void					init		(const StringMap& params);
	
  protected:
	virtual void					initTrain		(ANNetwork& network) const;
	virtual double					trainOnce		(ANNetwork& network, const PatternSource& set) const;
	virtual double					trainPattern	(ANNetwork& network, const PatternSource& set, int p) const;
	virtual void					backpropagate	(ANNetwork& network, const PatternSource& set, int p) const;
	virtual void					updateWeights	(ANNetwork& network) const;

  protected:
	double	mEta;			/**< Learning speed. */
	double	mMomentum;		/**< Momentum. */
	double	mDecay;			/**< Weight decay multiplier. */
	bool	mBatchLearning;	/**< Should batch learning be used? */

	/** Deltas for each weight in the network, in internal order.
	 *
	 *  We store these here, because we don't want to alter the
	 *  network objects just because of the training algorithm.
	 **/
	mutable Vector	mWeightDeltas;

	/** Errors at each neuron.
	 *
	 *  We store these here, because we don't want to alter the
	 *  network objects just because of the training algorithm.
	 **/
	mutable Vector	mError;
};

#endif
