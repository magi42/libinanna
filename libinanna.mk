################################################################################
#    This file is part of the MagiC++ library.                                 #
#                                                                              #
#    Copyright (C) 1998-2002 Marko Grönroos <magi@iki.fi>                      #
#                                                                              #
################################################################################
#                                                                              #
#   This library is free software; you can redistribute it and/or              #
#   modify it under the terms of the GNU Library General Public                #
#   License as published by the Free Software Foundation; either               #
#   version 2 of the License, or (at your option) any later version.           #
#                                                                              #
#   This library is distributed in the hope that it will be useful,            #
#   but WITHOUT ANY WARRANTY; without even the implied warranty of             #
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU          #
#   Library General Public License for more details.                           #
#                                                                              #
#   You should have received a copy of the GNU Library General Public          #
#   License along with this library; see the file COPYING.LIB.  If             #
#   not, write to the Free Software Foundation, Inc., 59 Temple Place          #
#   - Suite 330, Boston, MA 02111-1307, USA.                                   #
#                                                                              #
################################################################################

################################################################################
# Define root directory of the source tree
################################################################################
export SRCDIR ?= ..

################################################################################
# Define module name and compilation type
################################################################################
modname         = libinanna
compile_library = 1
makedox         = 1

################################################################################
# Include build framework
################################################################################
include $(SRCDIR)/build/magicdef.mk

################################################################################
# Source files
################################################################################
sources =	annetwork.cc backprop.cc dataformat.cc equalization.cc \
		neuron.cc rprop.cc topology.cc annfilef.cc connection.cc \
		dataformats.cc learning.cc patternset.cc termination.cc \
		trainer.cc prediction.cc


headers =	annetwork.h backprop.h dataformats.h learning.h rprop.h tools.h \
		annfilef.h connection.h equalization.h neuron.h termination.h \
		topology.h annfilefs.h dataformat.h initializer.h patternset.h \
		tfunc.h trainer.h prediction.h

headersubdir = inanna

################################################################################
# Recursively compile some subprojects
################################################################################
# makemodules = extras

################################################################################
# Compile
################################################################################
include $(SRCDIR)/build/magiccmp.mk

