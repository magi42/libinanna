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

#ifndef __PREDICTION_H__
#define __PREDICTION_H__

#include <magic/mmath.h>
#include <magic/mmatrix.h>
#include <magic/mmap.h>
#include <magic/mclass.h>
#include <magic/mpararr.h>
#include <inanna/annetwork.h>

class PatternSet;		// In patternset.h
class MatrixEqualizer;	// In equalization.h
class TrainingObserver;	// In trainer.h



///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  |   |                |    |       ___                    ----            //
//  |\ /|        _    |  | _  |       |  \   ___   |   ___  (      ___   |   //
//  | V |  __  |/ \  -+- |/ | | \   | |   |  ___| -+-  ___|  ---  /   ) -+-  //
//  | | | /  \ |   |  |  |  | |  \  | |   | (   |  |  (   |     ) |---   |   //
//  |   | \__/ |   |   \ |  | |   \_/ |__/   \__|   \  \__| ___/   \__    \  //
//                               \_/                                         //
///////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
 * Contains data that has month information.
 ******************************************************************************/
class MonthlyDataSet : public Matrix {
  public:

	/** Default constructor. */
			MonthlyDataSet	() {mFirstMonth0=mFirstYear=-1;}

	/** Typical constructor for the monthly data set. */
			MonthlyDataSet	(const Matrix& matrix, int dateYYMM=0, int lag=0) : Matrix (matrix) {
				if (const MonthlyDataSet* set = dynamic_cast<const MonthlyDataSet*>(&matrix)) {
					mFirstMonth0 = set->mFirstMonth0;
					mFirstYear = set->mFirstYear;
					mLag = set->mLag;
				} else {
					mFirstMonth0 = (dateYYMM%100)-1;
					mFirstYear = dateYYMM/100;
					mLag = lag;
				}
			}

	/** Returns a subset of the data set. */
	Ref<MonthlyDataSet>	sub				(int row0, int row1, int col0, int col1) const;

	/** Returns number of lag months for prediction. */
	int					lag				() const {return mLag;}

	/** Sets number of lag months for prediction. */
	void				setLag			(int lag) {mLag=lag;}

	/** Returns the zero-based number of the first month in the data set. */
	int					firstMonth0		() const {return mFirstMonth0;}

	/** Sets the zero-based number of the first month in the data set. */
	void				setFirstMonth0	(int m0) {mFirstMonth0=m0;}

  protected:
	int		mFirstMonth0;
	int		mFirstYear;
	int		mLag;
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                          //
// ----                | o           o            -----                 ----                    |           //
// |   )      ___      |    ___   |           _     |    ___   ____  |  |   )  ___   ____       |  |   ____ //
// |---  |/\ /   )  ---| | |   \ -+- |  __  |/ \    |   /   ) (     -+- |---  /   ) (     |   | | -+- (     //
// |     |   |---  (   | | |      |  | /  \ |   |   |   |---   \__   |  | \   |---   \__  |   | |  |   \__  //
// |     |    \__   ---| |  \__/   \ | \__/ |   |   |    \__  ____)   \ |  \   \__  ____)  \__! |   \ ____) //
//                                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
 * Holder for prediction test results.
 ******************************************************************************/
class PredictionTestResults : public Object {
  public:
				PredictionTestResults () {}

	void		make		(int variables);
	void		addValue	(int variable, double value);
	void		calculate	();
	
	double		averageError;	// For all variables
	double		MSE;			// For all variables
	double		RMSE;			// For all variables
	Vector		averageErrors;	// For each variable
	Vector		MSEs;			// For each variable
	Vector		RMSEs;			// For each variable

	OStream&	operator>>	(OStream& out) const;

  private:
	int		mValues; /** Number of values. */
	
