/*******************************************************************************
*   This file is part of the Inanna library.                                   *
*                                                                              *
*   Copyright (C) 1998-2002 Marko Grönroos <magi@iki.fi>                       *
*                                                                              *
********************************************************************************
*                                                                              *
*   This program is free software; you can redistribute it and/or modify       *
*   it under the terms of the GNU General Public License as published by       *
*   the Free Software Foundation; either version 2 of the License, or          *
*   (at your option) any later version.                                        *
*                                                                              *
*******************************************************************************/

#include <fstream.h>

#include <magic/mobject.h>
#include <magic/mapplic.h>
#include "inanna/annetwork.h"
#include "inanna/annfilef.h"
#include "inanna/equalization.h"

////////////////////////////////////////////////////////////////////////////////

bool debug_new_test () {
	String* foo = new String ("kakka");
	(*foo) += "hiihaa";
	delete foo;

	Array<String> arr;
	arr.add (new String ("yksi"));
	arr.add (new String ("kaksi"));

	// Test dynamic_cast
	Object* obj = new String ("hii");
	String* bar = dynamic_cast<String*> (obj);
	delete obj;

	return true;
}

////////////////////////////////////////////////////////////////////////////////

ANNetwork* createNetwork () {
	ANNetwork* net = new ANNetwork;
	net->make ("10-10-10-5");
	net->connectFullFfw (true);
	net->init (0.5);
	return net;
}

bool testCreateDestroy () {
	ANNetwork* net = createNetwork ();
	delete net;
	return true;
}

////////////////////////////////////////////////////////////////////////////////

bool neuronOperations () {
	ANNetwork* net = new ANNetwork;

	for (int i=0; i<10; i++)
		net->add (new Neuron ());
	net->removeUnit (5);
	net->removeUnit (0);
	net->removeUnit (net->size-1);
	while (net->size > 0)
		net->removeUnit (0);

	net->make ("10-5-5");
	net->removeUnit (5);

	delete net;
	return true;
}

////////////////////////////////////////////////////////////////////////////////

bool connectionOperations (void) {
	ANNetwork net;

	for (int i=0; i<10; i++)
		net.add (new Neuron ());

	for (int i=0; i<10; i++)
		for (int j=0; j<10; j++)
			net.connect (i, j);

	net[5].disconnectTo (net[6]);
	net[7].disconnectFrom (net[7]);
	net[8].disconnectTo (net[8]);
	net[3].disconnectFrom (net[4]);
	net[2].disconnectAll ();
	net.removeUnit (5);
	net[5].disconnectTo (net[0]);
	net[5].disconnectFrom (net[1]);
	net[2].disconnectTo (net[5]);
	net[3].disconnectFrom (net[5]);

	ASSERT (net[2].incomings() == 0 && net[2].outgoings() == 0);

	return true;
}

////////////////////////////////////////////////////////////////////////////////

bool testSaveLoad (void)
{
	ANNetwork net ("10-5-10");
	net.connectFullFfw (true);
	net.init (0.5);
	TRACELINE;
	ANNFileFormatLib::save ("/tmp/testsave.net", net, "SNNS");
	TRACELINE;

	ANNetwork loadnet;
	TRACELINE;
	ANNFileFormatLib::load ("/tmp/testsave.net", loadnet);
	TRACELINE;
	ANNFileFormatLib::save ("/tmp/testsave2.net", loadnet, "SNNS");

	// Check that the files are equal
	if (system ("diff /tmp/testsave.net /tmp/testsave2.net > /dev/null"))
		return false;

	return true;
}

////////////////////////////////////////////////////////////////////////////////

// Creates a sample equalizer and analyzes some data
MatrixEqualizer* createEqualizer (void) {
	MatrixEqualizer* eq = new MatrixEqualizer (new MinmaxEq (0,1));

	// Give the equalizer some random data to equalize
	Matrix matrix (10,10);
	for (int i=0; i<10; i++)
		eq->analyze (matrix);

	return eq;
}

bool equalizerSaveLoad (void)
{
	MatrixEqualizer* eq = dynamic_cast<MatrixEqualizer*>(createEqualizer ());

	// Save
	TextOStream out (new File ("/tmp/testsaveeq.equ"));
	/* out.iword (xmlflag) = false; */ // Use XML?
	(*eq) >> out;
	out.device()->close ();

	// Load
	TextIStream in (new File ("/tmp/testsaveeq.equ"));
	MatrixEqualizer* eq2 = dynamic_cast<MatrixEqualizer*>(readEqualizer (in));
	if (eq2->planes() != eq->planes())
		throw Exception (strformat("Loading MatrixEqualizer failed. Wrong number of equalization planes: got %d, expected %d.",
								   eq2->planes(), eq->planes()));

	delete eq;
	delete eq2;
	return true;
}

////////////////////////////////////////////////////////////////////////////////

// Tries to save the equalizer with the network object
bool networkEqualizerSaveLoad (void) {
	ios::sync_with_stdio ();
	
	// Create network and equalizer
	ANNetwork* net = createNetwork ();
	net->setEqualizer (createEqualizer ());

	// Save the network (with the equalizer)
	ANNFileFormatLib::save ("/tmp/testsave.net", *net, "SNNS");

	// Load the network (with the equalizer)
	ANNetwork loadnet;
	ANNFileFormatLib::load ("/tmp/testsave.net", loadnet);

	// Verify preservation of equalizer
	MatrixEqualizer* eq = dynamic_cast<MatrixEqualizer*>(net->getEqualizer());
	MatrixEqualizer* eq2 = dynamic_cast<MatrixEqualizer*>(loadnet.getEqualizer());
	if (eq2->planes() != eq->planes())
		throw Exception (strformat("Loading MatrixEqualizer failed. Wrong number of equalization planes: got %d, expected %d.", eq2->planes(), eq->planes()));

	delete net;
	return true;
}

////////////////////////////////////////////////////////////////////////////////

int printout=true;

void testf (CONSTR funcname, bool (* func) ()) {
	if (printout)
		printf ("Calling %s...", funcname);
	if (printout)
		fflush (stdout);
	bool ok = func ();
	if (printout)
		printf (ok? "OK!\n" : "\nFAIL!\n");
}

#define test(fname) testf (#fname, fname)

Main () {
	//assertmode = ASSERT_CRASH;
	printf ("---------------------------------------------------\n");

	for (int i=0; i<1; i++) {
		test (debug_new_test);
		test (testCreateDestroy);
		test (neuronOperations);
		test (connectionOperations);
		test (testSaveLoad);
		test (equalizerSaveLoad);
		test (networkEqualizerSaveLoad);
		printout=false;
	}

	printf ("---------------------------------------------------\n");
	printf ("ANNetwork operations test program exiting...\n");
}
