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
export SRCDIR ?= ../../..

################################################################################
# Define module name and compilation type
################################################################################
modname   = neuroprediction
modpath   = libinanna/extras/prediction
modtarget = neuroprediction

################################################################################
# Include build framework
################################################################################
include $(SRCDIR)/build/magicdef.mk

################################################################################
# Source files for libmagic.a
################################################################################
sources = predictionmain.cc

headers = 

libdeps = inanna magic app

datafiles = company_1_90-95.tsv company_1_90-99.tsv company_2_74-80.tsv

configfiles = neuroprediction.cfg

EXTRA_INCLUDE_DIRS += -I$(SRCDIR)/libinanna/include

################################################################################
# Compile
################################################################################
include $(SRCDIR)/build/magiccmp.mk

################################################################################
# Library dependencies
################################################################################
$(libdir)/libmagic.a:
$(libdir)/libapp.a:
$(libdir)/libinanna.a:
