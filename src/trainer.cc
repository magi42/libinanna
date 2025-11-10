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

#include "inanna/trainer.h"
#include "inanna/termination.h"
#include "inanna/patternset.h"

///////////////////////////////////////////////////////////////////////////////
//                     -----           o                                     //
//                       |        ___      _    ___                          //
//                       |   |/\  ___| | |/ \  /   ) |/\                     //
//                       |   |   (   | | |   | |---  |                       //
//                       |   |    \__| | |   |  \__  |                       //
///////////////////////////////////////////////////////////////////////////////

Trainer::Trainer () {
	mTerminatorName     = "GL2";
	mGeneralizationLoss = 0.0;
	mTrained            = 0;
	mTotalTrained       = 0;
	pTrainingObserver   = NULL;
}

/*virtual*/ void Trainer::init (const StringMap& params) {
	INITPARAMS(params, 
			   );
}

/*virtual*/ void Trainer::initTrain (ANNetwork& network) const
{
	// Initialize weights
	network.init (0.5);
}

double Trainer::train (ANNetwork&           network,
					   const PatternSource& trainset,
					   int                  cycles,
					   const PatternSource* validationSet,
					   int                  validationInterval)
{
	ASSERT (trainset.patterns>0);
	ASSERT (cycles>0);

	// Initialize training method
	initTrain (network);

	// Initialize recording
	mTrainingProfile.make (cycles);
	for (int i=0; i<mTrainingProfile.size(); i++)
		mTrainingProfile[i] = 0.0;

	// If validation set is present, build a terminator that monitors it.
	Terminator* arnold=NULL;
	bool ensureValidGTTrain = true;
	if (validationSet) {
		mValidationProfile.make (cycles/validationInterval+1);
		for (int i=0; i<mValidationProfile.size(); i++)
			mValidationProfile[i]=0.0;

		////////////////////////////////////////
		// Determine termination method
		
		String terminator = mTerminatorName;
		
		// Check if we should _not_ check for that validError>trainError
		if (terminator.left(1) == "-") {
			ensureValidGTTrain = false;
			terminator = terminator.mid (1);
		}
		
		// Can't use termination if there are no validation patterns...
		if (validationSet->patterns==0)
			terminator = "none";
		
		// Order a terminator from the factory
		arnold = buildTerminator (terminator, *validationSet, validationInterval);
	}

	////////////////////////////////////////
	// Train and validate
	
	double	trainMSE;
	double	GL			= 0;
	int		validations	= 0;
	bool	terminate	= false;
	for (mTotalTrained=0; mTotalTrained<cycles;) {
		// Train all patterns once
		trainMSE = mTrainingProfile[mTotalTrained] = trainOnce (network, trainset);
		mTotalTrained++;

		// Streamed output
		if (false)
			printf ("Cycle %d MSE=%f\n", mTotalTrained, trainMSE);
		
		// Validate for early stopping
		if (arnold && mTotalTrained>0 && !(mTotalTrained%validationInterval)) {

			// Calculate the validation error for the current network
			// state
			terminate = arnold->validate (network, *this, mTotalTrained);
			mValidationProfile[validations++] = arnold->validationError ();

			// Let the terminator calculate the GL value. Some
			// terminators use this value to determine termination.
			GL = arnold->generalizationLoss ();

			// Do not terminate if the validation error is lower than
			// the training error
			if (ensureValidGTTrain && arnold->validationError() < trainMSE)
				break;
		}

		// Report the cycle to the training observer, if present
		if (pTrainingObserver) {
			pTrainingObserver->cycleTrained (*this, mTotalTrained);

			// The observer has the power to stop training. This is
			// typically a cancel command given interactively by a
			// user.
			if (pTrainingObserver->wantsToStop())
				break;
		}

	}

	// Restore the state with the lowest error on validation set. Do
	// not restore if the validation error is smaller than the training error
	if (arnold && (!ensureValidGTTrain || arnold->minimumError() > trainMSE)) {
		arnold->restore (network);
		mTrained = arnold->howManyTrained ();
	} else
		// No validation, keep the latest state
		mTrained = mTotalTrained;

	//////////////////////////////////////////////////////////
	// Record and clean up some statistics (truncate vectors)
	
	mGeneralizationLoss = GL;
	if (mTrained>0)
		mTrainingProfile.resize (mTrained);
	else
		mTrainingProfile.make (0);
	if (arnold)
		mValidationProfile.resize (validations);
	else
		mValidationProfile.make (0);

	
	// You will now be terminated
	delete arnold;
	
	return trainMSE; // Return final training MSE
}


