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

#include <stdio.h>
#include <ctype.h>
#include <magic/mobject.h>
#include <magic/mmath.h>
#include <magic/mmap.h>
#include <magic/mclass.h>
#include "inanna/patternset.h"
#include "inanna/dataformat.h"

// #define rnd(range) (rand()%range)

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//   ----                                 ----                              //
//   |   )  ___   |   |   ___        _   (                     ___   ___    //
//   |---   ___| -+- -+- /   ) |/\ |/ \   ---   __  |   | |/\ |   \ /   )   //
//   |     (   |  |   |  |---  |   |   |     ) /  \ |   | |   |     |---    //
//   |      \__|   \   \  \__  |   |   | ___/  \__/  \__! |    \__/  \__    //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

impl_abstract (PatternSource, {CObject});

PatternSource::PatternSource ()
{
	patterns	= 0;
	inputs		= 0;
	outputs		= 0;
}

/*******************************************************************************
 * Copy constructor.
 *
 * Inheritor's copy constructor should call this.
 ******************************************************************************/
PatternSource::PatternSource (const PatternSource& orig)
{
	patterns	= orig.patterns;
	inputs		= orig.inputs;
	outputs		= orig.outputs;
	mName		= orig.mName;
}

void PatternSource::make1 (int patts, int ins, int outs)
{
	patterns	= patts;
	inputs		= ins;
	outputs		= outs;
}

/*******************************************************************************
 * @fn double PatternSource::input (int p, int i) const
 *
 * Returns the value of input variable i in pattern p.
 ******************************************************************************/

/*******************************************************************************
 * @fn double PatternSource::output (int p, int j) const
 *
 * Returns the value of output variable j in pattern p.
 ******************************************************************************/

/*******************************************************************************
 * Returns the output class index for pattern p (the index of the
 * highest output).
 ******************************************************************************/
int PatternSource::getClass (int p) const {
	if (outputs>1) {
		// Multiple class indicators -> scan trough them to find the highest
		double max=-666;
		int maxI=0;
		for (int i=0; i<outputs; i++)
			if (output(p,i) > max) {
				max = output(p,i);
				maxI = i;
			}
		return maxI;
	} else {
		// Single, binary class indicator
		return (output(p,0)>0.5)? 1:0;
	}
}

/*******************************************************************************
 * @fn void PatternSource::recombine(int startp=-1, int endp=-1)
 *
 * Shuffles the patterns in the set.
 *
 *  @param startp OPTIONAL Lower index of shuffling range.
 *  @param endp OPTIONAL Upper index of shuffling range.
 ******************************************************************************/

/*******************************************************************************
 * @fn void PatternSource::recombine2(int startp=-1, int endp=-1)
 *
 * As @ref recombine above, except that this shuffles so that the even
 * and odd-numbered patterns are not mixed, but preserve their
 * evenness and oddity.
 *
 * Pattern set or the shuffling range (see the parameters below) must
 * have even number of patterns.
 *
 * @param startp OPTIONAL Lower index of shuffling range.
 * @param endp OPTIONAL Upper index of shuffling range.
 ******************************************************************************/

/*******************************************************************************
 * Copies a range from another training set.
 *
 * @param startp OPTIONAL Lower index of copying range.
 * @param endp OPTIONAL Upper index of copying range.
 ******************************************************************************/
void PatternSource::copy (const PatternSource& other, int startp, int endp) {
	ASSERTWITH (startp<other.patterns && endp<other.patterns || endp==startp-1,
				format ("Copy range (%d-%d) out of source set limits (%d patterns)",
						startp, endp, other.patterns));
	ASSERTWITH (startp>=-1, format ("Invalid copy range %d", startp));

	// If the range is empty, do nothing
	if (endp==startp-1)
		return;
	
	ASSERTWITH (startp<=endp, "Start pattern must be greater than end pattern. "
				"Note: end point can be start-1, in which case no copying is done.");

	// Set parameters to default of not given
	if (startp==-1)
		startp = 0;
	if (endp==-1)
		endp = other.patterns-1;

	// Resize self
	make (endp-startp+1, other.inputs, other.outputs);

	// Copy
	for (int p=0; p<=endp-startp; p++) {
		for (int i=0; i<inputs; i++)
			set_input (p, i, other.input (p+startp, i));
		for (int i=0; i<outputs; i++)
			set_output (p, i, other.output (p+startp, i));
	}
}

