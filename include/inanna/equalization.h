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

#ifndef __INANNA_EQUALIZER_H__
#define __INANNA_EQUALIZER_H__

#include <magic/mobject.h>
#include <magic/mmath.h>
#include <magic/mmatrix.h>
#include <magic/mpararr.h>
#include <magic/mtextstream.h>

// XML format ios flag
extern int xmlflag;



//////////////////////////////////////////////////////////////////////////////
//                 -----                  | o                               //
//                 |                 ___  |   ___  ___                      //
//                 |---   __  |   |  ___| | |   / /   ) |/\                 //
//                 |     /  \ |   | (   | | |  /  |---  |                   //
//                 |____ \__|  \__!  \__| | | /__  \__  |                   //
//                          |                                               //
//////////////////////////////////////////////////////////////////////////////

/** Abstract interface for vector equalization methods. We do not
 *  actually equalize pattern sets, but more generally single vectors
 *  which each are one column (a component or variable plane) of a
 *  pattern set matrix.
 *
 *  You can equalize entire pattern sets by using the MatrixEqualizer.
 **/
class Equalizer : public Object {
	decl_dynamic (Equalizer);
  public:
						Equalizer		() {mHandleMissing=false;}
						Equalizer		(const Equalizer& o) {mHandleMissing=o.mHandleMissing;}
	
	virtual void		analyze			(const Vector& vec, bool additive=false); // MUST OVERLOAD
	virtual void		equalize		(Vector& vec) const; // MUST OVERLOAD
	virtual void		unequalize		(Vector& vec) const; // MUST OVERLOAD
	virtual Equalizer*	clone			() const;  // MUST OVERLOAD
	virtual void		handleMissing	(bool enable=true);

	// Implementations
	virtual	TextOStream&	operator>>	(TextOStream&) const;

  protected:
	bool	mHandleMissing;
};



///////////////////////////////////////////////////////////////////////////////
//          |   | o                                      -----               //
//          |   |    ____  |                  ___        |                   //
//          |---| | (     -+-  __   ___  |/\  ___| |/|/| |---   __           //
//          |   | |  \__   |  /  \ (   \ |   (   | | | | |     /  \          //
//          |   | | ____)   \ \__/  ---/ |    \__| | | | |____ \__|          //
//                                  __/                           |          //
///////////////////////////////////////////////////////////////////////////////

/** Histogram equalization method (strategy).
 **/
class HistogramEq : public Equalizer {
	decl_dynamic (HistogramEq);
  public:
							HistogramEq	(int precision=100000, double floor=0.0, double ceiling=1.0);
							HistogramEq (const HistogramEq& orig);
	virtual HistogramEq*	clone		() const {return new HistogramEq (*this);}
	
	virtual void			analyze		(const Vector& vec, bool additive=false);
	virtual void			equalize	(Vector& vec) const;
	virtual void			unequalize	(Vector& vec) const;
	
	/** Implementation of serialization. */
	virtual	TextOStream&	operator>>	(TextOStream&) const;

  private:
	static int		determineDomain (float x, int precision, float lowBound, float upBound);
	void			finalize		();
	void			analyze2		(const Vector& vec);

	double			mLowBound;	// Bound in the unequalized data
	double			mUpBound;	// Bound in the unequalized data
	double			mMin;		// Desired lower bound in the equalized data
	double			mMax;		// Desired upper bound in the equalized data
	Array<Vector>	mData;		// Accumulated additive analysis data
	Vector			mHistogram;	// Histogram formed by analysis
};



//////////////////////////////////////////////////////////////////////////////
//           ----                         o             -----               //
//          |      ___         ____  ____    ___    _   |                   //
//          | ---  ___| |   | (     (     |  ___| |/ \  |---   __           //
//          |   \ (   | |   |  \__   \__  | (   | |   | |     /  \          //
//          |___/  \__|  \__! ____) ____) |  \__| |   | |____ \__|          //
//                                                               |          //
//////////////////////////////////////////////////////////////////////////////

/** Normal distribution equalization method (strategy).
 **/