	void operator = (const PredictionTestResults& o) {FORBIDDEN}
	PredictionTestResults (const PredictionTestResults& o) {FORBIDDEN}
};



//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          //
// ----                | o           o             ----                                     //
// |   )      ___      |    ___   |           _   (      |       ___   |   ___              //
// |---  |/\ /   )  ---| | |   \ -+- |  __  |/ \   ---  -+- |/\  ___| -+- /   )  ___  \   | //
// |     |   |---  (   | | |      |  | /  \ |   |     )  |  |   (   |  |  |---  (   \  \  | //
// |     |    \__   ---| |  \__/   \ | \__/ |   | ___/    \ |    \__|   \  \__   ---/   \_/ //
//                                                                               __/   \_/  //
//////////////////////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
 * Abstract baseclass for different monthly prediction methods.
 *
 * Design Patterns: Strategy.
 ******************************************************************************/
class PredictionStrategy : public Object {
	decl_dynamic (PredictionStrategy);
  public:
						PredictionStrategy	() {FORBIDDEN}
						PredictionStrategy	(const String& name) : mName (name) {}

	virtual void		make				(const StringMap& params);
	virtual void		train				(const Matrix& traindata, int startmonth);
	virtual Ref<Matrix>	predict				(const Matrix& testdata, int startmonth) const;
	virtual PredictionTestResults*	test	(const Matrix& testdata, int startmonth) const;
	virtual void		testCurve			(const Matrix& testdata, int startmonth, const String& filename) const;

	/** Returns the name of the prediction method. */
	const String&		name				() const {return mName;}
	virtual int			inputMonths			() const {return mInputMonths;}
	
  protected:
	String	mName;

	/** Number of months to be used as prediction inputs before the
	 *  first predicted month. The testdata contains these patterns in
	 *  the beginning in the set.
	 **/
	int		mInputMonths;

	static Ref<Matrix>	rowDeltas			(const Matrix& matrix);
};

/*******************************************************************************
 * Previous Year prediction method
 *
 * A trivial prediction method that always predicts same values as
 * in the corresponding months in the previous year.
 *
 * Any sophisticated prediction method should do better than this.
 ******************************************************************************/
class PreviousYear : public PredictionStrategy {
	decl_dynamic (PreviousYear);
  public:
						PreviousYear			() : PredictionStrategy ("PreviousYear") {mInputMonths=0;}
						PreviousYear			(const StringMap& params);
	virtual void		train					(const Matrix& traindata, int startmonth);
	virtual Ref<Matrix>	predict					(const Matrix& testdata, int startmonth) const;
	virtual int			inputMonths				() const {return 0;}
  protected:
	Matrix			mData;
};

/*******************************************************************************
 * Average of Previous Years prediction method
 *
 * A trivial prediction method that predicts, for each month in the predicted
 * year, the average value of the particular month in the past years.
 *
 * Any sophisticated prediction method should do better than this.
 ******************************************************************************/
class PreviousYearsAvg : public PredictionStrategy {
	decl_dynamic (PreviousYearsAvg);
  public:
						PreviousYearsAvg		() : PredictionStrategy ("PreviousYearsAvg") {mInputMonths=0;}
						PreviousYearsAvg		(const StringMap& params);
	virtual void		train					(const Matrix& traindata, int startmonth);
	virtual Ref<Matrix>	predict					(const Matrix& testdata, int startmonth) const;
	virtual int			inputMonths				() const {return 0;}

  protected:
	Matrix			mMonthlyAvg;
};

/*******************************************************************************
 * Average Change of Previous Years prediction method
 *
 * A somewhat trivial relative prediction method that calculates the monthly
 * changes in previous years, takes a monthly average of them, and applies
 * them to the predicted year.
 *
 * This has proven to be a rather good prediction method for many cases.
 ******************************************************************************/
class AverageDeltaPrediction : public PredictionStrategy {
	decl_dynamic (AverageDeltaPrediction);
  public:
						AverageDeltaPrediction	() : PredictionStrategy ("AverageDelta") {mInputMonths=1;}
						AverageDeltaPrediction	(const StringMap& params);
	virtual void		train					(const Matrix& traindata, int startmonth);
	virtual Ref<Matrix>	predict					(const Matrix& testdata, int startmonth) const;
	const Matrix&		deltas					() const {return mMonthAvg;}
	virtual int			inputMonths				() const {return 1;}
  protected:
	Matrix			mMonthAvg;
	PackArray<int>	mMonthCnt;
};

/*******************************************************************************
 * A combined trivial prediction method
 *
 * A prediction method that combines the other trivial methods by first ranking
 * their performance on each month in the training set, and then choosing the
 * best ranking method to predict a particular month in a test set.
 *
 * This has proven to be a rather good prediction method for many cases.
 ******************************************************************************/
class CombinedPrediction : public PredictionStrategy {
	decl_dynamic (CombinedPrediction);
  public:
						CombinedPrediction		() : PredictionStrategy ("CombinedPrediction") {}
						CombinedPrediction		(const StringMap& params);
	virtual void		make					(const StringMap& params);
	virtual void		train					(const Matrix& traindata, int startmonth);
	virtual Ref<Matrix>	predict					(const Matrix& testdata, int startmonth) const;

