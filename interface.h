#ifndef __INTERFACE_H
#define __INTERFACE_H

#include "gedcom.h"
#include "external.h"

Gedcom_ctxt start_record(Gedcom_rec rec, int level, char *xref, char *tag);
void        end_record(Gedcom_rec rec, Gedcom_ctxt self);

Gedcom_ctxt start_element(Gedcom_elt elt, Gedcom_ctxt parent,
			  int level, char *tag, char *raw_value,
			  Gedcom_val parsed_value);
void        end_element(Gedcom_elt elt, Gedcom_ctxt parent, Gedcom_ctxt self,
			Gedcom_val parsed_value);


#endif /* __INTERFACE_H */