/*******************************************************************************
 * Creates this set as union of two sets.
 ******************************************************************************/
void PatternSource::join (const PatternSource& a, const PatternSource& b) {
	// First check the possibility that either or both of the sets might be empty
	if (a.patterns==0 || b.patterns==0) {
		// These cases we handle just by copying. For some reason.
		make (0,0,0);
		if (a.patterns>0)
			copy (a);
		if (b.patterns>0)
			copy (b);
		return;
	}

	// Not empty
	
	ASSERTWITH (a.inputs==b.inputs && a.outputs==b.outputs,
				format ("Tsets to be joined must be of equal dimensions (was: %d->%d . %d->%d)",
						a.inputs, a.outputs, b.inputs, b.outputs));
	ASSERTWITH (a.patterns==0 || b.patterns==0 ||
				a.inputs>0 && a.outputs>0 && b.inputs>0 && b.outputs>0,
				"Tsets to be joined may not have null dimension");
	make (a.patterns+b.patterns, a.inputs, a.outputs);

	// Copy a
	for (int p=0; p<a.patterns; p++) {
		for (int i=0; i<inputs; i++)
			set_input (p, i, a.input (p, i));
		for (int i=0; i<outputs; i++)
			set_output (p, i, a.output (p, i));
	}

	// Copy b
	for (int p=0; p<b.patterns; p++) {
		for (int i=0; i<inputs; i++)
			set_input (p+a.patterns, i, b.input (p, i));
		for (int i=0; i<outputs; i++)
			set_output (p+a.patterns, i, b.output (p, i));
	}
}

/*******************************************************************************
 * Splits this set into two subsets a and b according to given ratio.
 ******************************************************************************/
void PatternSource::split (PatternSource& a, PatternSource& b, double ratio) const
{
	ASSERT (ratio>=0 && ratio<=1);
	ASSERT (patterns>0);
	ASSERT (inputs>0);

	a.make (int(patterns*ratio), inputs, outputs);
	b.make (patterns-int(patterns*ratio), inputs, outputs);

	a.copy (*this, 0, int(patterns*ratio)-1);
	b.copy (*this, int(patterns*ratio), patterns-1);

	/*
	ASSERTWITH (a.patterns>0 && b.patterns>0,
				"Resulting pattern set was empty (ratio was maybe too near 0.0 or 1.0,\n"
				"    or the source set was too small?)");
				*/
}

/*******************************************************************************
 * Copies another set by filtering the input variables with bit string
 * mask containing characters "0" and "1". If bits string is null
 * (len==0), all inputs will be taken.
 ******************************************************************************/
void PatternSource::filter (const PatternSource& src, const String& bits)
{
	ASSERTWITH (src.patterns>0 && src.inputs+src.outputs>0,
				"Filter source TSet must have dimensions");
	ASSERTWITH (uint(src.inputs) == bits.length() || bits.length()==0,
				format ("Filter mask must have length (was %d) "
						"equal to source TSet input vector dimension (was %d)",
						bits.length(), src.inputs));
	
	// Count features
	int features=0;
	if (bits.length()==0)
		features = src.inputs;
	else
		for (uint i=0; i<bits.length(); i++)
			if (bits[i] == '1')
				features++;
	
	ASSERTWITH (features>0, "Training set filter may not be empty");

	// Re-make self to this size
	make (src.patterns, features, src.outputs);

	// For each pattern
	for (int p=0; p<patterns; p++) {
		
		// Copy the input features by filtering
		int feature=0;
		for (int i=0; i<src.inputs; i++)
			if (bits.length()==0 || bits[i]=='1')
				set_input (p, feature++, src.input (p, i));
		
		// Copy the outputs
		for (int j=0; j<src.outputs; j++)
			set_output (p, j, src.output (p, j));
	}
}

/*******************************************************************************
 * @fn int PatternSource::classes() const
 *
 * Returns the number of output classes in the set.
 *
 * This assumes that the pattern set is for a classification task. For
 * each class there is one output. But, if there are only two classes,
 * they can be represented by just one output.
 ******************************************************************************/

/*******************************************************************************
 * Returns the number of instances for each class.
 *
 * This works only for patterns sets used in classification tasks
 * where there is one output for each class (or just one output in
 * case of two classes). It is assumed that when a pattern belongs to
 * a certain class with index c, the c:th input variable is set to
 * value >0.5. Otherwise it is set to value <0.5.
 ******************************************************************************/