class GaussianEq : public Equalizer {
	decl_dynamic (GaussianEq);
  public:
						GaussianEq	(double avg=0.0, double stddev=0.0) {mAverage=0; mStdDev=0;}
						GaussianEq	(const GaussianEq& orig);
	virtual GaussianEq*	clone		() const {return new GaussianEq (*this);}
	
	virtual void		analyze		(const Vector& vec, bool additive=false);
	virtual void		equalize	(Vector& vec) const;
	virtual void		unequalize	(Vector& vec) const;

  private:
	double	mAverage;
	double	mStdDev;
};



//////////////////////////////////////////////////////////////////////////////
//                 |   | o                       -----                      //
//                 |\ /|     _          ___      |                          //
//                 | V | | |/ \  |/|/|  ___| \ / |---   __                  //
//                 | | | | |   | | | | (   |  X  |     /  \                 //
//                 |   | | |   | | | |  \__| / \ |____ \__|                 //
//                                                        |                 //
//////////////////////////////////////////////////////////////////////////////

/** Linear range equalization method (strategy).
 **/
class MinmaxEq : public Equalizer {
	decl_dynamic (MinmaxEq);
  public:
						MinmaxEq	(double trgMin=0.0, double trgMax=1.0);
						MinmaxEq	(const MinmaxEq& orig);
	virtual MinmaxEq*	clone		() const {return new MinmaxEq (*this);}
	
	virtual void		analyze		(const Vector& vec, bool additive=false);
	virtual void		equalize	(Vector& vec) const;
	virtual void		unequalize	(Vector& vec) const;
	
	/** Implementation of serialization. */
	virtual	TextOStream&	operator>>	(TextOStream& out) const;

	/** Implementation of serialization. */
	virtual	TextIStream&	operator<<	(TextIStream& in);
	
  public:
	double	mTrgMin;
	double	mTrgMax;
	double	mDataMin;
	double	mDataMax;
};



//////////////////////////////////////////////////////////////////////////////
//    |   |               o     -----                  | o                  //
//    |\ /|  ___   |            |                 ___  |   ___  ___         //
//    | V |  ___| -+- |/\ | \ / |---   __  |   |  ___| | |   / /   ) |/\    //
//    | | | (   |  |  |   |  X  |     /  \ |   | (   | | |  /  |---  |      //
//    |   |  \__|   \ |   | / \ |____ \__|  \__!  \__| | | /__  \__  |      //
//                                       |                                  //
//////////////////////////////////////////////////////////////////////////////

/** Equalizes matrices. Stores equalization information. */
class MatrixEqualizer : public Equalizer {
	decl_dynamic (MatrixEqualizer);
  public:

						MatrixEqualizer		(Equalizer* prototype=NULL);

	void				analyze				(const Matrix& mat, bool additive=false);
	void				equalize			(Matrix& mat) const;
	void				unequalize			(Matrix& mat) const;

	/** Returns equalizer for given matrix column (component
     *  plane). The equalizer may have analyzed data or it may have
     *  not, depending on its history.
	 **/
	const Equalizer&	getPlane			(int plane) const {return *mPlaneEqualizers.getp(plane);}

	/** Returns number of columns (equalization planes). */
	int					planes				() const {return mPlaneEqualizers.size();}
	
	/** Implementation of serialization. */
	virtual	TextOStream&	operator>>	(TextOStream& out) const;

	/** Implementation of serialization. */
	virtual	TextIStream&	operator<<	(TextIStream& in);

	virtual void		handleMissing	(bool enable=true);
	
  private:
	void				bothEqualize		(Matrix& mat, bool uneq) const;

	Array<Equalizer>	mPlaneEqualizers;	/**< Equalizers for each column plane for analyzed matrices. */
	bool				mIsGlobal;			/**< Do we use global analysis mode? */
};

////////////////////////////////////////////////////////////////////////////////

/** Dynamically creates and reads an equalizer object from the given
 *  input stream.
 **/
Equalizer* readEqualizer (TextIStream& in);

#endif
