## File to include in other Makefile's, defining common commands
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
