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

#include "fstream"

#include <magic/mobject.h>
#include <magic/mregexp.h>

#include "inanna/annfilef.h"
#include "inanna/equalization.h"



////////////////////////////////////////////////////////////////////////////////////
//   _   |   | |   | ----- o |       -----                          |     o       //
//  / \  |\  | |\  | |       |  ___  |                     ___   |  |       |     //
// /   \ | \ | | \ | |---  | | /   ) |---   __  |/\ |/|/|  ___| -+- |     | |---  //
// |---| |  \| |  \| |     | | |---  |     /  \ |   | | | (   |  |  |     | |   ) //
// |   | |   | |   | |     | |  \__  |     \__/ |   | | |  \__|   \ |____ | |__/  //
////////////////////////////////////////////////////////////////////////////////////


/*******************************************************************************
 * Creates a network from the given file.
 *
 * @throws file_not_found, invalid_format, assertion_failed
 ******************************************************************************/
void ANNFileFormatLib::load (const String& filename,
							 ANNetwork& set)
	throw (file_not_found, invalid_format, assertion_failed, open_failure, stream_failure)
{
	ASSERTWITH (!isempty(filename), "Filename required (was empty)");

	// Open the file
	TextIStream* in = &stin;
	if (filename != "-") {
		in = new TextIStream (new File (filename, IO_Readable));
		if (!*in)
			throw file_not_found (format ("Network file '%s' not found", (CONSTR) filename));
	}

	// TODO: Support other filetypes than SNNS
	load (*in, set, "SNNS");

	if (in != &stin)
		delete in;
}

/*******************************************************************************
 * Tries to load the given file to the given network.
 *
 * @throws file_not_found, invalid_format, assertion_failed
 ******************************************************************************/
void ANNFileFormatLib::load (TextIStream& in,
							 ANNetwork& net,
							 const String& filetype)
	throw (invalid_format, assertion_failed, stream_failure)
{
	ANNFileFormat* handler = create (filetype);
	try {
		handler->load (in, net);
	} catch (...) {
		delete handler; // Clean object
		throw; // Rethrow
	}
	delete handler;
}

/*******************************************************************************
 * Writes the given network to file with the given name. File format
 * can also be specified.
 *
 *  @throws file_not_found, invalid_format, assertion_failed
 ******************************************************************************/
void ANNFileFormatLib::save (
	const String&    filename,		/**< Network save file name. */
	const ANNetwork& net,			/**< Network object to be saved. */
	const char*      fileformat)	/**< File format. */
	throw (invalid_filename, invalid_format, assertion_failed, stream_failure, open_failure)
{
	// Default to standard output stream
	TextOStream* out = &sout;

	// If a filename is given, open it for writing.
	if (filename != "-")
		out = new TextOStream (new File (filename, IO_Writable));

	// TODO: This is a silly check.
	if (! out)
		throw open_failure (strformat (i18n("Network file '%s' couldn't be opened for writing"),
									   (CONSTR) filename));

	try {
		// TODO: Make file format dynamic
		save (*out, net, fileformat);
	} catch (...) {
		// Delete the stream object, if created earlier.
		if (out != &sout)
			delete out;

		throw; // Re-throw
	}

	// Delete the stream object, if created earlier.
	if (out != &sout)
		delete out;
}

/*******************************************************************************
 * Writes a network to a stream.
 ******************************************************************************/
void ANNFileFormatLib::save (
	TextOStream&     out,       /**< Output stream to write to. */
	const ANNetwork& net,       /**< Network to serialize.      */
	const String&    filetype)  /**< File format to use.        */
	throw (assertion_failed, invalid_filename, stream_failure, invalid_format)
{
	// Create the file format handler.
	ANNFileFormat* handler = create (filetype);
	
	try {
		handler->save (out, net);
	} catch (must_overload e) {
		delete handler;
		throw invalid_filename (format (i18n("Save-to-file operation not supported for file type\n%s"),
										(CONSTR) e.what()));
	} catch (...) {
		delete handler; // Clean object
		throw; // Rethrow
	}
	delete handler;
}


/*******************************************************************************
 * Factory creates a data format handler according to filename. To
 * make the factory more extensible, there would need to be some sort
 * of dynamic registry of Factory Methods.
 ******************************************************************************/