Array<int> PatternSource::countClasses () const {
	Array<int> result;
	result.make (classes());
	
	for (int c=0; c<result.size(); c++)
		result[c] = 0;
	for (int p=0; p<patterns; p++)
		result[getClass (p)]++;
	return result;
}

void PatternSource::check () const {
	ASSERT (inputs>=0);
	ASSERT (inputs<1000);
	ASSERT (outputs>=0);
	ASSERT (outputs<1000);
	ASSERT (patterns>=0);
	ASSERT (patterns<100000);
}



///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//            ----                                 ----                      //
//            |   )  ___   |   |   ___        _   (      ___   |             //
//            |---   ___| -+- -+- /   ) |/\ |/ \   ---  /   ) -+-            //
//            |     (   |  |   |  |---  |   |   |     ) |---   |             //
//            |      \__|   \   \  \__  |   |   | ___/   \__    \            //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
 * Constructs an empty pattern set of the given dimensions.
 ******************************************************************************/
PatternSet::PatternSet (int patts, /**< Number of patterns                    */
						int ins,   /**< Number of inputs                      */
						int outs   /**< Number of outputs, or 0 if no outputs */)
{
	patterns = patts;
	inputs   = ins;
	outputs  = outs;

	// If we got the size parameters, build the structure here
	if (patts && ins && outs)
		make (patts, ins, outs);
}

/*******************************************************************************
* Constructs and loads a pattern set from a file.
*
* The file must be in SNNS pattern file format. If the dimensions of the
* training set have not been set, load tries to deduce them from the
* file. If the dimensions of the file differ from the values that may have
* been given in the construction, an exception is thrown.
*******************************************************************************/
PatternSet::PatternSet (const String& filename, /**< Name of the pattern file.   */
						int           ins,      /**< Number of input variables.  */
						int           outs      /**< Number of output variables. */)
{
	patterns = 0;
	inputs   = ins;
	outputs  = outs;

	load (filename);
}

/*******************************************************************************
 * Copy constructor.
 ******************************************************************************/
PatternSet::PatternSet (const Matrix& m, int mins, int mouts)
{
	ASSERT (mins+mouts == m.cols || mins==-1);
	if (mins==-1)
		mins = m.cols;
	make (m.rows, mins, mouts);
	for (int r=0; r<m.rows; r++) {
		for (int c=0; c<mins; c++) 
			set_input (r,c,m.get(r,c));
		for (int c=0; c<mouts; c++)
			set_output (r,c+mins,m.get(r,c+mins));
	}
}

/*******************************************************************************
 * Copy constructor.
 ******************************************************************************/
PatternSet::PatternSet (const PatternSet& orig)
		: PatternSource (orig), mInps (orig.mInps), mOutps (orig.mOutps)
{
}

/*******************************************************************************
 * Constructs an empty pattern set with the given dimensions.
 *
 * @param patts Number of patterns in the set
 * @param ins Number of input variables
 * @param outs Number of output variables
 ******************************************************************************/
void PatternSet::make (int patts, /**< Number of patterns                    */
					   int ins,   /**< Number of input variables             */
					   int outs   /**< Number of outputs, or 0 if no outputs */)
{
	// ASSERTWITH (patts>0 && ins>0 && outs>0, "Zero argument not allowed");
	PatternSource::make1 (patts, ins, outs);
	mInps.make (patts, ins);
	mOutps.make (patts, outs);
}

/*******************************************************************************
* Loads a pattern set from a pattern file.
*
* Supports SNNS file format (.pat), Proben1 (.dt), and matrix file format
* (other suffixes). Overloadable to allow implementations of other formats.
*******************************************************************************/
void PatternSet::load (const char* filename)
{
	DataFormatLib::load (filename, *this);
}

/*******************************************************************************
* Loads a pattern set from a pattern input stream.
*
* @param extension File name extension; ".raw",".pat" (SNNS), or ".dt" (Proben1)
*******************************************************************************/
void PatternSet::load (TextIStream& in, const String& extension)
{
	DataFormatLib::load (in, *this, extension);
}

/*******************************************************************************
* Saves the pattern set.
*
* The pattern set is saved with the given output format.
*******************************************************************************/
void PatternSet::save (const String& filename, /**< Name of the pattern file. */
					   int           filetype  /**< One of FT_SNNS (SNNS format, default), FT_RAW (raw format). */)
{
	DataFormatLib::save (filename, *this);
}

/*******************************************************************************
 * Outputs the patternset to a C-style FILE stream.
 ******************************************************************************/
