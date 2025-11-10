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

#include <magic/mmath.h> 
#include "inanna/patternset.h"
#include "inanna/termination.h"
#include "inanna/annetwork.h"


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                   |                                                       //
//                   |      ___   ___        _    ___                        //
//                   |     /   )  ___| |/\ |/ \  /   ) |/\                   //
//                   |     |---  (   | |   |   | |---  |                     //
//                   |____  \__   \__| |   |   |  \__  |                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

Learner::Learner ()
{
}

/*******************************************************************************
* Trains the network.
*
* MUST BE OVERLOADED if the ANN system uses any training (not necessary if
* it doesn't).
*
* @return Final MSE (mean squared error) at the end of the training.
*
* @param set Training set.
*
* @param cycles Maximum number of training cycles. The training may be
* terminated before this by the @ref Terminator method used.
*
* @param cycint Validation interval for @ref Terminator (early stopping)
* method. Default value -1 disables validation and thus early stopping, and
* training is carried out for the maximum number of training cycles.
*
* @param vsize OPTIONAL Number of patterns in the 'set' used as a
* validation set.
*******************************************************************************/
double Learner::train (const PatternSet& set, int cycles, int cycint, int vsize)
{
	MUST_OVERLOAD;
	return 0.0;
}

/*******************************************************************************
* Trains the network.
*
* Coating for the other train method above; accepts validation set as a
* parameter.
*
* @param trainSet Training set.
*
* @param validationSet Validation set.
*
* @param cycles Maximum number of training cycles. The training may be
* terminated before this by the @ref Terminator method used.
*
* @param cycint Validation interval for @ref Terminator (early stopping)
* method. Default value -1 disables validation and thus early stopping, and
* training is carried out for the maximum number of training cycles.
*******************************************************************************/
double Learner::train (const PatternSet& trainSet,
					   const PatternSet& validationSet,
					   int cycles,
					   int cycint)
{
	NOT_IMPLEMENTED;
	return 0.0;
}

/*******************************************************************************
* Trains the network for one cycle.
*
* MUST BE OVERLOADED if training is desired.
*******************************************************************************/
double Learner::trainOnce (const PatternSet& trainset)
{
	MUST_OVERLOAD;
	return 0.0;
}

/*******************************************************************************
* Tests a specific pattern from a set. Returns a double vector containing
* the output values.
*
* @return Vector containing the values of output units.
*******************************************************************************/
Vector Learner::testPattern (const PatternSource& set, /**< The @ref PatternSet where the pattern is stored. */
							 int pattern               /**< Index number of the pattern in the @ref PatternSet. */) const
{
	MUST_OVERLOAD;
	return Vector(1);
}

/*******************************************************************************
* Tests an entire pattern set.
*
* @return Returns MSE (mean squared error).
*******************************************************************************/
double Learner::test (const PatternSource& set) const
{
	ASSERT (set.patterns>0);
	
	double errorSum = 0.0; // Sum of squared errors (SSE)
	for (int p=0; p<set.patterns; p++) {
		Vector res = testPattern (set, p);
		ASSERT (res.size() == set.outputs);
		
		for (int j=0; j<res.size(); j++)
			errorSum += sqr (res[j] - set.output (p, j));
	}
	
	return errorSum / (set.patterns * set.outputs); // Mean of squared errors (MSE)
}

/*******************************************************************************
* Makes a classification test with the given test set.
*
* If there is just one output unit, it is interpreted to indicate two class
* memberships. In any other case, each output unit designates different
* class.
*
* @return Returns the classification results for each class in a
* @ref ClassifResults struct.
*******************************************************************************/
ClassifResults* Learner::testClassify (const PatternSource& set) const
{
	ASSERT (set.patterns>0);

	ClassifResults* result = new ClassifResults;

	// Determine the number of classes (not so trivial)
	int classes = (set.outputs>1)? set.outputs : 2;
				   
	// Initialize the class hit counter
	result->classcnts.make (classes);
	for (int i=0; i<classes; i++)
		result->classcnts[i] = 0;

	// Initialize the class instance counter
	result->classSizes.make (classes);
	for (int i=0; i<classes; i++)
		result->classSizes[i] = 0;
	
	// Classify each pattern in the set
	int failures=0;
	double errorSum = 0.0; // Sum of squared errors (SSE)
	for (int p=0; p<set.patterns; p++) {
		// Find the correct class 
		int correctClass = set.getClass (p);
		
		// Determine success
		bool success=false;

		Vector res = testPattern (set, p);

		// Record the SSE
		for (int j=0; j<res.size(); j++)
			errorSum += sqr (res[j] - set.output (p, j));

		if (set.outputs==1) {
			if (correctClass == int(res[0]+0.5))
				success = true;
		} else {
			int highestClass = maxIndex (res);
			if (highestClass == correctClass)
				success = true;
		}
		
		// Record the success
		if (!success) {
			failures++;
			
			// For the particular class
			result->classcnts[correctClass]++;
		}

		// And increment the number of instances for this particular class
		result->classSizes[correctClass]++;
	}
	
	// Return the mean
	result->mse = errorSum/(set.patterns*set.outputs); // Mean of squared errors (MSE)
	result->failures = failures;

	return result;
}

/*******************************************************************************
* Copy or conversion from @ref ANNetwork.
*
* Implementing this is essential for the use of @ref Terminator early
* stopping, because transient network states are stored in ANNetwork
* objects during training.
*
*  @param onlyWeights If this flag is 'true', both networks are
*  assumed to have the same topology. Therefore, only weights and
*  activations have to be copied.
*******************************************************************************/
void Learner::copyFreeNet (const ANNetwork& fnet,
						   bool             onlyWeights)
{
	copy (static_cast<const Learner&> (fnet));
}

/*******************************************************************************
* Copy or conversion from any other neural network.
*
* @param fnet Learner to be copied. It is probable that the
* implementor wants to use dynamic_cast to find out the true class of the
* source class.
*
* @param onlyWeights If this flag is 'true', both networks are assumed to
* have the same topology. Therefore, only weights and activations have to
* be copied.
*******************************************************************************/
void Learner::copy (const Learner& fnet,
					bool           onlyWeights)
{
}

/*******************************************************************************
* Conversion to @ref ANNetwork, a highly object-oriented network
* representation.
*******************************************************************************/
ANNetwork* Learner::toANNetwork	() const
{
	MUST_OVERLOAD;
	return NULL;
}


/*******************************************************************************
* @fn void Learner::make (const char* description)
* @brief Creates a network according to the given structure description.
*
* The exact interpretation of the description are dependent on the
* implementor.
**/

/** @fn void Learner::init (double r=-1)
*
* @brief Initializes the network with the given weight range (use default
* if negative).
**/

