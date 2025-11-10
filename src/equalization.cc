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

#include "inanna/equalization.h"
#include "magic/mclass.h"

impl_dynamic (Equalizer, {Object});
impl_dynamic (HistogramEq, {Equalizer});
impl_dynamic (GaussianEq, {Equalizer});
impl_dynamic (MinmaxEq, {Equalizer});
impl_dynamic (MatrixEqualizer, {Equalizer});

// Testing XML flagging
int xmlflag = ios::xalloc();


//////////////////////////////////////////////////////////////////////////////
//                 -----                  | o                               //
//                 |                 ___  |   ___  ___                      //
//                 |---   __  |   |  ___| | |   / /   ) |/\                 //
//                 |     /  \ |   | (   | | |  /  |---  |                   //
//                 |____ \__|  \__!  \__| | | /__  \__  |                   //
//                          |                                               //
//////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
 * Analyzes the vector and calculates equalization parameters for
 * it. Equalization is applied by the equalize() method below.
 *
 * @param vec Vector to be analyzed for equalization.
 *
 * @param additive Should the analysis be additive? If true, all
 * data is analyzed in accumulating manner. Additive analysis is
 * identical to concatenating multiple vectors and then calling
 * analyze(vec,false).
 *
 * @warning Must be overloaded by any inheritor.
 ******************************************************************************/
void Equalizer::analyze (const Vector& vec, bool additive)
{
	MUST_OVERLOAD;
}

/*******************************************************************************
 * Applies equalization to the given vector.
 *
 * Note for implementors: handle missing parameters, if enabled by
 * mHandleMissing setting.
 *
 * @warning Must be overloaded by any inheritor.
 ******************************************************************************/
void Equalizer::equalize (Vector& vec) const
{
	MUST_OVERLOAD;
}

/*******************************************************************************
 * Applies equalization reversely to the given vector.
 *
 * @warning Must be overloaded by any inheritor.
 ******************************************************************************/
void Equalizer::unequalize (Vector& vec) const
{
	MUST_OVERLOAD;
}

/*******************************************************************************
 * Standard clone operator, which must be implemented by
 * inheritors. It is required by the MatrixEqualizer to create copies
 * of equalizer objects.
 *
 * @warning Must be overloaded by any inheritor.
 ******************************************************************************/
Equalizer* Equalizer::clone () const
{
	MUST_OVERLOAD;
	return NULL;
}

/*******************************************************************************
 * Sets whether or not the equalization should handle missing values
 * in a standard manner.
 ******************************************************************************/
void Equalizer::handleMissing (bool enable)
{
	mHandleMissing=enable;
}


/*******************************************************************************
 * Implementation of serialization.
 *
 * @warning Must be overloaded by any inheritor.
 ******************************************************************************/
TextOStream& Equalizer::operator>> (TextOStream&) const
{
	MUST_OVERLOAD;
	return sout;
}


///////////////////////////////////////////////////////////////////////////////
//          |   | o                                      -----               //
//          |   |    ____  |                  ___        |                   //
//          |---| | (     -+-  __   ___  |/\  ___| |/|/| |---   __           //
//          |   | |  \__   |  /  \ (   \ |   (   | | | | |     /  \          //
//          |   | | ____)   \ \__/  ---/ |    \__| | | | |____ \__|          //
//                                  __/                           |          //
///////////////////////////////////////////////////////////////////////////////

HistogramEq::HistogramEq (int precision, double floor, double ceiling) {
	mHistogram.make (precision);
	mMin = floor;
	mMax = ceiling;
	mLowBound = 1E30;
	mUpBound = -1E30;
}

HistogramEq::HistogramEq (const HistogramEq& orig) : Equalizer (orig) {
	mHistogram.make (orig.mHistogram.size());
	mMin = orig.mMin;
	mMax = orig.mMax;
	mLowBound = orig.mLowBound;
	mUpBound = orig.mUpBound;
}

int HistogramEq::determineDomain (float x, int precision, float lowBound, float upBound) {

	if (upBound-lowBound == 0)
		return 0;

	// Underflow: the value may not be in range if it's from another set
	if (x < lowBound)
		return 0;

	// Overflow
	if (x > upBound)
		return precision-1;

	// If in range
	return int (((x-lowBound)/(upBound-lowBound))*(precision-1));
}

void HistogramEq::analyze (const Vector& vec, bool additive)
{
	if (additive) {
		// It is not possible to directly implement additive analysis with
		// histogram equalization. Therefore, we have to store all data
		// and perform the actual equalization later, when the equalize()
		// method is called for the first time.
		
		mData.add (new Vector (vec));
	} else {
		// If the analysis is non-additive, we analyze immediately.
		analyze2 (vec);
	}
}

