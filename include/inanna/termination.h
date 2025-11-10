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

#ifndef __TERMINATION_H__
#define __TERMINATION_H__

/** @file termination.h
 *
 *  This module defines several early-stopping strategies
 **/

#include "annetwork.h"

// Externals
class ANNetwork;
class PatternSource;
class Trainer;

/** Abstract superclass for early stopping strategies for @ref AnyNetwork.
 *
 *  The idea behing early stopping is as follows. When neural networks
 *  are trained with a training set, they often learn the training set
 *  so well that they "overfit" to it and start behaving badly with
 *  other patterns. With early stopping we try to stop the learning at
 *  the time when the networks starts to generalize
 *  badly. Generalization ability is tested with a separate validation
 *  set.
 *
 *  Terminates the subject when it is about to learn too much.
 **/
class Terminator : public Object {
  public:
	/** Standard constructor.
	 *
	 *  @param validationset Validation set that is used for measuring
	 *  the generalization ability of the neural network taught with
	 *  the separate training data.
	 *
	 *  @param striplen Interval of training cycles between validation
	 *  and thus early stopping test.
	 **/
					Terminator			(const PatternSource& validationset, int striplen);
	

	/** Tests the networks with the validation set and returns 'true'
	 *  if the training should be terminated. The @ref
	 *  Terminator::restore method should be called to restore
	 *  the optimal network state.
	 *
	 *  @see SavingTerminator
	 **/
	bool			validate			(const ANNetwork& net, Trainer& trainer, int cyclesTrained);

	/** Restores the network state state in @ref
	 *  Terminator::validate(). The most important implementor of this
	 *  method is @ref SavingTerminator::restore().
	 *
	 *  @return Value 'true' if an earlier network state was restored,
	 *  'false' if not.
	 *
	 *  @see SavingTerminator
	 **/
	virtual bool	restore				(ANNetwork& net) {return false;}

	/** Returns the current generalization loss (GL), OR calculates
	 *  it for the given parameters, if given.
	 *
	 *  @param min OPTIONAL Lowest validation MSE measured so far.
	 *  @param opt OPTIONAL Optimal validation MSE measured so far. [TODO: check this]
	 **/
	double			generalizationLoss	(double min=-666, double opt=-666) const;

	/** Returns the last measured validation error. */
	double			validationError		() const {return mLastValidError;}

	/** Returns the lowest measured validation error. */
	double			minimumError		() const {return mMinValidError;}

	/** Returns the number of training cycles the network has been
	 *  trained before the minimum validation error point.
	 **/
	int				howManyTrained		() const {return mMinCycle;}

  protected:
	/** Smalled validation error so far. */
	double	mMinValidError;

	/** The number of training cycles at minimum. */
	int		mMinCycle;
	
	/** Last validation error. */
	double	mLastValidError;
	
	/** Validation pattern set. */
	const PatternSource& mValidationSet;
	
	/** Validation interval. */
	int		mStripLength;

	/** */
	virtual bool	check				(const ANNetwork& net, int cyclesTrained) = 0;

};

/** Terminator factory. Manufactures a terminator according to the
 *  model name given in the string. See classes below for descriptions
 *  of method names.
 **/
Terminator* buildTerminator (const String& modelName,
							 const PatternSource& validationset,
							 int validationInterval);

///////////////////////////////////////////////////////////////////////////////

/** A super-fast @ref Terminator that doesn't save the network state.
 *
 *  Name parameter for buildTerminator() to create this object is:
 *  "FT#", where # is a numeric parameter, typically 2. Super-fast
 *  terminator: otherwise equal to the UP-terminator, but this one
 *  doesn't save and at the end restore the network.
 **/
class TerminatorT800 : virtual public Terminator {
  public:

	/** Additional parameter 'hits' gives the number of successive
	 *  raises in the validation error required for the termination.
	 *
	 *  @see Terminator::Terminator
	 **/
					TerminatorT800		(const PatternSource& validationset,
										 int hits, int striplen);

	bool			check				(const ANNetwork& net, int cyclesTrained);

  protected:
	/** Number of successive increases in the validation error. */
	int			mRaises;
	
