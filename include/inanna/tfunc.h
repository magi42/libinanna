/***************************************************************************
 *   copyright            : (C) 2000 by Marko Grönroos                     *
 *   email                : magi@iki.fi                                    *
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************
 *
 **/

#ifndef __TFUNC_H__
#define __TFUNC_H__


///////////////////////////////////////////////////////////////////////////////
//     -----                                     -----                       //
//       |        ___    _    ____  __  ___      |             _    ___      //
//       |   |/\  ___| |/ \  (     /   /   ) |/\ |---  |   | |/ \  |   \     //
//       |   |   (   | |   |  \__  +-- |---  |   |     |   | |   | |         //
//       |   |    \__| |   | ____) |    \__  |   |      \__! |   |  \__/     //
//                                 |                                         //
///////////////////////////////////////////////////////////////////////////////

class TransferFunc : public Object {
  public:
						TransferFunc	() {}
	virtual double		calc			(double x) const {MUST_OVERLOAD}
	virtual double		derivative		(double x) const {MUST_OVERLOAD}
  protected:
};

class SigmoidTFunc : public Object {
  public:
						SigmoidTFunc	() {}
	virtual double		calc			(double x) const {return sigmoid(x);}
	virtual double		derivative		(double x) const {return x*(1-x);}
  protected:
};

#endif