void HistogramEq::finalize ()
{
	// Calculate the total number of values in stored data.
	int totlen=0;
	for (int i=0; i<mData.size(); i++)
		totlen += mData[i].size();
	
	// Copy the accumulated data to an analysis vector
	Vector analvec (totlen);
	for (int i=0, t=0; i<mData.size(); i++)
		for (int j=0; j<mData[i].size(); j++, t++)
			analvec[t] = mData[i][j];
	
	// We do not need the data any longer, so we can destroy it now.
	mData.make (0);
	
	// Analyze the data
	analyze2 (analvec);
}

void HistogramEq::analyze2 (const Vector& vec)
{
	mLowBound = min (vec);
	mUpBound = max (vec);

	// Initialize histogram
	for (int i=0; i<mHistogram.size(); i++)
	 	mHistogram[i] = 0.0;
	
	// Make histogram
	for (int i=0; i<vec.size(); i++)
		if (!is_undef (vec[i]))
			mHistogram [determineDomain (vec[i], mHistogram.size(), mLowBound, mUpBound)] += 1;
	
	// Make cumulative histogram
	float sum = 0.0;
	float lastsum = 0.0;
	int lastpos=0;
	for (int i=0; i<mHistogram.size(); i++)
		if (mHistogram[i]>0.0) {
			sum += mHistogram[i];
			if (i>0) {
				for (int j=lastpos+1; j<=i; j++)
					mHistogram[j] = lastsum+(j-lastpos)/(i-lastpos+0.0)*(sum-lastsum);
				lastsum=sum;
			} else {
				mHistogram[i] = 0.0;
				sum=0.0;
			}

			lastpos=i;
		}

	// Put actual target values in the histogram
	for (int i=0; i<mHistogram.size(); i++)
		mHistogram[i] = mHistogram[i] / mHistogram[mHistogram.size()-1];
}

/*virtual*/ void HistogramEq::equalize (Vector& vec) const {
	// If additive analysis has been used, the analysis must be
	// finalized.
	if (mData.size()>0)
		const_cast<HistogramEq&>(*this).finalize ();
	ASSERTWITH (mHistogram.size(), "Histogram equalizer has not analyzed any data.");
	
	// Equalize each value in the vector
	for (int i=0; i<vec.size(); i++)
		if (!is_undef (vec[i]))
			vec[i] = mMin+(mMax-mMin) * mHistogram[determineDomain (vec[i], mHistogram.size(), mLowBound, mUpBound)];
		else if (mHandleMissing)
			vec[i] = (mMax+mMin)/2;
}

/*virtual*/ void HistogramEq::unequalize (Vector& vec) const {
	// If additive analysis has been used, the analysis must be
	// finalized.
	if (mData.size()>0)
		const_cast<HistogramEq&>(*this).finalize();
	ASSERTWITH (mHistogram.size(), "Histogram equalizer has not analyzed any data.");
	
	// Unequalize each value in the vector
	for (int i=0; i<vec.size(); i++) {
		if (!is_undef (vec[i])) {
			// Find reverse function by iterating through the
			// histogram and finding the slot. NOTE: This solution is
			// VERY slow! A binary search would be faster.
			double searchvalue01 = (vec[i]-mMin)/(mMax-mMin);
			bool found=false;
			for (int j=0; j<mHistogram.size()-1; j++)
				if ((mHistogram[j]+mHistogram[j+1])/2 > searchvalue01) {
					// Calculate the updated value. Note that we would
					// get more accurate results by interpolating
					// between current and previous slot.
					vec[i] = mLowBound+mHistogram[j]*(mUpBound-mLowBound);
					found = true;
					break;
				}
			// If the value was not found from the histogram, it must
			// be greater than the upper bound. Since we can't
			// calculate beyond the bounds, limit to the upper bound.
			if (!found)
				vec[i] = mUpBound;
		} else if (mHandleMissing)
			vec[i] = mLowBound+mHistogram[mHistogram.size()/2]*(mUpBound-mLowBound);
	}
}

TextOStream& HistogramEq::operator>> (TextOStream& out) const
{
	out << "<HistogramEq lowBound=" << mLowBound
		<< " upBound=" << mUpBound
		<< " min=" << mMin
		<< " max=" << mMax
		<< ">";
	out << "</HistogramEq>";

	return out;
}


//////////////////////////////////////////////////////////////////////////////
//           ----                         o             -----               //
//          |      ___         ____  ____    ___    _   |                   //
//          | ---  ___| |   | (     (     |  ___| |/ \  |---   __           //
//          |   \ (   | |   |  \__   \__  | (   | |   | |     /  \          //
//          |___/  \__|  \__! ____) ____) |  \__| |   | |____ \__|          //
//                                                               |          //
//////////////////////////////////////////////////////////////////////////////