void PatternSet::print (FILE* out) const
{
	if (!out)
		out=stdout;

	for (int p=0; p<mInps.rows; p++) {
		fprintf (out, "# Input pattern %d:\n", p);
		for (int i=0; i<mInps.cols; i++)
			fprintf (out, "%f ", mInps.get (p,i));
		fprintf (out, "\n");
		fprintf (out, "# Output pattern %d:\n", p);
		for (int i=0; i<mOutps.cols; i++)
			fprintf (out, "%f ", mOutps.get (p,i));
		fprintf (out, "\n");
	}
}

/*******************************************************************************
 * Mutates the inputs of the training set by given number of
 * errors. The number of errors equals to the hamming distance between
 * normal and mutated patterns.
 ******************************************************************************/
void PatternSet::mutate (int errcnt) {
	int mutated[errcnt];
	int r,
		clear;

	for (int p=0; p<mInps.rows; p++)
		for (int i=0; i<errcnt; i++) {
			clear = 0;
			while (!clear) {
				r = rnd (mInps.cols)+1;
				clear=1;
				for (int j=0; j<i-1; j++)
					if (mutated[j]==r)
						clear=0;
			}
			mutated[i]		= r;
			mInps.get (p,r)	= 1-mInps.get (p,r);
		}
}

/*******************************************************************************
 * Permutates a set of patterns to a random configuration. If range is
 * specified, only those patterns are reshuffled.
 ******************************************************************************/
void PatternSet::recombine (int startp, int endp) {
	if (startp==-1)
		startp=0;
	if (endp==-1)
		endp=patterns-1;
	
	ASSERTWITH (startp<=endp, "Range error in recombination");

	// For each pattern
	for (int p=0; p<patterns; p++) {

		// Swap with another, random pattern

		int o=rnd (patterns);

		// Swap inputs
		for (int i=0; i<inputs; i++)
			swap (mInps.get (p, i), mInps.get (o, i));

		// Swap outputs
		for (int i=0; i<outputs; i++)
			swap (mOutps.get (p, i), mOutps.get (o, i));
	}
}

/*******************************************************************************
 * As above, except ...
 ******************************************************************************/
void PatternSet::recombine2 (int startp, int endp) {
	if (startp==-1)
		startp=0;
	if (endp==-1)
		endp=patterns-1;
	
	ASSERTWITH (startp<=endp, "Range error in recombination");

	// For each pattern
	for (int p=0; p<patterns; p++) {

		// Swap with another, random pattern

		int odd = p%2;
		int o=0;
		do {
			o=rnd (patterns/2)*2+odd;
		} while (o>=patterns);

		// Swap inputs
		for (int i=0; i<inputs; i++)
			swap (mInps.get (p, i), mInps.get (o, i));

		// Swap outputs
		for (int i=0; i<outputs; i++)
			swap (mOutps.get (p, i), mOutps.get (o, i));
	}
}
	
/*******************************************************************************
 * Returns all the patterns (both input and output variables) as a Matrix.
 ******************************************************************************/
Ref<Matrix> PatternSet::getMatrix () const {
	Ref<Matrix> result = new Matrix (patterns, inputs+outputs);
	for (int p=0; p<result->rows; p++)
		for (int c=0; c<result->cols; c++)
			if (c<inputs)
				result->get(p,c) = input(p,c);
			else
				result->get(p,c) = output(p,c-inputs);
	return result;
}

/*******************************************************************************
 * Returns a reference to the underlying matrix container.
 ******************************************************************************/
Matrix& PatternSet::getInputMatrix ()
{
	return mInps;
}

/*******************************************************************************
 * Implementation for @ref PatternSource. Copies a range from another
 * training set.
 *
 *  @param startp OPTIONAL Lower index of copying range.
 *  @param endp OPTIONAL Upper index of copying range.
 ******************************************************************************/
void PatternSet::copy (const PatternSet& orig, int start, int end) {
	PatternSource::copy (orig, start, end);
}

/*******************************************************************************
 * Checks internal consistency of the object.
 ******************************************************************************/
void PatternSet::check () const {
	PatternSource::check ();
}

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
 * Constructs a subset of a full pattern set.
 *
 * Use the window functions to define the subset.
 ******************************************************************************/
