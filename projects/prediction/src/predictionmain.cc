/***************************************************************************
 *   This is a command-line prediction software for Inanna.                *
 *                                                                         *
 *   Copyright (C) 1998-2004 Marko Grönroos <magi@iki.fi>                  *
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <magic/mapplic.h>
#include <magic/mtextstream.h>

#include <inanna/patternset.h>
#include <inanna/trainer.h>
#include <inanna/prediction.h>

class ReportObserver : public TrainingObserver {
  public:
	void cycleTrained (const Trainer& trainer, int totalCycles) {
		printf ("Cycles trained: %d\r", totalCycles);
		fflush (stdout);
	}
};

/*******************************************************************************
 * Runs the learning algorithm and makes the predictions
 ******************************************************************************/
void predict (
	PredictionStrategy*           strategy,
	Matrix&                       alldata,
	Matrix&                       traindata,
	Matrix&                       testdata,
	int                           runs,
	Array<PredictionTestResults>& allResults,
	Matrix&                       prediction)
{
	int testmonth = int (alldata.get(alldata.rows - 12 - strategy->inputMonths(),0)+0.1);

	for (int r=0; r<prediction.rows; r++)
		for (int c=0; c<prediction.cols; c++)
			prediction.get(0,0) = 0.0;

	// Make training-testing runs and average results
	for (int i=0; i<runs; i++) {
		printf ("Run %d\n", i);
		
		// Train the strategy
		strategy->train (traindata, int (alldata.get(0,0)+0.01));
		
		// Test the strategy numerically
		PredictionTestResults* results = strategy->test (testdata, testmonth);
		allResults.add (results);
		sout.printf ("Average error: %f\n", results->averageError);
		for (int v=0; v<results->averageErrors.size(); v++)
			sout.printf ("Variable %d: %f\n", v, results->averageErrors[v]);
		
		// If ADP, save some minor results
		if (AverageDeltaPrediction* adp = dynamic_cast<AverageDeltaPrediction*>(strategy)) {
			String adp_resultfile = "results/avgDeltas.mat";
			FILE* out = fopen (adp_resultfile, "w");
			if (! out)
				throw open_failure (strformat ("Could not open output file '%s' for writing.", (CONSTR) adp_resultfile));
				
			adp->deltas().save (out);
			fclose (out);
		}

		// Make prediction
		prediction += strategy->predict (testdata, testmonth);
	}
	prediction /= runs;
}

/*******************************************************************************
 * Writes the prediction results to an output file.
 ******************************************************************************/
void writePrediction (
	Matrix& prediction,
	Matrix& alldata,
	const String& resultfile)
{
	printf ("Prediction matrix, %dx%d\n", prediction.rows, prediction.cols);
	FILE* out = fopen(resultfile, "w");
	if (!out)
		throw open_failure (strformat ("Could not open output file '%s' for writing.", (CONSTR) resultfile));

	for (int r=0; r<prediction.rows; r++) {
		fprintf (out, "%d ", int (alldata.get(alldata.rows-12+r,0)));
		for (int c=0; c<prediction.cols; c++)
			fprintf (out, "%f ", prediction.get(r,c));
		fprintf (out, "\n");
	}
	fclose (out);
	printf ("Written to '%s'.\n", (CONSTR) resultfile);
}

/*******************************************************************************
 * Writes prediction test reports to standard output and to result file.
 ******************************************************************************/
void report (
	Array<PredictionTestResults>& allResults,
	Matrix&                       prediction,
	Matrix&                       alldata,
	PredictionStrategy*           strategy)
{
	// Calculate averages and stddevs for the results
	Vector averageErrors (allResults.size());								// For all variables
	Array<Vector> averageVarErrors (allResults[0].averageErrors.size());	// For each variable
	for (int i=0; i<averageVarErrors.size(); i++)			// Create the vectors
		averageVarErrors[i].make (allResults.size());
	for (int i=0; i<allResults.size(); i++) {				// Put the averages into the vectors
		averageErrors[i] = allResults[i].averageError;
		for (int j=0; j<averageVarErrors.size(); j++)
			averageVarErrors[j][i] = allResults[i].averageErrors[j];
	}

	// Print statistics
	printf ("Average error for all inputs and runs: %f (stddev %f)\n",
			avg (averageErrors), stddev(averageErrors));
	printf ("Average error for each input for all runs:\n");
	for (int j=0; j<averageVarErrors.size(); j++)
		printf ("Variable %d: %f (stddev %f)\n",
				j, avg (averageVarErrors[j]), stddev (averageVarErrors[j]));

	// Write prediction to a file
	writePrediction (prediction,
					 alldata,
					 strformat("results/%s.res", (CONSTR)strategy->name()));
}

///////////////////////////////////////////////////////////////////////////////
//                            |   |       o                                  //
//                            |\ /|  ___      _                              //
//                            | V |  ___| | |/ \                             //
//                            | | | (   | | |   |                            //
//                            |   |  \__| | |   |                            //
///////////////////////////////////////////////////////////////////////////////

Main ()
{
	// assertmode = ASSERT_CRASH;
	sout.autoFlush ();

	readConfig ("neuroprediction.cfg");

	//////////////////////////////////////////////////////////////////////
	// PREPARE STRATEGY

	// Create auditing strategy
	PredictionStrategyLib& audStr = PredictionStrategyLib::instance();

	int strategyNum = paramMap()["strategy"].toInt();

	PredictionStrategy* strategy = audStr.create (strategyNum);

	strategy->make (paramMap());

	sout.printf ("Strategy: %s\n", (CONSTR) strategy->name());

	if (AbsoluteNeuralPrediction* anp = dynamic_cast<AbsoluteNeuralPrediction*>(strategy))
		anp->setObserver (new ReportObserver ());
	
	//////////////////////////////////////////////////////////////////////
	// PREPARE DATA

	// Load a data file
	PatternSet loaded (dataDir() + paramMap()["datafile"], 10, 0);
	fprintf (stderr, "Loaded %d samples of length %d\n", loaded.patterns, loaded.inputs+loaded.outputs);

	// Put data into matrix
	Ref<Matrix> alldataRef = loaded.getMatrix ();
	Matrix& alldata = alldataRef.object();

	sout << "Loaded=" << loaded << "\n";

	// Split it into training and test set
	Matrix traindata = alldata.sub (0,alldata.rows-13, 1, Matrix::end);
	Matrix testdata = alldata.sub (alldata.rows-12-strategy->inputMonths(), Matrix::end, 1, Matrix::end);
	
	fprintf (stderr, "Train data: %d patterns with %d values\n", traindata.rows, traindata.cols);
	fprintf (stderr, "Test  data: %d patterns with %d values\n", testdata.rows, testdata.cols);

	// Prepare result holders
	Array<PredictionTestResults> allResults;
	Matrix                       prediction (12, testdata.cols);

	// MAKE PREDICTIONS (train & test)
	predict (strategy,
			 alldata,
			 traindata,
			 testdata,
			 paramMap()["runs"].toInt(),
			 allResults,
			 prediction);

	// REPORTING
	report (allResults, prediction, alldata, strategy);
	
	// Finished
	delete strategy;
}

// http://www.neci.nj.nec.com/homepages/lawrence/papers/finance-tr96/latex.html
// http://www.nodes.de/PredTimeSeries.htm
