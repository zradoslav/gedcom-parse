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
	if [ ! -r $@ ] ; \
	then \
	  touch $@; \
	fi
	if ! diff -q $< $@ > /dev/null; \
	then \
	  rm $@; \
	  cp $< $@; \
	fi
