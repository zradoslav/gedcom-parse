#include "interface.h"

Gedcom_rec_start_cb record_start_callback[] =
  { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

Gedcom_rec_end_cb record_end_callback[] =
  { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };


void subscribe_to_record(Gedcom_rec rec,
			 Gedcom_rec_start_cb cb_start,
			 Gedcom_rec_end_cb cb_end)
{
  record_start_callback[rec] = cb_start;
  record_end_callback[rec]   = cb_end;
}

Gedcom_ctxt start_record(Gedcom_rec rec, char *xreftag)
{
  Gedcom_rec_start_cb cb = record_start_callback[rec];
  if (cb != NULL)
    return (*cb)(xreftag);
  else
    return NULL;
}

void end_record(Gedcom_rec rec, Gedcom_ctxt self)
{
  Gedcom_rec_end_cb cb = record_end_callback[rec];
  if (cb != NULL)
    (*cb)(self);
}

Gedcom_ctxt start_element(Gedcom_elt elt, Gedcom_ctxt parent, 
			  int level, char *tag, char *raw_value,
			  void *parsed_value)
{
  printf("Start of element %d, parent context is %d\n", elt, (int)parent);
  printf("  value is: %d %s %s\n", level, tag, raw_value);
  return (Gedcom_ctxt)(elt+100);
}

void end_element(Gedcom_ctxt parent, Gedcom_ctxt self)
{
  printf("End of element, context is %d, parent context is %d\n",
	 (int)self, (int)parent);
}