PatternSubset::PatternSubset (const PatternSet& orig)
{
	mpSet    = &orig;
	mOwnSet  = false;
	patterns = mpSet->patterns;
	inputs   = mpSet->inputs;
	outputs  = mpSet->outputs;
	mPatternStart    = 0;
	mPatternInterval = 1;
	mPatternEnd      = mpSet->patterns - 1;
	mInputStart      = 0;
	mInputEnd        = mpSet->inputs -1;
	mOutputStart     = 0;
	mOutputEnd       = mpSet->outputs -1;
	mScanMode        = false;
	mScanOffset      = 0;
	mEndAccessed     = false;
}

/*******************************************************************************
 * Constructs a subset of a full pattern set.
 *
 * This constructor takes ownership of the given full PatternSet
 * object and will destroy it during the destruction of this object.
 *
 * Use the window functions to define the subset.
 ******************************************************************************/
PatternSubset::PatternSubset (const PatternSet* pOrig)
{
	mpSet            = pOrig;
	mOwnSet          = true;
	patterns         = mpSet->patterns;
	inputs           = mpSet->inputs;
	outputs          = mpSet->outputs;
	mPatternStart    = 0;
	mPatternInterval = 1;
	mPatternEnd      = mpSet->patterns - 1;
	mInputStart      = 0;
	mInputEnd        = mpSet->inputs - 1;
	mOutputStart     = 0;
	mOutputEnd       = mpSet->outputs - 1;
	mScanMode        = false;
	mScanOffset      = 0;
	mEndAccessed     = false;
}

PatternSubset::~PatternSubset ()
{
	if (mOwnSet)
		delete mpSet;
}

/*******************************************************************************
 * Sets pattern window.
 *
 * The number of patterns in the window will be (end-start+1)/interval.
 ******************************************************************************/
void PatternSubset::setPatternWindow (
	int start,   /**< First pattern in the window.                         */
	int end,     /**< Last pattern in the window, or -1 for last possible. */
	int interval /**< Interval for sampling patterns.                      */)
{
	if (end == -1)
		end = mpSet->patterns - 1;
	if (! (start >= 0 && end >= start && end < mpSet->patterns))
		throw out_of_bounds (String("Pattern window range [%1,%2] outside range [0,%3].").arg(start).arg(end).arg(mpSet->patterns - 1));

	mPatternStart    = start;
	mPatternEnd      = end;
	mPatternInterval = interval;
	patterns         = (mPatternEnd - mPatternStart) / mPatternInterval + 1;
	mScanOffset      = 0; // Reset

	// Reset
	if (interval == 1)
		setScanning (false);
	else
		setScanning (mScanMode);
}

/*******************************************************************************
 * Sets window for the input variables.
 *
 * The range must be within the input range of the full set.
 ******************************************************************************/
void PatternSubset::setInputWindow (
	int start, /**< First input variable in the full set to be included. */
	int end    /**< Last input variable in the full set to be included, or -1 for last possible. */)
{
	if (end == -1)
		end = mpSet->inputs-1;
	if (! (start >= 0 && end >= start && end < mpSet->inputs))
		throw out_of_bounds (String("Input variable window range [%1,%2] outside range [0,%3].").arg(start).arg(end).arg(mpSet->inputs));

	mInputStart = start;
	mInputEnd   = end;
	inputs      = end - start + 1;
}

/*******************************************************************************
 * Sets window for the input variables.
 ******************************************************************************/
void PatternSubset::setOutputWindow (
	int start, /**< First output variable in the full set to be included. */
	int end    /**< Last output variable in the full set to be included, or -1 for last possible. */)
{
	if (end == -1)
		end = mpSet->outputs-1;

	if (! (start >= 0 && end >= start && end < (mAutoassociation? mpSet->inputs : mpSet->outputs)))
		throw out_of_bounds (String("Output variable window range [%1,%2] outside range [0,%3].").arg(start).arg(end).arg(mAutoassociation? mpSet->inputs : mpSet->outputs));

	mOutputStart = start;
	mOutputEnd   = end;
	outputs      = end - start + 1;
}

/*******************************************************************************
 * Sets scanning mode for interval windows.
 *
 * When the scanning mode is used, the interval set with
 * setPatternWindow will be offset between subsequent iterations of
 * the set. The offset starts from 0, and is increased by 1
 * automatically every time the first pattern in the window is
 * accessed after accessing the last pattern.
 *
 * Notice that if the interval defined in setPatternWindow does not
 * divide the full set evenly, the last (uneven) pattern range will be
 * ignored. In such case, calling this method will reduce the size of
 * the window to make the division even, and therefore also reduce the
 * number of available patterns accordingly by one.
 ******************************************************************************/