GaussianEq::GaussianEq (const GaussianEq& orig)
		: Equalizer(orig),
		  mAverage (orig.mAverage), mStdDev (orig.mStdDev) {	
}

/*virtual*/ void GaussianEq::analyze (const Vector& vec, bool additive) {
	// Calculate average
	float sum=0.0;
	int actualPatterns=0; // There may be missing values
	for (int i=0; i<vec.size(); i++)
		if (!is_undef (vec[i])) {
			sum += vec[i];
			actualPatterns++;
		}
	mAverage = sum/actualPatterns;

	// Calculate stddev
	sum=0.0;
	for (int i=0; i<vec.size(); i++)
		if (!is_undef (vec[i]))
			sum += sqr (vec[i]-mAverage);
	mStdDev = sqrt (sum / actualPatterns);
}

/*virtual*/ void GaussianEq::equalize (Vector& vec) const {
	for (int i=0; i<vec.size(); i++)
		if (!is_undef (vec[i]))
			vec[i] = (vec[i] - mAverage)/mStdDev;
		else if (mHandleMissing)
			vec[i] = 0.0;
}

/*virtual*/ void GaussianEq::unequalize (Vector& vec) const {
	for (int i=0; i<vec.size(); i++)
		if (!is_undef (vec[i]))
			vec[i] = (vec[i] - mAverage)/mStdDev;
		else if (mHandleMissing)
			vec[i] = mAverage;
}



//////////////////////////////////////////////////////////////////////////////
//                 |   | o                       -----                      //
//                 |\ /|     _          ___      |                          //
//                 | V | | |/ \  |/|/|  ___| \ / |---   __                  //
//                 | | | | |   | | | | (   |  X  |     /  \                 //
//                 |   | | |   | | | |  \__| / \ |____ \__|                 //
//                                                        |                 //
//////////////////////////////////////////////////////////////////////////////

MinmaxEq::MinmaxEq (double trgMin, double trgMax)
{
	mTrgMin = trgMin;
	mTrgMax = trgMax;
	mDataMin = 1E30;
	mDataMax = -1E30;
}

MinmaxEq::MinmaxEq (const MinmaxEq& orig)
		: Equalizer(orig),
		  mTrgMin (orig.mTrgMin), mTrgMax (orig.mTrgMax),
		  mDataMin (orig.mDataMin), mDataMax (orig.mDataMax)
{
}

// Deprecated: now implemented in standard libraries
// template<class T> T min (T x, T y) {return (x<y)? x:y;}
// template<class T> T max (T x, T y) {return (x>y)? x:y;}

void MinmaxEq::analyze (const Vector& vec, bool additive)
{
	mDataMin = additive? min<double>(mDataMin,min (vec)) : min (vec);
	mDataMax = additive? max<double>(mDataMax,max (vec)) : max (vec);
}

void MinmaxEq::equalize (Vector& vec) const
{
	for (int i=0; i<vec.size(); i++)
		if (!is_undef (vec[i]))
			vec[i] = mTrgMin + (vec[i] - mDataMin)/(mDataMax-mDataMin)*(mTrgMax-mTrgMin);
		else if (mHandleMissing)
			vec[i] = (mTrgMax+mTrgMin)/2; // Use the center value for default
}

void MinmaxEq::unequalize (Vector& vec) const
{
	for (int i=0; i<vec.size(); i++)
		if (!is_undef (vec[i]))
			vec[i] = (vec[i]-mTrgMin)/(mTrgMax-mTrgMin)*(mDataMax-mDataMin)+mDataMin;
		else if (mHandleMissing)
			vec[i] = (mDataMax+mDataMin)/2; // Use the center value for default
}

TextOStream& MinmaxEq::operator>> (TextOStream& out) const
{
	out << getclassname() << " "
		<< mDataMin << " " << mDataMax << " "
		<< mTrgMin << " " << mTrgMax << " ";
	return out;
}

TextIStream& MinmaxEq::operator<< (TextIStream& in)
{
	in >> mDataMin >> mDataMax >> mTrgMin >> mTrgMax;
	return in;
}


//////////////////////////////////////////////////////////////////////////////
//    |   |               o     -----                  | o                  //
//    |\ /|  ___   |            |                 ___  |   ___  ___         //
//    | V |  ___| -+- |/\ | \ / |---   __  |   |  ___| | |   / /   ) |/\    //
//    | | | (   |  |  |   |  X  |     /  \ |   | (   | | |  /  |---  |      //
//    |   |  \__|   \ |   | / \ |____ \__|  \__!  \__| | | /__  \__  |      //
//                                       |                                  //
//////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
 * Constructor. The prototype is an Equalizer instance which has been
 * created with appropriate parameters for the specific equalization
 * method.
 *
 *  The prototype is applied for building equalizers for each
 *  column plane.
 *
 *  The MatrixEqualizer takes the ownership of the prototype and
 *  destroys it accordingly.
 ******************************************************************************/