	/** Number of required successive increases in the validation
	 *  error required for termination.
	 *
	 *  @see mRaises
	 **/
	const int	mMaxRaises;
	
};

/** Dummy terminator that doesn't terminate at all.
 *
 **/
class DummyTerminator : virtual public Terminator {
  public:

					DummyTerminator		(const PatternSource& validationset, int striplen) : Terminator (validationset, striplen) {}
	bool			check				(const ANNetwork& net, int cyclesTrained) {mMinCycle = cyclesTrained; return false;}
};

/** A middle-level @ref Terminator abstraction that supports saving
 *  the best network state so far.
 **/
class SavingTerminator : virtual public Terminator {
  public:
					SavingTerminator	(const PatternSource& validationset, int striplen);
					~SavingTerminator	();

	/** Restores the saved network weight state to the network given
	 *  as parameter.
	 **/
	bool			restore				(ANNetwork& net);

  protected:

	/** Stores the given network state for later restoration.
	 **/
	void			save				(const ANNetwork& net, int cyclesTrained);

  private:
	/** The best network state (lowest validation error) so far. We
	 *  save the state as an ordered weight vector (including the
	 *  biases).
	 **/
	Vector		mBestWeights;
};

// Methods by Lutz Prechelt (prechelt@ira.uka.de). See his article
// "Automatic Early Stopping Using Cross Validation: Quantifying the
// Criteria", 1996.

/** Generalization Loss @ref Terminator by Prechelt. Parameter about 5
 *  recommended for finding a "good" solution. GL1 a little slower
 *  than UP2, GL5 about 1/3 of the speed of UP2.  name="GL#", where #
 *  is a numeric parameter, typically 2-5.
 **/
class GLTerminator : public SavingTerminator {
  public:

	/** Standard constructor.
	 *
	 *  @param validationset Validation set that is used for measuring
	 *  the generalization ability of the neural network taught with
	 *  the separate training data.
	 *
	 *  @param threshold Generalization Loss (GL) threshold value to
	 *  terminate training.
	 *
	 *  @param striplen Interval of training cycles between validation
	 *  and thus early stopping test.
	 **/
					GLTerminator		(const PatternSource& validationset,
										 double threshold, int striplen);
	
	bool			check				(const ANNetwork& net, int cyclesTrained);

  protected:
	/** Generalization loss threshold for stopping.
	 **/
	double	mThreshold;
};

/** Another @ref Terminator used in the examples of Prechelt's
 *  paper. Four parameters needed. First is the GL parameter (typically
 *  2-5), second is a training progress minimum threshold parameter
 *  (typically 0.1), third is a number of strips (typically 8) and
 *  fourth is a GL/P ratio threshold parameter (typically 3). See page
 *  27 of the paper for reference. Example: "PR5,0.1,8,3".
 **/
class PRTerminator : public GLTerminator {
  public:
					PRTerminator		(const PatternSource& validationset, double thrshld,
										 int striplen, const String& pars);
	
	bool			check				(const ANNetwork& net, int cyclesTrained);

  private:
	int		mStrips;
	double	mGLperP;
	int		mRaises, mMaxRaises;
	bool	mGLFulfilled, mUPFulfilled, mGLperPFulfilled;
	double	mK;
	
	double			progress			(const ANNetwork& net) const;
};

/** Progress quotient @ref Terminator by Prechelt.
 *
 *  name="PQ#", where # is a numeric parameter, typically 2-5.
 **/
class PQTerminator : public SavingTerminator {
	double	mThreshold;		// Generalization loss threshold for stopping
  public:
					PQTerminator		(const PatternSource& validationset,
										 double threshold, int striplen);
	
	bool			check				(const ANNetwork& net, int cyclesTrained);
};

/** A @ref Terminator that stops traning when the generalization error
 *  increased in n successive strips. Very fast with parameter value 2.
 *
 *  name="UP#", where # is a numeric parameter, typically 2.
 **/
class UPTerminator : public SavingTerminator, public TerminatorT800 {
  public:
					UPTerminator		(const PatternSource& validationset,
										 int maxraises, int striplen);

	bool			check				(const ANNetwork& net, int cyclesTrained);
};


#endif