ANNFileFormat* ANNFileFormatLib::create (const String& fileformat) throw (invalid_format) {
	// Factory method, very trivial and rigid implementation.
    // TODO: Make more dynamic
	
	// Parse the contents according to the file name extension
	if (fileformat == "SNNS")
		return new SNNS_ANNFormat ();
	else
		throw invalid_format (i18n("Only SNNS (.net) file format supported currently"));
}



///////////////////////////////////////////////////////////////////////
//   _   |   | |   | ----- o |       -----                           //
//  / \  |\  | |\  | |       |  ___  |                     ___   |   //
// /   \ | \ | | \ | |---  | | /   ) |---   __  |/\ |/|/|  ___| -+-  //
// |---| |  \| |  \| |     | | |---  |     /  \ |   | | | (   |  |   //
// |   | |   | |   | |     | |  \__  |     \__/ |   | | |  \__|   \  //
///////////////////////////////////////////////////////////////////////

void SNNS_ANNFormat::load (TextIStream& in, ANNetwork& net) const
	throw (stream_failure, invalid_format)
{
	String str_units        ("units : ");
	String str_lfunc		("learning function : ");
	String str_ufunc		("update function   : ");
	String str_unitdefault	("unit default section");
	String str_unitdefin	("unit definition section");
	String str_conndefin	("connection definition section");
	String re_unitline		("^ +[0-9]+");
	String str_equalization	("Equalization:");
	String str_EON			("# end-of-network");

	net.empty ();

	String linebuf;
	//Array<String> linesubs;
	int state = 0;
	int inputs=0, hiddens=0, outputs=0;
	while (in.readLine (linebuf)) {
		linebuf = linebuf.stripWhiteSpace ();

		// Stop reading on End-Of-Network
		if (linebuf.left (str_EON.length()) == str_EON)
			break;

		switch (state) {
		  case 0:
			  // Network size
			  if (linebuf.left (str_units.length()) == str_units)
				  net.make (linebuf.mid(str_units.length()).toInt());
			  
			  // Learning function
			  if (linebuf.left (str_lfunc.length()) == str_lfunc) {
				  if (linebuf.mid(str_lfunc.length()) != "Rprop")
					  throw invalid_format (strformat("SNNS learning function '%s' not supported",
													  (CONSTR) linebuf.mid (str_lfunc.length())));
			  }
			  if (linebuf.left (str_unitdefault.length()) == str_unitdefault)
				  state = 1;
			  break;
			  
		  case 1: // Reading unit default section
			  if (linebuf.left (str_unitdefin.length()) == str_unitdefin)
				  state = 2;
			  break;
			  
		  case 2: // Reading unit definition section
			  // It's a unit definition line if it begins with a value
			  if (linebuf.length() > 0 && isdigit(linebuf[0])) {
				  // Extract unit fields
				  Array<String> unitfields;
				  linebuf.split (unitfields, '|');
				  for (int i=0; i<unitfields.size(); i++)
					  unitfields[i] = unitfields[i].stripWhiteSpace();
				  int unitid = unitfields[0].toInt()-1;

				  // Read activation and bias
				  net[unitid].setActivation (unitfields[3].toFloat());
				  net[unitid].setBias (unitfields[4].toFloat());

				  // Read unit type
				  if (unitfields[5].stripWhiteSpace()=="i")
					  inputs++;
				  else if (unitfields[5].stripWhiteSpace()=="h")
					  hiddens++;
				  else if (unitfields[5].stripWhiteSpace()=="o")
					  outputs++;

				  // Extract unit coordinates
				  Array<String> coords;
				  unitfields[6].split (coords, ',');
				  ASSERTWITH (coords.size()==3, "ANNetwork SNNS file must have 3-dimensional coordinates for units");
				  net[unitid].moveTo (coords[0].stripWhiteSpace().toFloat(),
									  coords[1].stripWhiteSpace().toFloat(),
									  coords[2].stripWhiteSpace().toFloat());
			  }
			  
			  if (linebuf.left (str_conndefin.length()) == str_conndefin)
				  state = 3;
			  break;
			  
		  case 3: // Reading connection definition section
			  // It's a connection definition line if it begins with a value
			  if (linebuf.length() > 0 && isdigit(linebuf[0])) {
				  // Extract connection fields
				  Array<String> unitfields;
				  linebuf.split (unitfields, '|');
				  int targetid = unitfields[0].stripWhiteSpace().toInt()-1;

				  // Extract connections to this target unit
				  Array<String> conns;
				  unitfields[2].split (conns, ',');
				  for (int i=0; i<conns.size(); i++) {
					  Array<String> connpair;
					  conns[i].split (connpair, ':');

					  // Make connection
					  Connection* newconn = net.connect (connpair[0].stripWhiteSpace().toInt()-1, targetid);

					  // Set conenction weight
					  newconn->setWeight (connpair[1].stripWhiteSpace().toFloat());
				  }
			  }
			  
			  if (linebuf.left (str_equalization.length()) == str_equalization && !net.getEqualizer()) {
				  // Read the '# ' in the beginning of the next row
				  in >> linebuf;

				  // Read the equalization object
				  net.setEqualizer (readEqualizer(in));
			  }
			  break;
		};
	}

	// Make topology handler according to parameters acquired
	const_cast<ANNLayering&>(dynamic_cast<const ANNLayering&>(net.getTopology())).make (format ("%d-%d-%d", inputs, hiddens, outputs));
}

