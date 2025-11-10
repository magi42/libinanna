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

#include <magic/mclass.h>
#include <magic/mtextstream.h>

#include "inanna/neuron.h"
#include "inanna/connection.h"

impl_dynamic (Connection, {Object});


///////////////////////////////////////////////////////////////////////////////
//             ___                                   o                       //
//            /   \        _     _    ___   ___   |           _              //
//            |      __  |/ \  |/ \  /   ) |   \ -+- |  __  |/ \             //
//            |     /  \ |   | |   | |---  |      |  | /  \ |   |            //
//            \___/ \__/ |   | |   |  \__   \__/   \ | \__/ |   |            //
///////////////////////////////////////////////////////////////////////////////

Connection::Connection (const Neuron* source, const Neuron* target, double weight)
{
	mpSource = const_cast<Neuron*>(source);
	mpTarget = const_cast<Neuron*>(target);
	mWeight  = weight;
}

Connection::~Connection ()
{
	if (mpSource)
		mpSource->disconnectTo (*this);
	if (mpTarget)
		mpTarget->disconnectFrom (*this);
}

void Connection::init (double r)
{
	mWeight = 2*r*frnd()-r;
}

double Connection::transfer ()
{
	return mWeight * mpSource->output();
}

void Connection::copy (const Connection& other)
{
	mWeight = other.weight();
#ifdef CMP_WARNINGS
#warning "TODO: Copying connections not implemented"
#endif
	mpSource = NULL;
	mpTarget = NULL;
}


void Connection::check (int netSize) const
{
	ASSERT (mpSource!=NULL);
	ASSERT (mpTarget!=NULL);
}

void Connection::writeXML (TextOStream& out) const
{
	sout.printf ("\t\t<%s>", (CONSTR) getclassname());
	if (mpSource)
		sout.printf ("<SOURCEID>%d</SOURCEID>", mpSource->id());
	sout.printf ("<WEIGHT>%f</WEIGHT>", mWeight);
	sout.printf ("</%s>\n", (CONSTR) getclassname());
}
