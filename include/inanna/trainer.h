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

#ifndef __INANNA_TRAINER_H__
#define __INANNA_TRAINER_H__

#include <magic/mparameter.h>
#include "inanna/annetwork.h"

// Local predeclarations
class Trainer;
class TrainingObserver;


///////////////////////////////////////////////////////////////////////////////
//                     -----           o                                     //
//                       |        ___      _    ___                          //
//                       |   |/\  ___| | |/ \  /   ) |/\                     //
//                       |   |   (   | | |   | |---  |                       //
//                       |   |    \__| | |   |  \__  |                       //
///////////////////////////////////////////////////////////////////////////////

/** Abstract baseclass for neural network training algorithms (strategies).
 *
 *  A trainer has three functions - to train network with data, to
 *  store temporary data, and to keep record of the training process.
 *
 *  Design Patterns: Strategy.
 **/
class Trainer : public Object, public IParameterized {
  public:
							Trainer			();

	/** Initialize the algorithm with the given parameters.
	 **/
	virtual void			init			(const StringMap& params);

	/** Train the given network with the given training set.
	 *
	 *  Notice that the trainer should change just the weights of the
	 *  network, not alter the network objects in any way, for example
	 *  to store algorithm-specific data to neurons or
	 *  connections. That data should be stored to the trainer object
	 *  itself.
	 **/
	virtual double			train			(ANNetwork&           network,
											 const PatternSource& trainset,
											 int                  cycles,
											 const PatternSource* pValidationSet=NULL,
											 int                  validationInterval=0);
	
	/** Sets the termination method by name (UP2, GL5, etc).
	 *
	 *  See @ref Terminator, it's inheritors and the global
	 *  buildTerminator() method for more info about the parameters.
	 **/
	void					setTerminator	(const String& name) {mTerminatorName=name;}

	// Informative methods
	
	/** Returns the number of training cycles the network has been
	 *  trained so far.
	 *
	 *  The network may actually have been trained more than this, but
	 *  an earlier weight state may have been restored by a @ref
	 *  Terminator (early stopping method).
	 **/
	int						cyclesTrained	() const {return mTrained;}

	/** Total number of training cycles, including any cycles cut out
	 *  by a @ref Terminator.
	 *
	 *  @see LearningMapping::cyclesTrained
	 **/
	int						totalCycles		() const {return mTotalTrained;}

	/** Returns the percentual error growth between lowest validation
	 *  error and the validation error at the end of the training.
	 **/
	double					generalizLoss	() const {return mGeneralizationLoss;}

	/** This method is used by the @ref Terminator method to store the
	 *  generalization loss value here.
	 **/
	void					setGeneralizLoss(double gl) {mGeneralizationLoss = gl;}

	/** Returns a history record of training set error during
	 *  training. Useful for both statistical analysis of learning
	 *  curves and possibly also some @ref Terminator methods.
	 **/
	const Vector&			trainingRecord	() const {return mTrainingProfile;}
	
	/** Returns a history recording of validation set error during
	 *  training. Useful for both statistical analysis of learning
	 *  curves and possibly also some @ref Terminator methods.
	 **/
	const Vector&			validationRecord() const {return mValidationProfile;}

	/** Sets the observer object for the trainer, to track the the training progress.
	 **/
	void					setObserver		(TrainingObserver* observer) {pTrainingObserver=observer;}
	
  protected:

	/** Initializes training. */
	virtual void			initTrain		(ANNetwork& network) const;

	/** Trains the pattern set once. */
	virtual double			trainOnce		(ANNetwork& network, const PatternSource& set) const {MUST_OVERLOAD; return 0.0;}

  protected:

	/** Name of the current termination method. */
	String	mTerminatorName;

	/** Number of relevant training cycles trained so far. The network
	 *  may have been trained more than this, but an earlier weight
	 *  state may have been restored by a @ref Terminator (early
	 *  stopping method).
	 **/
	int		mTrained;

	/** Total number of training cycles, including any cycles cut out
	 *  by a @ref Terminator.
	 *
	 *  @see LearningMapping::mTrained
	 **/
	int		mTotalTrained;

	/** Current loss of generalization ability of the network in
	 *  respect to the state with the lowest validation error.
	 *
	 *  This value changes during the training if some @ref Terminator
	 *  which uses the GL measurement is used.
	 *
	 *  Generalization loss is calculated by dividing last fitness by
	 *  the best validation error.
	 **/
	double	mGeneralizationLoss;

	/** History record of training set error during training. Useful
	 *  for both statistical analysis of learning curves and possibly
	 *  also some @ref Terminator methods.
	 **/
	Vector	mTrainingProfile;
	
	/** Recording of validation set error during training. Useful for
	 *  both statistical analysis of learning curves and possibly also
	 *  some @ref Terminator methods.
	 **/
	Vector	mValidationProfile;

	/** Observer object that gets called after every cycle.
	 */
	TrainingObserver*	pTrainingObserver;
	
	friend class Terminator;
};



///////////////////////////////////////////////////////////////////////////////////////
// -----           o       o              ___                                        //
//   |        ___      _       _         |   | |      ____  ___             ___      //
//   |   |/\  ___| | |/ \  | |/ \   ___  |   | |---  (     /   ) |/\ |   | /   ) |/\ //
//   |   |   (   | | |   | | |   | (   \ |   | |   )  \__  |---  |    \ /  |---  |   //
//   |   |    \__| | |   | | |   |  ---/ `___´ |__/  ____)  \__  |     V    \__  |   //
//                                  __/                                              //
///////////////////////////////////////////////////////////////////////////////////////

/** Callback observer for monitoring training progress.
 *
 *  Patterns: Observer.
 **/
class TrainingObserver : public Object {
  public:
					TrainingObserver	() {mStop = false;}

	/** Initializes the observer for a training. */
	void			initTraining		() {mStop = false;}

	/** Returns true of the observer wants to stop learning.
	 *
	 *  For observer implementors: to inform the Trainer that the
	 *  observer wants to stop learning, call the method @ref
	 *  stopTraining().
	 **/
	bool			wantsToStop			() const {return mStop;}

	/** Callback method for informing the observer that one cycle has
	 *  been trained.
	 *
	 *  The Trainer::trainOnce() calls this method, after finishing
	 *  with a training cycle.
	 *
	 *  @param trainer The trainer object.
	 *  @param totalCycles Total number of cycles trained so far.
	 **/
	virtual	void	cycleTrained		(const Trainer& trainer, int totalCycles)=0;
	
  protected:
	/** The observer should call this method from the @ref
	 *  cycleTrained() method when it wants to stop training.
	 *
	 *  The training is stopped after the observer returns from the
	 *  @ref cycleTrained() method.
	 **/
	void			stopTraining		() {mStop=true;}
	
  private:
	bool	mStop;
};

#endif