void PatternSubset::setScanning (bool mode)
{
	mScanMode     = mode;
	mScanOffset   = 0;
	mEndAccessed  = false;

	// If the interval does not divide the full set evenly, reduce the
	// number of patterns by one to allow even scanning to the last
	// pattern with all offsets.
	int extra = (mPatternEnd - mPatternStart + 1) % mPatternInterval;
	if (extra != 0)
		mPatternEnd -= extra; // Discard patterns that cause unevenness

	// Recalculate the number of patterns in the window
	patterns = (mPatternEnd - mPatternStart) / mPatternInterval + 1;
}

/*******************************************************************************
 * Sets autoassociation mode.
 *
 * In autoassociation mode, inputs are automatically copied to
 * outputs. Notice that the input variable window will automatically
 * be applied to output variables as well, and there is currently no
 * way to avoid this.
 ******************************************************************************/
void PatternSubset::setAutoassociation (bool mode)
{
	mAutoassociation = mode;
	outputs = inputs;
}


/*******************************************************************************
 * @fn const PatternSet& PatternSubset::contained() const
 *
 * Returns the entire pattern set object.
 ******************************************************************************/

/*******************************************************************************
 * Returns the corresponding pattern position in the full pattern set.
 ******************************************************************************/
int PatternSubset::realPattern (int p) const
{
	return mPatternStart + p*mPatternInterval + mScanOffset;
}

/** Implementation for PatternSource. */
void PatternSubset::print (FILE* out) const
{
}

double PatternSubset::input (int p, int i) const
{
	if (mScanMode) {
		PatternSubset& ncthis = const_cast <PatternSubset&>(*this);
		if (!mEndAccessed && p == mPatternEnd)
			ncthis.mEndAccessed = true;
		else
			if (mEndAccessed && p == 0) {
				ncthis.mEndAccessed = false;
				if (++ncthis.mScanOffset >= mPatternInterval)
					ncthis.mScanOffset = 0;
			}
	}
	
	return mpSet->input (mPatternStart + p*mPatternInterval + mScanOffset, mInputStart + i);
}

double PatternSubset::output (int p, int j) const
{
	if (mAutoassociation)
		return input (p, j);

	if (mScanMode) {
		PatternSubset& ncthis = const_cast <PatternSubset&>(*this);
		if (!mEndAccessed && p == mPatternEnd)
			ncthis.mEndAccessed = true;
		else
			if (mEndAccessed && p == 0) {
				ncthis.mEndAccessed = false;
				if (++ncthis.mScanOffset >= mPatternInterval)
					ncthis.mScanOffset = 0;
			}
	}
	
	return mpSet->input (mPatternStart + p*mPatternInterval + mScanOffset, mInputStart + j);
}

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
 * Standard constructor.
 *
 *  @param inputs Number of input variables.
 *  @param outputs Number of output variables.
 *  @param filename Filename of the file that contains the series.
 ******************************************************************************/
ArrayTrainSet::ArrayTrainSet (int ins, int outs, char* filename)
{
	PatternSource::make1 (0, ins, outs);

	FILE* fin = fopen (filename, "r");
	if (!fin)
		throw new runtime_error ("file not found");

	while (!feof (fin)) {
		double	x;
		fscanf (fin, "%lf", &x);
		data.add (new double (x));
	}
	fclose (fin);

	patterns = data.size()-inputs-outputs;
}

void ArrayTrainSet::print (FILE* out) const {
/*	out	<< data.upper << " data items\n"
		<< patterns	<< " patterns\n"
		<< inputs	<< " inputs\n"
		<< outputs	<< " outputs\n";
	out << "t = {";
*/
	for (int i=0; i<data.size(); i++) {
		fprintf (out, "%f ", data[i]);
	}
	fprintf (out, "\n");
}

/*******************************************************************************
 * Scales all input and output values with the multiplier k.
 ******************************************************************************/
void ArrayTrainSet::multiply (double k) {
	for (int i=0; i<data.size(); i++)
		data[i] *= k;
}

/*******************************************************************************
 * Resizes the dataset to the given size.
 ******************************************************************************/
void ArrayTrainSet::truncate (int newsize) {
	PRE (newsize<data.size() && newsize>=(inputs+outputs));
	data.resize (newsize);
	patterns = data.size() - inputs - outputs;
}
