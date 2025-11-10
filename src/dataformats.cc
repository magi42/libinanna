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

#include <ctype.h>
#include <magic/mobject.h>
#include <magic/mlist.h>
#include <magic/mmap.h>
#include <magic/mregexp.h>
#include <magic/mtextstream.h>

#include "inanna/dataformat.h"
#include "inanna/dataformats.h"


//////////////////////////////////////////////////////////////////////////////////
//  ---- |   | |   |  ---- ___                   -----                          //
// (     |\  | |\  | (     |  \   ___   |   ___  |                     ___   |  //
//  ---  | \ | | \ |  ---  |   |  ___| -+-  ___| |---   __  |/\ |/|/|  ___| -+- //
//     ) |  \| |  \|     ) |   | (   |  |  (   | |     /  \ |   | | | (   |  |  //
// ___/  |   | |   | ___/  |__/   \__|   \  \__| |     \__/ |   | | |  \__|   \ //
//////////////////////////////////////////////////////////////////////////////////

struct PatternRow {
	Vector	values;
	String	comment;
	PatternRow (int n) {values.make(n);}
};

void SNNSDataFormat::load (TextIStream& in, PatternSet& set) const {

	// Some nice regular expressions to extract data from SNNS info lines
	RegExp no_patts	("patterns : (.+)");
	RegExp no_ins	("input units : (.+)");
	RegExp no_outs	("output units : (.+)");
	RegExp inpatt	("Input pattern (.+):");
	RegExp outpatt	("Output pattern (.+):");
	
	// State variables
	enum states {ST_NONE		= 0,
				 ST_INS			= 1,
				 ST_OUTS		= 2,
				 ST_NONE_INS	= 3,
				 ST_NONE_OUTS	= 4
				};
	int				state	= ST_NONE;	// Nice two bit flip-flop state
	int				readcnt	= 0;		// Amount of values in readarr
	String			buff;				// Input buffer
	Array<String>	linesubs;			// Subexpressions
	int				pattern	= 0;		// Pattern counter: current pattern
	int				patts	= 0;
	int				ins		= 0;
	int				outs	= 0;
	
	while (in.readLine(buff)) {

		if (isdigit (buff[0])) {
			if (!set.inputs || !set.outputs || !set.patterns)
				throw invalid_format ("Pattern file dimensions not given anywhere");

			// An interestingly strange state machine. I wonder how it
			// works. This is propably a good example of my
			// "evolutionary programming style", which means trying
			// until it works and not thinking. Or, maybe, this time I
			// superceded my thinking ability; my subconscious
			// intelligence must be enormous.
			switch (state) { // PLEASE NOTE THE CUTE FALLTHROUGHS! (They are really significant)
			  case ST_NONE_OUTS:
				  if (pattern>0 && readcnt!=set.outputs)
					  throw invalid_format (format ("Too short output vector #d in SNNS "
													"pattern set", pattern));
				  pattern++;
			  case ST_NONE:
				  state = ST_INS;
				  readcnt=0;
			  case ST_INS:
				  if (readcnt<set.inputs)
					  set.set_input (pattern, readcnt++, double(String (buff).toDouble()));
				  break;
			  case ST_NONE_INS:
				  if (pattern>0 && readcnt!=set.inputs)
					  throw invalid_format (format ("Too short input vector #d in SNNS "
													"pattern set", pattern));
				  state = ST_OUTS;
				  readcnt = 0;
			  case ST_OUTS:
				  if (readcnt<set.outputs)
					  set.set_output (pattern, readcnt++, double (String (buff).toDouble()));
			}
			
		} else {
			// The line seems to be text, read the rest of the line
			in.readLine (buff);

			// Extract number of patterns
			if (no_patts.match (buff, linesubs)) {
				patts = linesubs[1].toInt();
				if (set.patterns && patts!=set.patterns)
					throw invalid_format ("Pattern file has wrong dimensions");
			}

			// Extract number of inputs
			if (no_ins.match (buff, linesubs)) {
				ins = linesubs[1].toInt();
				if (set.inputs && ins!=set.inputs)
					throw invalid_format ("Pattern file has wrong dimensions");
			}

			// Extract number of outputs
			if (no_outs.match (buff, linesubs)) {
				outs = linesubs[1].toInt();
				if (set.outputs && outs!=set.outputs)
					throw invalid_format ("Pattern file has wrong dimensions");
			}

			// If we have read the set dimensions from the file, but
			// the set has not yet dimensions, create it.
			if ((!set.inputs || !set.outputs) && ins && outs && set.patterns)
				set.make (patts, ins, outs);

			// Jump to the expect states
			if (state==ST_INS)
				state = ST_NONE_INS;
			if (state==ST_OUTS)
				state = ST_NONE_OUTS;
		}
	}

	if (pattern!=set.patterns-1)
		throw invalid_format (format ("Wrong number of patterns in SNNS pattern file "
									  "(found %d of %d", pattern, set.patterns));
}

