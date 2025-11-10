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

#include <magic/mobject.h>
#include <magic/mtextstream.h>

#include <inanna/annetwork.h>
#include <inanna/trainer.h>
#include <inanna/rprop.h>
#include <inanna/patternset.h>
#include <inanna/equalization.h>
#include <inanna/annfilef.h>
#include <inanna/prediction.h>

impl_dynamic (PredictionStrategy, {Object});
impl_dynamic (ZeroDeltaPrediction, {PredictionStrategy});
impl_dynamic (AverageDeltaPrediction, {PredictionStrategy});
impl_dynamic (PreviousYear, {PredictionStrategy});
impl_dynamic (PreviousYearsAvg, {PredictionStrategy});
impl_dynamic (CombinedPrediction, {PredictionStrategy});
impl_dynamic (AbsoluteNeuralPrediction, {PredictionStrategy});
impl_dynamic (SingleNeuralPrediction, {AbsoluteNeuralPrediction});
impl_dynamic (DeltaNeuralPrediction, {PredictionStrategy});


/*******************************************************************************
 *
 ******************************************************************************/
Ref<MonthlyDataSet> MonthlyDataSet::sub (int row0, int row1, int col0, int col1) const
{
	Ref<MonthlyDataSet> result = new MonthlyDataSet (Matrix::sub (row0, row1, col0, col1));
	result->mFirstMonth0 = (mFirstMonth0+row0)%12;
	result->mFirstYear = mFirstYear+(mFirstMonth0+row0)/12;
	result->mLag = mLag;
	return result;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//   _             | o     o             -----                 ----                    |           //
//  / \            |    |      _           |    ___   ____  |  |   )  ___   ____       |  |   ____ //
// /   \ |   |  ---| | -+- | |/ \   ___    |   /   ) (     -+- |---  /   ) (     |   | | -+- (     //
// |---| |   | (   | |  |  | |   | (   \   |   |---   \__   |  | \   |---   \__  |   | |  |   \__  //
// |   |  \__!  ---| |   \ | |   |  ---/   |    \__  ____)   \ |  \   \__  ____)  \__! |   \ ____) //
//                                  __/                                                            //
/////////////////////////////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
 * Initializes test results.
 ******************************************************************************/
void PredictionTestResults::make (int variables /**< Number of variables in the data*/)
{
	averageError = 0.0;
	MSE = 0.0;
	RMSE = 0.0;
	averageErrors.make(variables);
	MSEs.make(variables);
	RMSEs.make(variables);
	for (int i=0; i<MSEs.size(); i++) {
		averageErrors[i] = 0.0;
		MSEs[i] = 0.0;
		RMSEs[i] = 0.0;
	}
	mValues = 0;
}

/*******************************************************************************
 * Adds a data point to test results
 ******************************************************************************/
void PredictionTestResults::addValue (int variable,
									double value)
{
	averageError += value;
	MSE += value*value;
	averageErrors[variable] += value;
	MSEs[variable] += value*value;

	if (variable==0)
		mValues++;
}

/*******************************************************************************
 * Calculates the test results
 ******************************************************************************/
void PredictionTestResults::calculate ()
{
	averageError /= mValues * MSEs.size();
	MSE /= mValues * MSEs.size();
	RMSE = sqrt (MSE);
	for (int i=0; i<MSEs.size(); i++) {
		averageErrors[i] /= mValues;
		MSEs[i] /= mValues;
		RMSEs[i] = sqrt (MSEs[i]);
	}
}

/*******************************************************************************
 * Prints out the test results
 ******************************************************************************/
OStream& PredictionTestResults::operator>> (OStream& out) const
{
	printf ("Average error = %f\n", averageError);
	printf ("Mean Squared Error = %f\n", MSE);
	printf ("Root of Mean Squared Error = %f\n", RMSE);
	printf ("Variable\tavgErr\t\tMSE\t\tRMSE\n");
	for (int i=0; i<MSEs.size(); i++)
		printf ("%3d\t\t%#5.10g\t%#5.10g\t%5.10g\n",
				i+1, averageErrors[i], MSEs[i], RMSEs[i]);
	return out;
}

/////////////////////////////////////////////////////////////////////////////////////
//   _             | o     o              ----                                     //
//  / \            |    |      _         (      |       ___   |   ___              //
// /   \ |   |  ---| | -+- | |/ \   ___   ---  -+- |/\  ___| -+- /   )  ___  \   | //
// |---| |   | (   | |  |  | |   | (   \     )  |  |   (   |  |  |---  (   \  \  | //
// |   |  \__!  ---| |   \ | |   |  ---/ ___/    \ |    \__|   \  \__   ---/   \_/ //
//                                  __/                                 __/   \_/  //
/////////////////////////////////////////////////////////////////////////////////////

/** Initialization. */
void PredictionStrategy::make (const StringMap& params)
{
	mInputMonths	= params["inputMonths"].toInt();
}

/*******************************************************************************
 * Trains the learning method with the given data.
 *
 * Must be implemented by prediction strategies.
 *
 * @param startmonth Zero-based month number for the first sample in the
 * data. The month number can be in range 0-11, but only the last two digits are
 * interpreted as months, so it can have values like 199905 or 9905.
 ******************************************************************************/
void PredictionStrategy::train (
	const Matrix& traindata, /**< Data for training the method. */
	int           startmonth)
{
	MUST_OVERLOAD;
}

/*******************************************************************************
 * Tests the data and returns the monthly predictions in matrix.
 *
 *  Must be implemented by prediction strategies.
 *
 *  @param startmonth See train().
 ******************************************************************************/
Ref<Matrix>	PredictionStrategy::predict (
	const Matrix& testdata,
	int           startmonth) const
{
	MUST_OVERLOAD;
	return Ref<Matrix> (NULL);
}

/*******************************************************************************
 * Tests the learning method with the given data.
 *
 *  @param inputmonths Most prediction methods require at least one month of
 *  data before the first data point to be predicted. Thus, if you want to
 *  predict 12 months sequence, you have to give for example 13 months of test
 *  data. Some prefer two months, while some may require three or even more.
 *
 *  @param inputmonths Number of months to be used as prediction inputs before
 *  the first predicted month. The testdata contains these patterns in the
 *  beginning in the set.
 *
 *  @param startmonth See train().
 ******************************************************************************/
/*virtual*/ PredictionTestResults* PredictionStrategy::test (
	const Matrix& testdata,
	int           startmonth) const
{
	// Let the strategy do the prediction
	Ref<Matrix> testresRef = predict (testdata, startmonth);
	Matrix& testres = testresRef;

	/**********************************************/
	/* Collect error statistics for each pattern  */

	// Create result storage object
	PredictionTestResults* result = new PredictionTestResults ();
	result->make (testres.cols);

	// Iterate through test patterns. Skip the mInputMonths, since they are
	// not actual test data points.
	for (int r=0; r<testres.rows; r++) {
		// Calculate error for one test pattern
		for (int c=0; c<testres.cols; c++) {
			// Calculate error between the desired and predicted value
			double error = fabs (testdata.get(r+inputMonths(),c) - testres.get(r,c));
			//printf ("row=%d, col=%d, error=%f\n", r, c, error);
			
			// Add one sample to the statistics calculator
			result->addValue (c, error);
		}
	}

	// Finalize the statistics calculation
	result->calculate ();
	return result;
}

/*******************************************************************************
 * Tests the data and plots the result as a curve with GnuPlot.
 *
 *  @param filename File where the EPS pictures should be printed.
 ******************************************************************************/
void PredictionStrategy::testCurve (const Matrix& testdata, int startmonth, const String& filename) const
{
}

/*******************************************************************************
 * Calculates row deltas (differences between rows) for a matrix.
 *
 * Row delta is the change in a column between two rows, that is,
 * Deltas[r,c] = M[r+1,c] - M[r,c].
 *
 * @return Reference object to the delta matrix with one row less than the
 * original matrix.
 ******************************************************************************/
Ref<Matrix> PredictionStrategy::rowDeltas (const Matrix& matrix)
{
	Ref<Matrix> result = new Matrix (matrix.rows-1, matrix.cols);
	Matrix& deltas = result.object();

	// Calculate row deltas
	for (int r=0; r<deltas.rows; r++)
		for (int c=0; c<deltas.cols; c++)
			deltas.get(r,c) = matrix.get(r+1,c) - matrix.get(r,c);

	return result;
}

///////////////////////////////////////////////////////////////////////////////
//    _                                     ___         |           ----     //
//   / \         ___       ___         ___  |  \   ___  |  |   ___  |   )    //
//  /   \ |   | /   ) |/\  ___|  ___  /   ) |   | /   ) | -+-  ___| |---     //
//  |---|  \ /  |---  |   (   | (   \ |---  |   | |---  |  |  (   | |        //
//  |   |   V    \__  |    \__|  ---/  \__  |__/   \__  |   \  \__| |     O  //
//                               __/                                         //
///////////////////////////////////////////////////////////////////////////////

AverageDeltaPrediction::AverageDeltaPrediction (
	const StringMap& params)
		: PredictionStrategy ("AverageDelta")
{
}

/*virtual*/ void AverageDeltaPrediction::train (
	const Matrix& traindata, /**< Data for training the method. */
	int           startmonth /**< Starting month of the data.   */)
{
	ASSERT ((int(fabs(traindata.get(0,0)-traindata.get(1,0)+0.001))%100) != 1);
	//cout << "traindata " << traindata.rows << " rows, startmonth=" << startmonth << "\n";
	
	// Calculate deltas
	Ref<Matrix> deltasRef = rowDeltas (traindata);
	Matrix& deltas = deltasRef.object();
	
	// Initialize average matrix
	mMonthAvg.make (12, deltas.cols);
	mMonthCnt.make (12);
	for (int r=0; r<mMonthAvg.rows; r++) {
		for (int c=0; c<mMonthAvg.cols; c++)
			mMonthAvg.get(r,c) = 0.0;
		mMonthCnt[r] = 0;
	}
	
	// Calculate average deltas for each month
	int startmonthi = startmonth%100;
	for (int r=0; r<deltas.rows; r++) {
		int month = (startmonthi+r-1+1+12)%12;
		for (int c=0; c<deltas.cols; c++)
			mMonthAvg.get(month,c) = (mMonthAvg.get(month,c)*mMonthCnt[month]
									  + deltas.get(r,c))/ (mMonthCnt[month]+1);
		mMonthCnt[month]++;
	}
}

/*virtual*/ Ref<Matrix> AverageDeltaPrediction::predict (const Matrix& testdata, int startmonth) const {
	Ref<Matrix> result = new Matrix (testdata.rows-inputMonths(), testdata.cols);
	int startmonthi = startmonth%100;
	for (int r=0; r<result->rows; r++) {
		for (int c=0; c<testdata.cols; c++) {
			int month = (startmonthi+inputMonths()+r-1)%12;
			result->get(r,c) = testdata.get(r+inputMonths()-1,c) + mMonthAvg.get(month, c);
		}
	}
	return result;
}



//////////////////////////////////////////////////////////////////////////////
//           -----                ___         |           ----              //
//              /   ___           |  \   ___  |  |   ___  |   )             //
//             /   /   ) |/\  __  |   | /   ) | -+-  ___| |---              //
//            /    |---  |   /  \ |   | |---  |  |  (   | |                 //
//           /____  \__  |   \__/ |__/   \__  |   \  \__| |     O           //
//////////////////////////////////////////////////////////////////////////////

ZeroDeltaPrediction::ZeroDeltaPrediction (const StringMap& params) : PredictionStrategy ("ZeroDelta") {
}

/*virtual*/ void ZeroDeltaPrediction::train (const Matrix& traindata, int startmonth) {
}

/*virtual*/ Ref<Matrix> ZeroDeltaPrediction::predict (const Matrix& testdata, int startmonth) const {
	Ref<Matrix> result = new Matrix (testdata.rows-1, testdata.cols);
	for (int r=0; r<result->rows; r++)
		for (int c=0; c<result->cols; c++)
			result->get(r,c) = testdata.get(r,c);
	return result;
}



///////////////////////////////////////////////////////////////////////////////////////////////
//  ___                   o                 | ----                | o           o            //
// /   \            |         _    ___      | |   )      ___      |    ___   |           _   //
// |      __  |/|/| |---  | |/ \  /   )  ---| |---  |/\ /   )  ---| | |   \ -+- |  __  |/ \  //
// |     /  \ | | | |   ) | |   | |---  (   | |     |   |---  (   | | |      |  | /  \ |   | //
// \___/ \__/ | | | |__/  | |   |  \__   ---| |     |    \__   ---| |  \__/   \ | \__/ |   | //
///////////////////////////////////////////////////////////////////////////////////////////////

CombinedPrediction::CombinedPrediction (const StringMap& params) : PredictionStrategy ("CombinedPrediction")
{
	make (params);
}

void CombinedPrediction::make (const StringMap& params)
{
	PredictionStrategy::make (params);
	
	// Create and initialize predictors
	mPredictors.add (new ZeroDeltaPrediction (params));
	mPredictors.add (new PreviousYear (params));
	mPredictors.add (new PreviousYearsAvg (params));
	mPredictors.add (new AverageDeltaPrediction (params));
	
	// We use the biggest number of required input months
	mInputMonths = determineInputMonths ();
}

int CombinedPrediction::determineInputMonths () const
{
	// Find the biggest number of required input months
	int inputMonths=0;
	for (int i=0; i<mPredictors.size(); i++)
		if (mPredictors[i].inputMonths() > inputMonths)
			inputMonths = mPredictors[i].inputMonths();
	return inputMonths;
}

/*virtual*/ void CombinedPrediction::train (const Matrix& traindata, int startmonth)
{
	int firstMonth = (startmonth%100)-1; // Zero-based month number

	// Train the predictors
	for (int p=0; p<mPredictors.size(); p++)
		mPredictors[p].train (traindata, startmonth);

	////////////////////////////////////////////////////////////////////////////////
	// Evaluate the capability of predictors for each month/variable pair
	
	// Record the ID of the best predictor found so far
	mPredictorChoises.make (12, traindata.cols);
	for (int r=0; r<12; r++)
		for (int c=0; c<traindata.cols; c++)
			mPredictorChoises.get(r,c) = 0;

	// Record the smallest error found so far
	Matrix smallestError (12, traindata.cols);
	smallestError = 1e30;

	// Use training data as test data
	const Matrix& testdata = traindata;
	
	// Test each predictor
	for (int p=0; p<mPredictors.size(); p++) {
		//cout << "\nPredictor " << p << ":\n";
		int predictorInputMonths = mPredictors[p].inputMonths();

		// Test the training data with a predictor
		Ref<Matrix> predictionRef = mPredictors[p].predict (testdata, startmonth);
		Matrix& prediction = predictionRef.object();

		// WARNING: Unused
		// int firstResultMonth = (firstMonth+predictorInputMonths)%12;
		
		// See if it did better than other predictors for any value
		for (int m=0; m<12 || m<prediction.rows; m++) {
			for (int v=0; v<prediction.cols; v++) {
				int testDataRow = m+predictorInputMonths;

				// Average over all years
				double error = 0.0;
				int year=0;
				for (; year*12+m<prediction.rows; year++)
					error += fabs(prediction.get(year*12+m,v)-testdata.get(year*12+testDataRow,v));

				// Calculate average error
				error /= year;

				// Ignore too small errors. They probably imply overfitting
				if (error > 0.001) {

					// Check if the result is better than with other methods
					int month = (testDataRow+firstMonth)%12;
					if (error < smallestError.get(month,v)) {
						// It is. Use this method to predict this month/variable.
						smallestError.get(month,v) = error;
						mPredictorChoises.get(month,v) = p;
					}
				}
			}
		}
	}

	//for (int r=0; r<12; r++) {
	//	for (int c=0; c<traindata.cols; c++)
	//		cout << mPredictorChoises.get(r,c) << " ";
	//	cout << "\n";
	//}
}

/*virtual*/ Ref<Matrix> CombinedPrediction::predict (const Matrix& testdata, int startmonth) const {
	int firstMonth = (startmonth%100)-1; // Zero-based month
	Ref<Matrix> result = new Matrix (testdata.rows-inputMonths(), testdata.cols);
	
	// Test each predictor
	for (int p=0; p<mPredictors.size(); p++) {
		int predictorInputMonths = mPredictors[p].inputMonths();

		Matrix testdata2 = testdata.sub (inputMonths()-predictorInputMonths, Matrix::end, 0, Matrix::end);
		int firstTestMonth = (firstMonth+inputMonths()-predictorInputMonths)%12;
		
		// Test the training data with a predictor
		Ref<Matrix> predictionRef = mPredictors[p].predict (testdata2, firstTestMonth+1);
		Matrix& prediction = predictionRef.object();

		// Apply the predictor to those datapoints which it has been
		// found to predict well
		for (int r=0; r<result->rows; r++)
			for (int c=0; c<result->cols; c++)
				if (mPredictorChoises.get((r+firstTestMonth+predictorInputMonths)%12, c) == p)
					result->get(r,c) = prediction.get(r, c);
	}

	return result;
}



//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//         ----                  o                                          //
//         |   )      ___                      ____  ___   ___              //
//         |---  |/\ /   ) |   | |  __  |   | (     /   )  ___| |/\         //
//         |     |   |---   \ /  | /  \ |   |  \__  |---  (   | |           //
//         |     |    \__    V   | \__/  \__! ____)  \__   \__| |           //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

PreviousYear::PreviousYear (const StringMap& params) : PredictionStrategy ("PreviousYear") {
}

/*virtual*/ void PreviousYear::train (const Matrix& traindata, int startmonth) {
	ASSERT (traindata.rows >= 12);
	ASSERT ((startmonth%100) == 1);
	
	// One year
	mData.make (12, traindata.cols);
	for (int r=0; r<mData.rows; r++)
		for (int c=0; c<mData.cols; c++)
			mData.get(r,c) = traindata.get (traindata.rows-12+r,c);
}

/*virtual*/ Ref<Matrix> PreviousYear::predict (const Matrix& testdata, int startmonth) const {
	ASSERT (mData.rows>0);
	ASSERT ((startmonth%100) == 1);

	Ref<Matrix> result = new Matrix (testdata.rows, testdata.cols);
	int firstMonth = (startmonth%100)-1; // Zero-based month
	for (int r=0; r<result->rows; r++)
		for (int c=0; c<result->cols; c++)
			result->get (r,c) = mData.get ((r+firstMonth)%12,c);

	return result;
}



//////////////////////////////////////////////////////////////////////////////////////
//                                                                                  //
// ----                  o                                          _               //
// |   )      ___                      ____  ___   ___       ____  / \              //
// |---  |/\ /   ) |   | |  __  |   | (     /   )  ___| |/\ (     /   \ |   |  ___  //
// |     |   |---   \ /  | /  \ |   |  \__  |---  (   | |    \__  |---|  \ /  (   \ //
// |     |    \__    V   | \__/  \__! ____)  \__   \__| |   ____) |   |   V    ---/ //
//                                                                             __/  //
//////////////////////////////////////////////////////////////////////////////////////

PreviousYearsAvg::PreviousYearsAvg (const StringMap& params) : PredictionStrategy ("PreviousYearsAvg") {
}

/*virtual*/ void PreviousYearsAvg::train (const Matrix& traindata, int startmonth) {
	ASSERT (traindata.rows >= 24);
	
	// One year
	mMonthlyAvg.make (12, traindata.cols);
	for (int r=0; r<mMonthlyAvg.rows; r++)
		for (int c=0; c<mMonthlyAvg.cols; c++) {
			// Calculate average of previous years
			float sum = 0.0;
			int year = 0;
			for (; traindata.rows-12+r-year*12 >= 0; year++)
				sum += traindata.get (traindata.rows-12+r-year*12, c);
			mMonthlyAvg.get(r,c) = sum/float(year);
		}
}

/*virtual*/ Ref<Matrix> PreviousYearsAvg::predict (const Matrix& testdata, int startmonth) const {
	ASSERT (mMonthlyAvg.rows>0);

	int firstMonth = (startmonth%100)-1; // Zero-based month
	Ref<Matrix> result = new Matrix (testdata.rows, testdata.cols);
	for (int r=0; r<result->rows; r++)
		for (int c=0; c<result->cols; c++)
			result->get(r,c) = mMonthlyAvg.get((firstMonth+r)%12, c);

	return result;
}



////////////////////////////////////////////////////////////////////////////////////
//   _                    |                 |   |                       | ----    //
//  / \  |      ____      |        |   ___  |\  |  ___             ___  | |   )   //
// /   \ |---  (      __  | |   | -+- /   ) | \ | /   ) |   | |/\  ___| | |---    //
// |---| |   )  \__  /  \ | |   |  |  |---  |  \| |---  |   | |   (   | | |       //
// |   | |__/  ____) \__/ |  \__!   \  \__  |   |  \__   \__! |    \__| | |     O //
////////////////////////////////////////////////////////////////////////////////////

AbsoluteNeuralPrediction::~AbsoluteNeuralPrediction	() {
	delete mpNetwork;
}

void AbsoluteNeuralPrediction::make (const StringMap& params) {
	mUseAllOutputs		= params["AbsoluteNeuralPrediction.useAllOutputs"].toInt();
	mUseAllInputs		= params["AbsoluteNeuralPrediction.useAllInputs"].toInt();
	ASSERT (mUseAllInputs || (!mUseAllInputs && !mUseAllOutputs));

	mGlobalEqualization	= params["AbsoluteNeuralPrediction.globalEqualization"].toInt();
	mHiddenTopology		= params["AbsoluteNeuralPrediction.hidden"];
	mpNetwork			= NULL;

	mParams = params;

	PredictionStrategy::make (params);
}

PatternSet* AbsoluteNeuralPrediction::makeSet (const Matrix& data, int startmonth) const {
	//TRACE2 ("Making pattern set from = %d rows, %d cols", data.rows, data.cols);
	
	int startmonthi = (startmonth % 100)-1;
	int inputs = data.cols;
	
	// Input patterns have inputs for each month (12) + inputmonths*attributes.
	PatternSet* result = new PatternSet (data.rows-inputMonths(),
										 12+inputMonths()*(mUseAllInputs? inputs:1),
										 mUseAllOutputs? inputs:1);

	// Copy the monthly attributes to the pattern set
	for (int p=0; p<result->patterns; p++) {
		// Zero the input month indicator flags
		for (int m=0; m<12; m++)
			result->set_input (p, m, 0);

		// Set input month indicators as 1
		for (int im=0; im<mInputMonths; im++)
			result->set_input (p, (startmonthi+p+im+12)%12, 1);
				
		// Copy input month values to inputs
		if (mUseAllInputs)
			for (int j=0; j<inputs; j++)
				for (int prm=0; prm<mInputMonths; prm++)
					result->set_input (p, 12+inputs*prm+j, data.get(p+prm, j));
		else {
			// Only one input variable
			for (int prm=0; prm<mInputMonths; prm++)
				result->set_input (p, 12+prm, data.get(p+prm, mVariable));
		}
		
		// Copy current month values to outputs
		if (mUseAllOutputs)
			for (int j=0; j<inputs; j++)
				result->set_output (p, j, data.get(p+mInputMonths, j));
		else
			// Only one output variable
			result->set_output (p, 0, data.get(p+mInputMonths, mVariable));
	}
	return result;
}

/*virtual*/ void AbsoluteNeuralPrediction::train (const Matrix& traindata, int startmonth) {
	//TRACE2 ("Training data = %d rows, %d cols", traindata.rows, traindata.cols);
	
	////////////////////////////////////////////////////////////////////////////////
	// Create network
	
	if (!mpNetwork) {
		mpNetwork = new ANNetwork;
		mpNetwork->make (format("%d%s%d", inputVariables(traindata.cols),
								(CONSTR) mHiddenTopology,
								outputVariables(traindata.cols)));
		mpNetwork->connectFullFfw (false);
	}

	// Initialize the network
	mpNetwork->init (0.5);

	////////////////////////////////////////////////////////////////////////////////
	// Prepare data

	// Equalize data
	Matrix equalized = traindata;
	if (!mpNetwork->getEqualizer()) {
		MatrixEqualizer* mequalizer = new MatrixEqualizer (new MinmaxEq(0.0, 1.0)); // new HistogramEq (100000, 0.0, 1.0)
		mequalizer->analyze (traindata, mGlobalEqualization);
		mpNetwork->setEqualizer (mequalizer);
	}
	dynamic_cast<MatrixEqualizer*>(mpNetwork->getEqualizer())->equalize (equalized);

	// Create pattern set
	PatternSet* trainset = makeSet (equalized, startmonth);
	//trainset->print ();
	//fprintf (stderr, "Training data has %d patterns with %d inputs and %d outputs\n",
	//trainset->patterns, trainset->inputs, trainset->outputs);

	//PatternSet* trainset2 = makeSet (traindata, startmonth);
	//delete trainset2;
	
	////////////////////////////////////////////////////////////////////////////////
	// Train

	// Create trainer and set parameters
	RPropTrainer trainer;
	trainer.init (mParams);
	trainer.setTerminator (mParams["terminator"]);

	// Set observer to display current training cycle
	if (rpObserver)
		trainer.setObserver (rpObserver);

	// Train
	trainer.train (*mpNetwork, *trainset, mParams["maxCycles"].toInt(),
				   NULL, mParams["validationInterval"].toInt());

	//fprintf (stderr, "Trained for %d cycles, MSE=%f\n", trainer.totalCycles(), trainmse);
	delete trainset;
}

/*virtual*/ Ref<Matrix> AbsoluteNeuralPrediction::predict (const Matrix& testdata, int startmonth) const {
	//TRACE2 ("Test data = %d rows, %d cols", testdata.rows, testdata.cols);
	
	// Equalize
	Matrix equalized = testdata;
	dynamic_cast<MatrixEqualizer*>(mpNetwork->getEqualizer())->equalize (equalized);
	
	// Create pattern set and use it
	PatternSet* testset = makeSet (equalized, startmonth);

	////////////////////////////////////////////////////////////////////////////////
	// Test the network

	// Create a matrix where to place the results
	Ref<Matrix> result = new Matrix (testset->patterns, mUseAllOutputs? testset->outputs:1);

	// Test one month at a time
	for (int p=0; p<testset->patterns; p++) {
		Vector v = mpNetwork->testPattern (*testset, p);
		for (int i=0; i<v.size(); i++)
			result->get(p,i) = v[i];
	}
	delete testset;

	// Unequalize the results to get money values again
	dynamic_cast<MatrixEqualizer*>(mpNetwork->getEqualizer())->unequalize (result);

	return result;
}

void AbsoluteNeuralPrediction::load (TextIStream& in) {
	delete mpNetwork;

	mpNetwork = new ANNetwork();
	ANNFileFormatLib::load (in, *mpNetwork, "SNNS");
}

void AbsoluteNeuralPrediction::save (TextOStream& out) const {
	ANNFileFormatLib::save (out, *mpNetwork, "SNNS");
}



///////////////////////////////////////////////////////////////////////////////
//      ---- o             |       |   |                       | ----        //
//     (         _         |  ___  |\  |  ___             ___  | |   )       //
//      ---  | |/ \   ___  | /   ) | \ | /   ) |   | |/\  ___| | |---        //
//         ) | |   | (   \ | |---  |  \| |---  |   | |   (   | | |           //
//     ___/  | |   |  ---/ |  \__  |   |  \__   \__! |    \__| | |     O     //
//                    __/                                                    //
///////////////////////////////////////////////////////////////////////////////

void SingleNeuralPrediction::make (const StringMap& params) {
	AbsoluteNeuralPrediction::make (params);
}

/*virtual*/ void SingleNeuralPrediction::train (const Matrix& traindata, int startmonth) {
	for (int v=0; v<mNetworks.size(); v++)
		mNetworks.cut (v);
	mNetworks.empty ();
	mNetworks.make (traindata.cols);

	// Train each variable separately
	sout.autoFlush ();
	for (int v=0; v<traindata.cols; v++) {
		sout.printf ("Training variable %d...\n", v);
		mVariable = v;
		AbsoluteNeuralPrediction::train (traindata, startmonth);
		mNetworks.put (mpNetwork, v);
		mpNetwork=NULL;
	}
	sout.printf("\n");
}

/*virtual*/ Ref<Matrix> SingleNeuralPrediction::predict (const Matrix& testdata, int startmonth) const {
	SingleNeuralPrediction* ncthis = const_cast<SingleNeuralPrediction*>(this);

	Ref<Matrix> result = new Matrix ();

	// For each variable
	for (int v=0; v<testdata.cols; v++) {
		ncthis->mVariable = v;

		// Use a small kludge to set the current network
		ncthis->mpNetwork = const_cast<ANNetwork*>(&mNetworks[v]);

		// Let the baseclass do the prediction for one variable
		Ref<Matrix> oneVariable = AbsoluteNeuralPrediction::predict (testdata, startmonth);

		// Create the result matrix when we get the number of rows.
		if (result->rows==0)
			result->make (oneVariable->rows, testdata.cols);
		
		// Copy the one variable to the total result
		for (int r=0; r<result->rows; r++)
			result->get (r, v) = oneVariable->get (r, 0);
	}
	ncthis->mpNetwork = NULL;

	return result;
}

void SingleNeuralPrediction::load (TextIStream& in) {
	mNetworks.empty ();

	// Read the number of networks
	int nets;
	in >> nets;
	if (nets==0)
		throw invalid_format ("Invalid network file format (SingleNeuralPrediction)");

	mNetworks.make (nets);
	for (int i=0; i<nets; i++) {
		mNetworks.put (new ANNetwork(), i);
		ANNFileFormatLib::load (in, mNetworks[i], "SNNS");
	}
}

void SingleNeuralPrediction::save (TextOStream& out) const {
	out << mNetworks.size() << "\n";
	for (int i=0; i<mNetworks.size(); i++)
		ANNFileFormatLib::save (out, mNetworks[i], "SNNS");
}



///////////////////////////////////////////////////////////////////////////////
//       ___         |           |   |                       | ----          //
//       |  \   ___  |  |   ___  |\  |  ___             ___  | |   )         //
//       |   | /   ) | -+-  ___| | \ | /   ) |   | |/\  ___| | |---          //
//       |   | |---  |  |  (   | |  \| |---  |   | |   (   | | |             //
//       |__/   \__  |   \  \__| |   |  \__   \__! |    \__| | |     O       //
///////////////////////////////////////////////////////////////////////////////

DeltaNeuralPrediction::DeltaNeuralPrediction (const StringMap& params) : PredictionStrategy ("DeltaNeural")
{
}

/*virtual*/ void DeltaNeuralPrediction::train (const Matrix& traindata, int startmonth)
{
}

/*virtual*/ Ref<Matrix> DeltaNeuralPrediction::predict (const Matrix& testdata, int startmonth) const
{
	return Ref<Matrix> (NULL);
}



///////////////////////////////////////////////////////////////////////////////////////////////////
//   _             | o     o              ----                                     |     o       //
//  / \            |    |      _         (      |       ___   |   ___              |       |     //
// /   \ |   |  ---| | -+- | |/ \   ___   ---  -+- |/\  ___| -+- /   )  ___  \   | |     | |---  //
// |---| |   | (   | |  |  | |   | (   \     )  |  |   (   |  |  |---  (   \  \  | |     | |   ) //
// |   |  \__!  ---| |   \ | |   |  ---/ ___/    \ |    \__|   \  \__   ---/   \_/ |____ | |__/  //
//                                  __/                                 __/   \_/                //
///////////////////////////////////////////////////////////////////////////////////////////////////

PredictionStrategyLib PredictionStrategyLib::sInstance;

PredictionStrategyLib::PredictionStrategyLib ()
{
	registerCls ("AverageDeltaPrediction");
	registerCls ("ZeroDeltaPrediction");
	registerCls ("AbsoluteNeuralPrediction");
	registerCls ("SingleNeuralPrediction");
	registerCls ("PreviousYear");
	registerCls ("PreviousYearsAvg");
	registerCls ("CombinedPrediction");
	//registerCls ("DeltaNeuralPrediction");
}

PredictionStrategy* PredictionStrategyLib::create (int i) const
{
	return dynamic_cast<PredictionStrategy*> (dyncreate (mClassNames[i]));
}



///////////////////////////////////////////////////////////////////////////////////
// -----           o       ----                                                  //
//   |        ___      _   |   )  ___       ___         ___   |   ___       ____ //
//   |   |/\  ___| | |/ \  |---   ___| |/\  ___| |/|/| /   ) -+- /   ) |/\ (     //
//   |   |   (   | | |   | |     (   | |   (   | | | | |---   |  |---  |    \__  //
//   |   |    \__| | |   | |      \__| |    \__| | | |  \__    \  \__  |   ____) //
///////////////////////////////////////////////////////////////////////////////////

void TrainParameters::defaults ()
{
  	trainCycles    = 10000;
  	delta0         = 0.1;
  	deltaMax       = 50;
  	weightDecay    = 0.99999;
  	useWeightDecay = true;
	hidden1        = 10;
	hidden2        = 5;
	hidden3        = 0;
	runs           = 1;
	allInputs      = true;
	allOutputs     = true;
	monthInputs    = true;
	equalizeGlobal = true;
	inputMonths    = 1;
}

String TrainParameters::hiddenString () const
{
	String result = "-";
	if (hidden1 > 0)
		result += String(hidden1) + "-";
	if (hidden2 > 0)
		result += String(hidden2) + "-";
	if (hidden3 > 0)
		result += String(hidden3) + "-";
	return result;
}

Ref<StringMap> TrainParameters::getParams () const
{
	Ref<StringMap> resultRef = new StringMap ();
	StringMap& result = resultRef.object();

	result.set ("trainCycles", String(trainCycles));
	result.set ("delta0", String(delta0));
	result.set ("deltaMax", String(deltaMax));
	result.set ("weightDecay", String(weightDecay));
	result.set ("useWeightDecay", String(useWeightDecay));
	result.set ("hidden1", String(hidden1));
	result.set ("hidden2", String(hidden2));
	result.set ("hidden3", String(hidden3));
	result.set ("runs", String(runs));
	result.set ("allInputs", String(allInputs));
	result.set ("allOutputs", String(allOutputs));
	result.set ("monthInputs", String(monthInputs));
	result.set ("equalizeGlobal", String(equalizeGlobal));
	result.set ("inputMonths", String(inputMonths));

	return resultRef;
}

void TrainParameters::setParams (const StringMap& map)
{
	trainCycles    = map["trainCycles"].toInt();
	delta0         = map["delta0"].toFloat();
	deltaMax       = map["deltaMax"].toFloat();
	weightDecay    = map["weightDecay"].toFloat();
	useWeightDecay = map["useWeightDecay"].toInt();
	hidden1        = map["hidden1"].toInt();
	hidden2        = map["hidden2"].toInt();
	hidden3        = map["hidden3"].toInt();
	runs           = map["runs"].toInt();
	allInputs      = map["allInputs"].toInt();
	allOutputs     = map["allOutputs"].toInt();
	monthInputs    = map["monthInputs"].toInt();
	equalizeGlobal = map["equalizeGlobal"].toInt();
	inputMonths    = map["inputMonths"].toInt();
}

void TrainParameters::write (TextOStream& out) const
{
	writeStringMap (getParams(), out);
	out << "# end-of-map\n";
}

void TrainParameters::read (TextIStream& in)
{
	setParams (readStringMap (in));
}

