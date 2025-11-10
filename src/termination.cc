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

#include "inanna/termination.h"
#include "inanna/annetwork.h"
#include "inanna/patternset.h"
#include "inanna/trainer.h"

Terminator* buildTerminator (const String& modelName,
							 const PatternSource& validationset,
							 int interval)
{
	if (modelName == "none")
		return NULL;
	if (modelName == "dummy")
		return new DummyTerminator (validationset, interval);
		
	ASSERT (modelName.length()>=3);
	String	prefix = modelName.left(2);
	int		intParam = modelName.mid (2).toInt();
	double	doubleParam = modelName.mid (2).toDouble();
	ASSERT (intParam>0);
	ASSERT (doubleParam>0.0);
	
	if (prefix=="FT")
		return new TerminatorT800 (validationset, intParam, interval);
	if (prefix=="GL")
		return new GLTerminator (validationset, doubleParam, interval);
	if (prefix=="PQ")
		return new PQTerminator (validationset, doubleParam, interval);
	if (prefix=="UP")
		return new UPTerminator (validationset, intParam, interval);
	if (prefix=="PR")
		return new PRTerminator (validationset, doubleParam, interval, modelName.mid(2));
	
	throw generic_exception (format ("Invalid terminator model name '%s'",
									 (CONSTR) modelName));
}


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//      -----                 o         |   |           |             |      //
//        |    ___                _     |\ /|  ___   |  | _           |      //
//        |   /   ) |/\ |/|/| | |/ \    | V | /   ) -+- |/ |  __   ---|      //
//        |   |---  |   | | | | |   |   | | | |---   |  |  | /  \ (   |      //
//        |    \__  |   | | | | |   | O |   |  \__    \ |  | \__/  ---|      //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

Terminator::Terminator (const PatternSource& vset, int striplen)
		: mValidationSet (vset), mStripLength (striplen) {
	mMinValidError = 666;
	mLastValidError = 666;
	mMinCycle = -1;
}

double Terminator::generalizationLoss (double last, double opt) const {
	if (last==-666 || opt==-666)
		return 100*((mLastValidError/mMinValidError) - 1);
	else
		return 100*((last/opt)-1);
}

bool Terminator::validate (const ANNetwork& net, Trainer& trainer, int cyclesTrained) {
	mLastValidError = net.test (mValidationSet);
	trainer.setGeneralizLoss (generalizationLoss());
	return check (net, cyclesTrained);
}



///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//           -----                 -----                 o                   //
//           |      ___   ____  |    |    ___                _               //
//           |---   ___| (     -+-   |   /   ) |/\ |/|/| | |/ \              //
//           |     (   |  \__   |    |   |---  |   | | | | |   |             //
//           |      \__| ____)   \   |    \__  |   | | | | |   | O           //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

TerminatorT800::TerminatorT800 (const PatternSource& vset, int hits, int striplen)
		: Terminator (vset, striplen), mMaxRaises (hits)
{
}
	
bool TerminatorT800::check (const ANNetwork& net, int cyclesTrained) {
	if (mLastValidError<=mMinValidError) {
		mMinValidError = mLastValidError;
		mRaises = 0;
	} else
		mRaises++;
	
	return (mRaises>=mMaxRaises);
}



///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//       ----             o             -----                 o              //
//      (      ___            _           |    ___                _          //
//       ---   ___| |   | | |/ \   ___    |   /   ) |/\ |/|/| | |/ \         //
//          ) (   |  \ /  | |   | (   \   |   |---  |   | | | | |   |        //
//      ___/   \__|   V   | |   |  ---/   |    \__  |   | | | | |   | O      //
//                                 __/                                       //
///////////////////////////////////////////////////////////////////////////////

SavingTerminator::SavingTerminator (const PatternSource& validationset, int striplen)
		: Terminator (validationset, striplen)
{
}

SavingTerminator::~SavingTerminator () {
}

void SavingTerminator::save (const ANNetwork& network, int cyclesTrained) {
	// On the first call, create storage for weights and biases
	if (mBestWeights.size()==0) {
		// Count the total number of connections in the entire network
		int connections=0;
		for (int i=0; i<network.size(); i++)
			connections += network[i].incomings();
		
		mBestWeights.make (connections + network.size());
	}

	// Copy weights and biases to the storate
	for (register int j=network.size()-1, ji=0; j>=0; j--)
		for (register int i=-1; i<network[j].incomings(); i++, ji++)
			if (i==-1) // Bias
				mBestWeights[ji] = network[j].bias ();
			else // Weight
				mBestWeights[ji] = network[j].incoming(i).weight();
	
	mMinCycle = cyclesTrained;
}

bool SavingTerminator::restore (ANNetwork& network) {
	// Restore weights and biases from the storage
	if (mBestWeights.size()>0)
		for (register int j=network.size()-1, ji=0; j>=0; j--)
			for (register int i=-1; i<network[j].incomings(); i++, ji++)
				if (i==-1) // Bias
					network[j].setBias (mBestWeights[ji]);
				else // Weight
					network[j].incoming(i).setWeight (mBestWeights[ji]);

	return true;
}



