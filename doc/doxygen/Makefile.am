## Process this file with automake to produce Makefile.in
# $Id$
# $Name$

EXTRA_DIST = doxygen.conf.in myfooter.html

doc:	doxygen.conf
	export VERSION=@VERSION@; doxygen doxygen.conf