void SNNSDataFormat::save (FILE* out, const PatternSet& set) const {
	fprintf (out, "SNNS pattern definition file V3.2\n"
			 "generated at Xxxxx time\n\n\n"
			 "No. of patterns : %d\n"
			 "No. of input units : %d\n"
			 "No. of output units : %d\n\n",
			 set.patterns, set.inputs, set.outputs);

	for (int p=0; p<set.patterns; p++) {
		fprintf (out, "# input %d\n", p);
		for (int i=0; i<set.inputs; i++) {
			if (i>0)
				fprintf (out, " ");
			if (is_undef (set.input (p,i)))
				fprintf (out, "0");
			else 
				fprintf (out, "%g", set.input (p,i));
		}
		fprintf (out, "\n# target %d\n", p);
		for (int i=0; i<set.outputs; i++) {
			fprintf (out, " ");
			if (is_undef (set.output (p,i)))
				fprintf (out, "0");
			else 
				fprintf (out, "%g", set.output (p,i));
		}
		fprintf (out, "\n");
	}
}


///////////////////////////////////////////////////////////////////////////////////////////
// ----                             ___                   -----                          //
// |   )          |      ___    _   |  \   ___   |   ___  |                     ___   |  //
// |---  |/\  __  |---  /   ) |/ \  |   |  ___| -+-  ___| |---   __  |/\ |/|/|  ___| -+- //
// |     |   /  \ |   ) |---  |   | |   | (   |  |  (   | |     /  \ |   | | | (   |  |  //
// |     |   \__/ |__/   \__  |   | |__/   \__|   \  \__| |     \__/ |   | | |  \__|   \ //
///////////////////////////////////////////////////////////////////////////////////////////

void Proben1DataFormat::load (TextIStream& in, PatternSet& set) const
{
	String buff;	// Read buffer
	
	// Read header
	String header;
	header.reserve (256);
	for (int i=0; i<7; i++) {
		in.readLine (buff);
		buff.chop();
		ASSERT (buff.length()>0);
		ASSERT (buff.find ("=")!=-1);
		header += buff;
		header += '&';
	}

	// Split the header into a map
	StringMap map;
	splitpairs (map, header, '=', '&');

	// Check that all required fields exist
	bool kludge=false;
	if (isnull(map["bool_in"])) kludge=true;
	if (isnull(map["real_in"])) kludge=true;
	if (isnull(map["bool_out"])) kludge=true;
	if (isnull(map["real_out"])) kludge=true;
	if (isnull(map["training_examples"])) kludge=true;
	if (isnull(map["validation_examples"])) kludge=true;
	if (isnull(map["test_examples"])) kludge=true;
	if (kludge)
		throw generic_exception ("Some required line(s) were missing from a "
								 "Proben1 data file header");

	// Extract the number of patterns, inputs and outputs
	int ins  = map["bool_in"].toInt() + map["real_in"].toInt();
	int outs = map["bool_out"].toInt() + map["real_out"].toInt();
	int pats = map["training_examples"].toInt() + map["validation_examples"].toInt()
		+ map["test_examples"].toInt();

	// Make my day. Oops. ^H^H^H^H^H^H^H^H^Hmyself
	set.make (pats, ins, outs);

	// Read the patterns
	for (int p=0; p<pats; p++) {
		// Read one line
		in.readLine (buff);
		buff.chop ();

		// Split it to fields (MUST be delimited with one space)
		Array<String> items;
		buff.split (items, ' ');

		// Count the number of items
		int pos=0;
		Array<String> fields (items.size());
		for (int i=0; i<items.size(); i++)
			if (items[i].length()>0)
				fields[pos++] = items[i];
		fields.resize (pos);
		
		ASSERTWITH (fields.size()==set.inputs+set.outputs,
					format ("Wrong number of fields (%d) in a Proben1 file "
							"(%d was expected).\n"
							"Propably due to bad formatting in line:\n"
							"%s\n"
							"Rules: number of fields must be equal to "
							"the sum of inputs and outputs and they \n"
							"must be separated by spaces (ASC32). No "
							"other characters are allowed\n",
							fields.size(), set.inputs+set.outputs, (CONSTR) buff));

		// Extract input values
		for (int i=0; i<ins; i++)
			set.set_input (p,i, fields[i].toDouble());

		// Extract output values
		for (int i=0; i<outs; i++)
			set.set_output (p,i, fields[i+ins].toDouble());
	}

	// Store sizes to dynamic attributes
	set.setAttribute ("training_examples", new Int (map["training_examples"].toInt()));
	set.setAttribute ("validation_examples", new Int (map["validation_examples"].toInt()));
	set.setAttribute ("test_examples", new Int (map["test_examples"].toInt()));
}



