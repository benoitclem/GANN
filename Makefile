###########################################################################
#    Makefile for GANN project
#    Copyright (C) 2015  BENOIT-PILVEN Clément / MARTY Damien
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License along
#    with this program; if not, write to the Free Software Foundation, Inc.,
#    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
###########################################################################

TOOL = 
GPP = $(TOOL)g++
BIN = gann
SRC = gann.cpp
LDFLAGS = -lm

%:
	@$(GPP) $(SRC) -o $(BIN) $(LDFLAGS) -D$@
	@echo ... $@ BUILD DONE ...

all:
	@echo "... PLEASE SPECIFY TARGET PROBLEM, PROGRAM NOT BUILD ...	\n \t-AORB\n \t-AANDB\n \t-ASUPB\n \t-AEQUALB"

clean:
	@rm -f $(BIN)
	@echo ... CLEAN DONE ...