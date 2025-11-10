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

#ifndef __INANNA_PATTERNSET_H__
#define __INANNA_PATTERNSET_H__

#include <magic/mobject.h>
#include <magic/mmath.h>
#include <magic/mmatrix.h>
#include <magic/mattribute.h>
#include <magic/mpararr.h>


//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//   ----                                 ----                              //
//   |   )  ___   |   |   ___        _   (                     ___   ___    //
//   |---   ___| -+- -+- /   ) |/\ |/ \   ---   __  |   | |/\ |   \ /   )   //
//   |     (   |  |   |  |---  |   |   |     ) /  \ |   | |   |     |---    //
//   |      \__|   \   \  \__  |   |   | ___/  \__/  \__! |    \__/  \__    //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

/** The abstract base class for pattern sources such are PatternSet.
 **/
class PatternSource : public Object, public Attributed {
	decl_dynamic (PatternSource);
  public:
					PatternSource		();

	/** Prints the contents of the set to the given stream. */
	virtual	void	print			(FILE* out = stdout) const {MUST_OVERLOAD}

	virtual	double	input			(int p, int i) const {MUST_OVERLOAD; return 0.0;}
	virtual	double	output			(int p, int j) const {MUST_OVERLOAD; return 0.0;}
	virtual void	set_input		(int p, int i, double value) {MUST_OVERLOAD;}
	virtual void	set_output		(int p, int j, double value) {MUST_OVERLOAD;}
	virtual int		getClass		(int p) const;
	virtual void	recombine		(int startp=-1, int endp=-1) {MUST_OVERLOAD;}
	virtual void	recombine2		(int startp=-1, int endp=-1) {MUST_OVERLOAD;}
	
	// Common operations
	
	virtual void	copy			(const PatternSource& other, int startp=-1, int endp=-1);

	/** Sugar for the @ref copy operation. */
	void			operator=		(const PatternSource& other) {copy (other);}

	/** Returns the name of the set. */
	const String&	name			() const {return mName;}

	/** Sets the name of the set. */
	void			setName			(const String& name) {mName=name;}

	void			join			(const PatternSource& a, const PatternSource& b);
	void			split			(PatternSource& a, PatternSource& b, double ratio) const;
	void			filter			(const PatternSource& source, const String& bits);

	int				classes			() const {return (outputs==1)? 2:outputs;}

	Array<int>		countClasses	() const;

	/** Implementation for @ref Object. */
	void			check			() const;

	/** Number of input variables in the patterns. */
	int	inputs;

	/** Number of output variables in the patterns. */
	int	outputs;

	/** Number of patterns in the pattern set. */
	int	patterns;

	/** Pattern set flags. */
	enum psetflags {INPUTS=1, OUTPUTS=2};

  protected:
	/** A name for the pattern set. */
	String	mName;

  protected:
					PatternSource		(const PatternSource& orig);

	/** Initializes a training set with given number of input and output
	 *  values and patterns.
	 **/
	virtual void	make		(int patterns, int inputs, int outputs) {MUST_OVERLOAD}
	void			make1		(int patterns, int inputs, int outputs);
	
};

#define TrainingSet PatternSource



///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//            ----                                 ----                      //
//            |   )  ___   |   |   ___        _   (      ___   |             //
//            |---   ___| -+- -+- /   ) |/\ |/ \   ---  /   ) -+-            //
//            |     (   |  |   |  |---  |   |   |     ) |---   |             //
//            |      \__|   \   \  \__  |   |   | ___/   \__    \            //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

/** Typical, tabular pattern set.
 **/
class PatternSet : public PatternSource {
  public:
	enum filetypes {FT_SNNS=0, FT_RAW=1};

						PatternSet	() {}
						PatternSet	(int patts, int ins, int outs);
						PatternSet	(const String& fname, int ins=0, int outs=0);
						PatternSet	(const Matrix& m, int ins=-1, int outs=0);
						PatternSet	(const PatternSet& orig);
						~PatternSet	() {}

	void				make		(int patts=0, int ins=0, int outs=0);
	virtual void		load		(const char* filename);
	virtual void		load		(TextIStream& in, const String& extension=".raw");
	void				save		(const String& filename, int filetype=FT_SNNS);
	
	// Virtual method implementations

	virtual void		print			(FILE* out = stdout) const;
	virtual double		input			(int p, int i) const {return mInps.get (p,i);}
	virtual double		output			(int p, int j) const {return mOutps.get (p,j);}
	virtual void		set_input		(int p, int i, double value) {mInps.get (p,i) = value;}
	virtual void		set_output		(int p, int j, double value) {mOutps.get (p,j) = value;}

