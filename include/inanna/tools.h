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

#ifndef __TOOLS_H__
#define __TOOLS_H__

/** Generic abstract factory for a specific type of classes.
 *
 **/
template<class T>
class Lib {
  public:
									Lib			() {}
	AuditingStrategy*				create		(int i) const {
		return dynamic_cast<T*> (dyncreate (mClassNames[i]));
	}
	int								classes		() const {return mClassNames.size;}
	void							registerCls	(const String& classname) {mClassNames.add(classname);}
	static	AuditingStrategyLib&	instance	() {return sInstance;}
  protected:
	static AuditingStrategyLib		sInstance;
	Array<String>					mClassNames;
};

#endif