void SNNS_ANNFormat::save (TextOStream& out,
						   const ANNetwork& net) const
	throw (stream_failure)
{
	// SNNS Version
	out << "SNNS network definition file V1.4-3D\n";
	out	<< "generated at <time>\n\n"
		<< "network name : Network\n"
		<< "source files :\n"
		<< "no. of units : " << net.size() << "\n"
		<< "no. of connections : 0\n"
		<< "no. of unit types : 0\n"
		<< "no. of site types : 0\n\n\n"
		<< "learning function : Rprop\n"
		<< "update function   : Topological_Order\n\n\n"
		<< "unit default section :\n\n"
		<< "act      | bias     | st | subnet | layer | act func     | out func\n"
		<< "---------|----------|----|--------|-------|--------------|-------------\n"
		<< " 0.00000 |  0.00000 | h  |      0 |     1 | Act_Logistic | Out_Identity\n"
		<< "---------|----------|----|--------|-------|--------------|-------------\n\n\n";

	// List units
	out << "unit definition section :\n\n"
		<< "no. | typeName | unitName | act      | bias     | st | position | act func             | out func | sites\n"
		<< "----|----------|----------|----------|----------|----|----------|----------------------|----------|-------\n";

	// Go through all units
	for (int i=0; i<net.size(); i++) {
		// Determine unit status
		char st='h'; // Hidden

		// The layering object tells the unit type (input/output/hidden)
		if (const ANNLayering* layering = dynamic_cast<const ANNLayering*> (&net.getTopology()))
			if (layering->layers() > 1)
				if (i < (*layering)[0])
					st = 'i';
				else if (i >= net.size()-(*layering)[-1])
					st = 'o';

		// Print unit line
		out << strformat ("%3d |          | unit     | % 3.5f | % 3.5f | %c  | %2g,%2g,%2g |||\n",
						  i+1, net[i].activation(), net[i].bias(), st,
						  net[i].getPlace().x, net[i].getPlace().y, net[i].getPlace().z);
	}
	out << "----|----------|----------|----------|----------|----|----------|----------------------|----------|-------\n";

	// List connections
	out << "\n\nconnection definition section :\n\n"
		<< "target | site | source:weight\n"
		<< "-------|------|---------------------------------------------------------------------------------------------------------------------\n";
	for (int i=0; i<net.size(); i++)
		if (net[i].incomings()>0) {
			out << strformat ("%6d |      |", i+1); // Target neuron ID
			for (int j=0; j<net[i].incomings(); j++) {
				if (j>0)
					out << ',';
				out << strformat ("%3d:% .5f",
								  net[i].incoming(j).source().id()+1, // Source neuron ID
								  net[i].incoming(j).weight()); // Connection weight
			}
			out << '\n';
		}
	out << "-------|------|---------------------------------------------------------------------------------------------------------------------\n";

	// Store equalization object
	if (const Equalizer* eq = net.getEqualizer()) {
		out << "\n# Equalization:\n"
			<< "# " << (*eq) << "\n";
	}

	out << "# end-of-network\n";
}
