## Process this file with automake to produce Makefile.in
# $Id$
# $Name$

INCLUDES = -I $(srcdir)/../intl -I $(srcdir)/../include -I $(srcdir)/../utf8
CFLAGS   = -g -O2 -W -Wall -Wno-unused-parameter -pedantic

lib_LTLIBRARIES = libgedcom_gom.la
libgedcom_gom_la_SOURCES = gom.c \
			   header.c \
		    	   submission.c \
		    	   submitter.c \
		    	   family.c \
		    	   individual.c \
		    	   multimedia.c \
		    	   note.c \
		    	   repository.c \
		    	   source.c \
		    	   address.c \
		    	   event.c \
		    	   place.c \
		    	   source_citation.c \
		    	   note_sub.c \
		    	   multimedia_link.c \
		    	   lds_event.c \
		    	   user_ref.c \
		    	   change_date.c \
		    	   personal_name.c \
		    	   family_link.c \
		    	   association.c \
			   source_event.c \
			   source_description.c \
		  	   user_rec.c \
			   gom_modify.c
noinst_HEADERS = header.h \
		 submission.h \
		 submitter.h \
		 family.h \
		 individual.h \
		 multimedia.h \
		 note.h \
		 repository.h \
		 source.h \
		 address.h \
		 event.h \
		 place.h \
		 source_citation.h \
		 note_sub.h \
		 multimedia_link.h \
		 lds_event.h \
		 user_ref.h \
		 change_date.h \
		 personal_name.h \
		 family_link.h \
		 association.h \
		 source_event.h \
		 source_description.h \
		 user_rec.h \
		 gom_internal.h

libgedcom_gom_la_LDFLAGS = -version-info $(LIBVERSION)
libgedcom_gom_la_LIBADD  = ../utf8/libutf8.la