MatrixEqualizer::MatrixEqualizer (Equalizer* templ)
{
	// Put the template as the first item in the equalizer set
	if (templ)
		mPlaneEqualizers.add (templ);
}

/*******************************************************************************
 * Analyzes a matrix for equalization.
 *
 * If the optional additive-parameter is true, the analysis is made
 * for all columns at the same time, i.e., they share their value
 * space. This may be useful if the relative values of different
 * columns are significant.
 ******************************************************************************/
void MatrixEqualizer::analyze (const Matrix& mat, bool global)
{
	ASSERTWITH (mPlaneEqualizers.size() > 0, "No equalizer template defined for matrix equalizer.");

	// Copy the template
	if (!global)
		mPlaneEqualizers.resize (mat.cols);
	else
		mPlaneEqualizers.resize (1);

	mIsGlobal = global;

	// Analyze each component column
	for (int i=0; i<mat.cols; i++) {
		if (i>0 && !global)
			mPlaneEqualizers.put (mPlaneEqualizers.getp(0)->clone(), i);

		// Copy matrix column to a vector
		Vector plane (mat.rows); // Component plane
		for (int j=0; j<plane.size(); j++)
			plane[j] = mat.get (j,i);

		// Analyze the column vector
		mPlaneEqualizers.getp(global? 0:i)->analyze (plane, global);
	}
}

/*******************************************************************************
 * Internal function for doing both equalization and unequalization.
 ******************************************************************************/
void MatrixEqualizer::bothEqualize (Matrix& mat, bool uneq) const
{
	for (int i=0; i<mat.cols; i++) {
		Vector plane (mat.rows);

		// Copy the column (component plane) vector from the matrix
		for (int j=0; j<plane.size(); j++)
			plane[j] = mat.get (j,i);

		// Apply the column equalizer. If there is only one equalizer,
		// but the matrix has more columns, we can assume that the
		// planes have been analyzed with global data.
		const Equalizer* eq = mPlaneEqualizers.getp(mIsGlobal? 0:i);
		if (uneq)
			eq->unequalize (plane);
		else
			eq->equalize (plane);

		//for (int q=0; q<plane.size(); q++)
		//	sout << plane[q] << " ";
		//sout << "\n";
		// Copy the equalized column (component plane) vector back to the matrix
		for (int j=0; j<plane.size(); j++)
			mat.get (j,i) = plane[j];
	}
}

/*******************************************************************************
 * Equalizes a matrix by applying equalization data acquired from a
 * previous analyze().
 ******************************************************************************/
void MatrixEqualizer::equalize (Matrix& mat) const
{
	bothEqualize (mat, false);
}

/*******************************************************************************
 * Reverses the equalization of a matrix by applying equalization data
 * acquired from a previous analyze().
 ******************************************************************************/
void MatrixEqualizer::unequalize (Matrix& mat) const
{
	bothEqualize (mat, true);
}

TextOStream& MatrixEqualizer::operator>> (TextOStream& out) const
{
	if (false /* out.iword(xmlflag */) {
		out << "<planeEqualizers size="
			<< mPlaneEqualizers.size() << ">\n";
		for (int i=0; i<mPlaneEqualizers.size(); i++) {
			out << "\t<equalizer id=" << i
				<< " class='" << mPlaneEqualizers[i].getclassname() << "'>"
				<< mPlaneEqualizers[i]
				<< "</equalizer>\n";
		}
		out << "</planeEqualizers>\n";
	} else {
		out << getclassname() << " " << mPlaneEqualizers.size() << " ";
		for (int i=0; i<mPlaneEqualizers.size(); i++)
			out << mPlaneEqualizers[i];
	}
	return out;
}

Equalizer* readEqualizer (TextIStream& in)
{
	// Read class name of the equalizer
	String classname;
	in >> classname;

	// Create the equalizer dynamically by class name
	if (Equalizer* obj = dynamic_cast<Equalizer*> (dyncreate (classname))) {
		(*obj) << in;
		return obj;
	} else
		throw invalid_format (i18n("Unexpected object class identifier '%1', class Equalizer expected.").arg(classname));
}

TextIStream& MatrixEqualizer::operator<< (TextIStream& in)
{
	// Read size
	int size;
	in >> size;
	mPlaneEqualizers.make(size);

	for (int i=0; i<size; i++)
		mPlaneEqualizers.put (readEqualizer(in), i);
	
	return in;
}


void MatrixEqualizer::handleMissing (bool enable)
{
	mHandleMissing = enable;
	for (int i=0; i < mPlaneEqualizers.size(); i++)
		mPlaneEqualizers[i].handleMissing (enable);
}
