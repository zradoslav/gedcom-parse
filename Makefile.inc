## File to include in other Makefile's, defining common commands and rules
# $Id$
# $Name$

define replace-VERSION
  cd $(distdir); \
  for file in $(VERSIONED_FILES); do \
  sed 's/\@VERSION\@/${VERSION}/' $$file > $$file.new; \
  rm $$file; \
  mv $$file.new $$file; \
  done
endef

%.tabgen.h:	%.tab.h
	if [ ! -r $(srcdir)/$@ ] ; \
	then \
	  touch $(srcdir)/$@; \
	fi
	if diff $< $(srcdir)/$@ > /dev/null 2>&1; \
	then \
	  : ; \
	else \
	  rm $(srcdir)/$@; \
	  cp $(srcdir)/$< $(srcdir)/$@; \
	fi