///////////////////////////////////////////////////////////////////////////////
//  ----               ___                   -----                           //
//  |   )  ___         |  \   ___   |   ___  |                     ___   |   //
//  |---   ___| \    / |   |  ___| -+-  ___| |---   __  |/\ |/|/|  ___| -+-  //
//  | \   (   |  \\//  |   | (   |  |  (   | |     /  \ |   | | | (   |  |   //
//  |  \   \__|   VV   |__/   \__|   \  \__| |     \__/ |   | | |  \__|   \  //
///////////////////////////////////////////////////////////////////////////////

void RawDataFormat::load (TextIStream& in, PatternSet& set) const
{
	List<PatternRow*> patlist;	// Will contain the rows in the input file
	int patNum=0;				// Number of patterns in the input file
	bool comments=false;		// Are there any comments found in the patterns rows?

	// Read the datafile
	String buffer;
	while (in.readLine (buffer)) { // Read one line until EOF
		PatternRow* prow = new PatternRow (set.inputs? set.inputs+set.outputs : 1);
		String item;		// One numeric or text item
		int itemIndex=0;	// Index of the above item within the row

		// Parse the row
		for (uint i=0; i <= buffer.length(); i++) {
			// Read one extra whitespace, to make parsing easier
			char c = (i==buffer.length())? '\n' : buffer[i];

			// If we have a field break, or End-Of-Line
			if (isspace (c)) {
				if (item.length()>0) { // Fields can be separated by more than one whitespace
					// If we don't know the number of columns, we must
					// grow it by one
					if (itemIndex >= set.inputs + set.outputs)
						prow->values.resize (itemIndex+1);
					
					// Copy parsed value to row vector
					if (set.inputs==0 || itemIndex < set.inputs+set.outputs)
						prow->values[itemIndex] = (item=="x")? UNDEFINED_FLOAT : item.toDouble();
					item="";
					
					itemIndex++;
				}
				// Check for beginning of comment
			} else if (item.length()==0 && !isdigit(c) && c!='x' && c!='-' && c!='+' && c!='.') {
				// The rest of the row is comment text
				prow->comment = buffer.mid (i);
				comments = true;
				break;
			} else // Field 
				item += c;
		}

		// Remove empty field at end-of-the-line
		while (itemIndex>0 && isempty (String(prow->values[itemIndex-1])))
			--itemIndex;

		// Add row only if it has fields
		if (itemIndex>0) {
			// If we didn't know the number of inputs before, we know it
			// after reading the first row.
			if (set.inputs==0)
				set.inputs = itemIndex;

			// Add the row only if it had right number of fields
			if (itemIndex == set.inputs + set.outputs)
				patlist.add (prow);
			else {
				delete prow;
				throw invalid_format (strformat ("Pattern set row %d has invalid number of fields (%d out of %d+%d)",
												 patNum+1, itemIndex, set.inputs, set.outputs));
			}
			
			patNum++;
		} else
			// Empty line
			delete prow;
	}

	// Adjust set size
	set.make (patNum, set.inputs, set.outputs);

	// Put the pattern list into the pattern array
	int p=0;
	for (ListIter<PatternRow*> l (patlist); !l.exhausted(); l.next(), p++) {
		for (int i=0; i<set.inputs; i++)
			set.set_input (p, i, l.get()->values[i]);
		for (int i=0; i<set.outputs; i++)
			set.set_output (p, i, l.get()->values[i+set.inputs]);
	}

	// Put comments as an attribute extension
	if (comments) {
		Array<String>* comments = new Array<String> (patNum);
		int p=0;
		for (ListIter<PatternRow*> l (patlist); !l.exhausted(); l.next(), p++)
			(*comments)[p] = l.get()->comment;
		set.setAttribute ("comments", comments);
	}

	// Delete the list. This is a bit strange.
	for (ListIter<PatternRow*> l (patlist); !l.exhausted(); l.next(), p++)
		delete l.get();
}
 
void RawDataFormat::save (FILE* out, const PatternSet& set) const
{
	for (int p=0; p<set.patterns; p++) {
		for (int i=0; i<set.inputs; i++) {
			if (i>0)
				fprintf (out, " ");
			if (is_undef (set.input (p,i)))
				fprintf (out, "x");
			else 
				fprintf (out, "%g", set.input (p,i));
		}
		for (int i=0; i<set.outputs; i++) {
			fprintf (out, " ");
			if (is_undef (set.output (p,i)))
				fprintf (out, "x");
			else 
				fprintf (out, "%g", set.output (p,i));
		}
		if (!isnull(set.getAttribute("comments")))
			fprintf (out, " %s", (CONSTR) dynamic_cast<const Array<String>&> (set.getAttribute("comments"))[p]);
		fprintf (out, "\n");
	}
}