  protected:
	Array<PredictionStrategy>	mPredictors;
	PackTable<int>			mPredictorChoises;

  private:
	int					determineInputMonths	() const;
};

/*******************************************************************************
 * A dummy prediction method
 *
 * This prediction method doesn't really predict anything -- it always predicts
 * the same value as in the previous month.
 *
 * Any sensible method should do better than this.
 ******************************************************************************/
class ZeroDeltaPrediction : public PredictionStrategy {
	decl_dynamic (ZeroDeltaPrediction);
  public:
						ZeroDeltaPrediction		() : PredictionStrategy ("ZeroDelta") {mInputMonths=1;}
						ZeroDeltaPrediction		(const StringMap& params);
	virtual void		train					(const Matrix& traindata, int startmonth);
	virtual Ref<Matrix>	predict					(const Matrix& testdata, int startmonth) const;
	virtual int			inputMonths				() const {return 1;}
  protected:
};


//////////////////////////////////////////////////////////////////////////////////
// |   |                       | ----                | o           o            //
// |\  |  ___             ___  | |   )      ___      |    ___   |           _   //
// | \ | /   ) |   | |/\  ___| | |---  |/\ /   )  ---| | |   \ -+- |  __  |/ \  //
// |  \| |---  |   | |   (   | | |     |   |---  (   | | |      |  | /  \ |   | //
// |   |  \__   \__! |    \__| | |     |    \__   ---| |  \__/   \ | \__/ |   | //
//////////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
 *  Neural network based prediction method.
 *
 *  Easily customizable by the undocumented parameters for the make()
 *  method.
 ******************************************************************************/
class AbsoluteNeuralPrediction : public PredictionStrategy {
	decl_dynamic (AbsoluteNeuralPrediction);
  public:
						AbsoluteNeuralPrediction	(const char* name=NULL) : PredictionStrategy (name? name:"AbsoluteNeural")  {mpNetwork=NULL; rpObserver=NULL;}
						~AbsoluteNeuralPrediction	();
	virtual void		make						(const StringMap& params);
	virtual void		train						(const Matrix& traindata, int startmonth);
	virtual Ref<Matrix>	predict						(const Matrix& testdata, int startmonth) const;

	void				setObserver					(TrainingObserver* observer) {rpObserver=observer;}
	virtual void		load						(TextIStream& in);
	virtual void		save						(TextOStream& out) const;

  protected:
	/** Trained network. */
	ANNetwork*			mpNetwork;

	/** Flag that says whether we train all variables at a time, i.e.,
	 *  have all the variables in the output layer.
	 **/
	bool				mUseAllOutputs;

	/** Should all input variables be used or just one? If false,
	 *  mUseAllOutputs must also be false (we can't predict all
	 *  variables with the information from just one variable).
	 **/
	bool				mUseAllInputs;

	/** Currently handled output variable. This value changes during
	 *  the run.
	 **/
	int					mVariable;

	/** Should global equalization be used? */
	bool				mGlobalEqualization;

	/** Topology description string for the hidden units, for example
	 *  "10-5-5".
	 **/
	String				mHiddenTopology;

	/** Application parameters, stored here for subsequent use. */
	StringMap			mParams;

	/** Training observer that can print out training log during
	 *  network training.
	 **/
	TrainingObserver*	rpObserver;

  protected:
	/** Builds pattern set from given dataset and starting month. */
	PatternSet*			makeSet					(const Matrix& data, int startmonth) const;