	Ref<Matrix>			getMatrix		() const;
	Matrix&				getInputMatrix	();

	void				mutate			(int errcnt);
	void				recombine		(int startp=-1, int endp=-1);
	void				recombine2		(int startp=-1, int endp=-1);
	virtual void		copy			(const PatternSet& orig, int start=-1, int end=-1);
	void				check			() const;

  protected:
	Matrix			mInps;	/** Input patterns */
	Matrix			mOutps;	/** Output patterns */

	/** Optional comments in patterns. */
	//Array<String>*	mComments;

	/** Calculates the averages of each input and output field. */
	void			calc_means		();

	/** Calculates the standard deviations of each input and output field. */
	void			calc_stddevs	();

  private:
	void operator= (const PatternSet& orig) {FORBIDDEN}
};



///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//   ----                                 ----                               //
//   |   )  ___   |   |   ___        _   (           |      ____  ___   |    //
//   |---   ___| -+- -+- /   ) |/\ |/ \   ---  |   | |---  (     /   ) -+-   //
//   |     (   |  |   |  |---  |   |   |     ) |   | |   )  \__  |---   |    //
//   |      \__|   \   \  \__  |   |   | ___/   \__! |__/  ____)  \__    \   //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
 * Dynamic subset of a pattern set.
 *
 * This class allows easy dynamic reduction of a full pattern set to
 * fewer patterns and/or variables.
 *
 * Using the @ref setScanning() method together with an interval given
 * in @ref setPatternWindow() allows accessing the entire full pattern
 * set during subsequent scans of the patterns. This allows using the
 * entire set, while making it look like a smaller set to the learning
 * algorithm. This is especially useful for batch learning.
 ******************************************************************************/
class PatternSubset : public PatternSource {
  public:
	PatternSubset	(const PatternSet& orig);
	PatternSubset	(const PatternSet* pOrig);
	~PatternSubset	();

	void	setPatternWindow	(int start, int end, int interval=1);
	void	setInputWindow		(int start, int end);
	void	setOutputWindow		(int start, int end);
	void	setScanning			(bool mode=true);
	void	setAutoassociation	(bool mode=true);

	inline const PatternSet&	contained	() const {return *mpSet;}
	int		realPattern			(int p) const;

	// Virtual method implementations
	virtual void	print			(FILE* out = stdout) const;
	virtual double	input			(int p, int i) const;
	virtual double	output			(int p, int j) const;

  protected:
	const PatternSet*	mpSet;
	bool		mOwnSet;
	int			mPatternStart;
	int			mPatternInterval;
	int			mPatternEnd;
	int			mInputStart;
	int			mInputEnd;
	int			mOutputStart;
	int			mOutputEnd;
	
	bool		mScanMode;
	int			mScanOffset;
	bool		mEndAccessed;
	bool		mAutoassociation;
};

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//       _                       -----           o        ----               //
//      / \           ___          |        ___      _   (      ___   |      //
//     /   \ |/\ |/\  ___| \   |   |   |/\  ___| | |/ \   ---  /   ) -+-     //
//     |---| |   |   (   |  \  |   |   |   (   | | |   |     ) |---   |      //
//     |   | |   |    \__|   \_/   |   |    \__| | |   | ___/   \__    \     //
//                          \_/                                              //
///////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
* Data series training set, especially useful for time series. This
 *  class was made to make it possible to use plain data sets as
 *  training sets without the need to first convert their source files
 *  to classical training set files. The class simply reads the set
 *  from file to an array, and then calculates the values for input
 *  and output requests relatively. It also calculates the number of
 *  training patterns from the size of the set and sizes of inputs and
 *  outputs.
 **/
class ArrayTrainSet : public PatternSource {
  public:

					ArrayTrainSet	(int inputs, int outputs, char* filename);
					~ArrayTrainSet	() {;}

	// Virtual method implementations
	
	virtual void	print			(FILE* out = stdout) const;
	virtual double	input			(int p, int i) const {return data[p+i];}
	virtual double	output			(int p, int j) const {return data[p+inputs+j];}

	// New methods
	
	void			multiply		(double k);

	void			truncate		(int size);

   private:
	/** FORBIDDEN */
	virtual void	make			(int patterns, int inputs, int outputs) {FORBIDDEN;}
	 
  public:
	 /** Here all the patterns are represented in the same array. First
	  *  pattern is at data[0..ins-1], second at data[1..ins-0], third
	  *  at data[2..ins+1], etc.
	  **/
	 Array<double>	data;

 };

#endif