//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//      ---- |     -----                 o                 o                //
//     |     |       |    ___                _    ___   |           _       //
//     | --- |       |   /   ) |/\ |/|/| | |/ \   ___| -+- |  __  |/ \      //
//     |   \ |       |   |---  |   | | | | |   | (   |  |  | /  \ |   |     //
//     |___/ |____   |    \__  |   | | | | |   |  \__|   \ | \__/ |   |     //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
*
*******************************************************************************/
GLTerminator::GLTerminator (
	const PatternSource& validationset,
	double            threshold,
	int               striplen)
		: Terminator (validationset, striplen),
		  SavingTerminator (validationset, striplen),
		  mThreshold (threshold) {
}

bool GLTerminator::check (
	const ANNetwork& net,
	int              cyclesTrained)
{
	if (mLastValidError<=mMinValidError) {
		mMinValidError = mLastValidError;
		save (net, cyclesTrained);
	}
	
	//TRACE3 ("Valid-MSE=%f, GL=%f, threshold=%f",
	//		mLastValidError, generalizationLoss(), mThreshold);
	return generalizationLoss() >= mThreshold;
}

/*******************************************************************************
*
*******************************************************************************/
PRTerminator::PRTerminator (
	const PatternSource& validationset,
	double            threshold,
	int               striplen,
	const String&     desc)
		: Terminator (validationset, striplen),
		  GLTerminator (validationset, threshold, striplen)
{
	mThreshold       = threshold;
	mGLperP          = 3.0;
	mK               = 5;
	mStripLength     = 5;
	mMaxRaises       = 8;

	// States
	mGLFulfilled     = false;
	mUPFulfilled     = false;
	mGLperPFulfilled = false;
	mRaises          = 0;
}

double PRTerminator::progress (const ANNetwork& net) const
{
#ifdef CMP_WARNINGS
#warning "TODO: Convert to Trainer"
#endif
	/*
	int endpos=net.cyclesTrained;
	double sum=0, min=1E30;
	const Vector& trainMSEs=net.trainingRecord();
	for (int i=endpos-mStripLength; i<endpos; i++) {
	 	sum += trainMSEs[i];
		if (trainMSEs[i]<min)
		 	min = trainMSEs[i];
	}
	return 1000*(sum/(mK*min)-1);
	*/
	return 0.0;
}

bool PRTerminator::check (const ANNetwork& net, int cyclesTrained)
{
	bool terminate = false;
	
	if (mLastValidError<=mMinValidError) {
		mMinValidError = mLastValidError;
		mRaises = 0;
		save (net, cyclesTrained);
	} else
		mRaises++;

	if (progress (net) < 0.1)
		terminate = true;

	// GL-criterion
	if (generalizationLoss() >= mThreshold)
		mGLFulfilled = true;

	// UP-criterion
	if (mRaises>=mMaxRaises)
		mUPFulfilled = true;

	// Progress ratio criterion
	if (generalizationLoss()/progress(net) > mGLperP)
		mGLperPFulfilled = true;

	if (mGLFulfilled && mUPFulfilled && mGLperPFulfilled)
		terminate = true;

	return terminate;
}



//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//     ----   ___  -----                 o                 o                //
//     |   ) |   |   |    ___                _    ___   |           _       //
//     |---  |   |   |   /   ) |/\ |/|/| | |/ \   ___| -+- |  __  |/ \      //
//     |     | \ |   |   |---  |   | | | | |   | (   |  |  | /  \ |   |     //
//     |     `__X´   |    \__  |   | | | | |   |  \__|   \ | \__/ |   |     //
//               \                                                          //
//////////////////////////////////////////////////////////////////////////////

PQTerminator::PQTerminator (
	const PatternSource& validationset,
	double            threshold,
	int               striplen)
		: Terminator (validationset, striplen),
		  SavingTerminator (validationset, striplen),
		  mThreshold (threshold)
{
}

bool PQTerminator::check (const ANNetwork& net, int cyclesTrained)
{
	double GL = generalizationLoss ();
	return GL>=mThreshold;
}



//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//     |   | ----  -----                 o                 o                //
//     |   | |   )   |    ___                _    ___   |           _       //
//     |   | |---    |   /   ) |/\ |/|/| | |/ \   ___| -+- |  __  |/ \      //
//     |   | |       |   |---  |   | | | | |   | (   |  |  | /  \ |   |     //
//     `___´ |       |    \__  |   | | | | |   |  \__|   \ | \__/ |   |     //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

UPTerminator::UPTerminator (
	const PatternSource& validationset,
	int                  maxraises,
	int                  striplen)
		: Terminator (validationset, striplen),
		  SavingTerminator (validationset, striplen),
		  TerminatorT800 (validationset, maxraises, striplen)
{
}

bool UPTerminator::check (const ANNetwork& net, int cyclesTrained)
{
	if (mLastValidError<=mMinValidError) {
		mMinValidError = mLastValidError;
		mRaises = 0;
		save (net, cyclesTrained);
	} else
		mRaises++;
	
	return (mRaises>=mMaxRaises);
}
