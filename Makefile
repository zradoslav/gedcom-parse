# $Id$
# $Name$
all:
	@if test -r GNUmakefile; then \
	   echo Sorry, GNU make is required; \
	   case `gmake --version 2>/dev/null` in \
	     *GNU*) echo Try again with 'gmake' ;; \
	   esac; \
	else \
	   echo Please run ./configure first; \
	fi
