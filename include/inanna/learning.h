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

#ifndef __INANNA_LEARNING_H__
#define __INANNA_LEARNING_H__

#include <magic/mobject.h>
#include <magic/mmath.h>
#include <magic/mstring.h>
#include <magic/mpararr.h>

// External
class PatternSource;
class PatternSet;
class ANNetwork;
class Terminator;

/** Struct for returning classification results from @ref
 *  Learner::testClassify.
 **/
struct ClassifResults {

	/** Mean squared error of the test. */
	double			mse;

	/** Total number of classification failures. */
	int				failures;

	/** Number of failures for each class. */
	PackArray<int>	classcnts;

	/** Number of instances of each class in the test set. */
	PackArray<int>	classSizes;
};



/////////////////////////////////////////////////////////////////////////////// 
//                                                                           //
//                _   |   | |   |                           |                //
//               / \  |\  | |\  |  ___   |                  |                //
//              /   \ | \ | | \ | /   ) -+- \    /  __  |/\ | /              //
//              |---| |  \| |  \| |---   |   \\//  /  \ |   |/               //
//              |   | |   | |   |  \__    \   VV   \__/ |   | \              //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

//#define LearningMapping AnyNetwork
//#define Learner         AnyNetwork

/** Abstract interface for any ANN implementations.
 *
 *  This abstraction is intended be generic enough to accomodate every
 *  kind of artificial neural networks. But, since there are currently
 *  just a very few such implementations, this class may not be very
 *  mature.
 *
 *  Slightly bloated.
 **/
class Learner {
  public:

							Learner			();
	virtual					~Learner		() {;}
	
	virtual void			make			(const char* description) = 0;
	virtual void			init			(double r=-1) = 0;

	virtual double			train			(const PatternSet& set, int cycles, int cycint=-1, int vsize=-1);
	double					train			(const PatternSet& trainSet,
											 const PatternSet& validationSet,
											 int cycles,
											 int cycint=-1);
	virtual double			trainOnce		(const PatternSet& trainset);
	virtual Vector			testPattern		(const PatternSource& set, int pattern) const;

	// These should not be overridden usually
	
	virtual double			test			(const PatternSource& set) const;
	virtual ClassifResults*	testClassify	(const PatternSource& set) const;

	// Common conversions

	virtual void			copyFreeNet		(const ANNetwork& fnet, bool onlyWeights = false);
	virtual void			copy			(const Learner& fnet, bool onlyWeights = false);
	virtual ANNetwork*		toANNetwork		() const;
};


#endif