  private:
	int					inputVariables			(int datacolumns) const {return 12+mInputMonths*(mUseAllInputs? datacolumns : 1);}
	int					outputVariables			(int datacolumns) const {return mUseAllOutputs? datacolumns : 1;}
};

class SingleNeuralPrediction : public AbsoluteNeuralPrediction {
	decl_dynamic (SingleNeuralPrediction);
  public:
						SingleNeuralPrediction		() : AbsoluteNeuralPrediction ("SingleNeural")  {mpNetwork=NULL;}
	virtual void		make						(const StringMap& params);
	virtual void		train						(const Matrix& traindata, int startmonth);
	virtual Ref<Matrix>	predict						(const Matrix& testdata, int startmonth) const;

	virtual void		load						(TextIStream& in);
	virtual void		save						(TextOStream& out) const;

  protected:
	Array<ANNetwork>	mNetworks;
};

/** Not in use currently.
 *
 **/
class DeltaNeuralPrediction : public PredictionStrategy {
	decl_dynamic (DeltaNeuralPrediction);
  public:
						DeltaNeuralPrediction	() : PredictionStrategy ("DeltaNeural") {}
						DeltaNeuralPrediction	(const StringMap& params);
	virtual void		train					(const Matrix& traindata, int startmonth);
	virtual Ref<Matrix>	predict					(const Matrix& testdata, int startmonth) const;
  protected:
};

/** This might be a nice change for future. */
class StochasticPrediction : public PredictionStrategy {
  public:
	virtual void		make						(const StringMap& params);
	virtual void		train						(const Matrix& traindata, int startmonth);
	virtual Ref<Matrix>	predict						(const Matrix& testdata, int startmonth) const;

  protected:
	virtual Matrix	predictRun					();
};

/** This might be a nice change for future. */
class NeuralPrediction {
  public:
  protected:
};



///////////////////////////////////////////////////////////////////////////////////////////////////
//   _             | o     o              ----                                     |     o       //
//  / \            |    |      _         (      |       ___   |   ___              |       |     //
// /   \ |   |  ---| | -+- | |/ \   ___   ---  -+- |/\  ___| -+- /   )  ___  \   | |     | |---  //
// |---| |   | (   | |  |  | |   | (   \     )  |  |   (   |  |  |---  (   \  \  | |     | |   ) //
// |   |  \__!  ---| |   \ | |   |  ---/ ___/    \ |    \__|   \  \__   ---/   \_/ |____ | |__/  //
//                                  __/                                 __/   \_/                //
///////////////////////////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
 * Factory of prediction strategies
 *******************************************************************************/
class PredictionStrategyLib {
  public:
									PredictionStrategyLib ();
	PredictionStrategy*				create		(int i) const;
	int								strategies	() const {return mClassNames.size();}
	void							registerCls	(const String& classname) {mClassNames.add(classname);}
	static	PredictionStrategyLib&	instance	() {return sInstance;}
  protected:
	static PredictionStrategyLib    sInstance;
	Array<String>					mClassNames;
};



///////////////////////////////////////////////////////////////////////////////////
// -----           o       ----                                                  //
//   |        ___      _   |   )  ___       ___         ___   |   ___       ____ //
//   |   |/\  ___| | |/ \  |---   ___| |/\  ___| |/|/| /   ) -+- /   ) |/\ (     //
//   |   |   (   | | |   | |     (   | |   (   | | | | |---   |  |---  |    \__  //
//   |   |    \__| | |   | |      \__| |    \__| | | |  \__    \  \__  |   ____) //
///////////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
 * Training parameter object for neural prediction methods.  KAuditor uses
 * this for handling training parameters.
 ******************************************************************************/
class TrainParameters {
  public:
  					TrainParameters	() {defaults();}

	void			defaults		();
	String			hiddenString	() const;
	Ref<StringMap>	getParams		() const;
	void			setParams		(const StringMap& map);
	void			write			(TextOStream& out) const;
	void			read			(TextIStream& in);

  public:
  	int 	trainCycles;
  	float	delta0;
  	float	deltaMax;
  	float	weightDecay;
  	bool	useWeightDecay;
	int		hidden1;
	int 	hidden2;
	int 	hidden3;
	int 	runs;
	bool	allInputs;
	bool	allOutputs;
	bool	monthInputs;
	bool	equalizeGlobal;
	int		inputMonths;
};

#endif

