/* Change date sub-structure in the gedcom object model.
   Copyright (C) 2002 The Genes Development Team
   This file is part of the Gedcom parser library.
   Contributed by Peter Verthez <Peter.Verthez@advalvas.be>, 2002.

   The Gedcom parser library is free software; you can redistribute it
   and/or modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The Gedcom parser library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the Gedcom parser library; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/* $Id$ */
/* $Name$ */

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "change_date.h"
#include "family.h"
#include "individual.h"
#include "note_sub.h"
#include "multimedia.h"
#include "note.h"
#include "repository.h"
#include "source.h"
#include "submitter.h"
#include "user_rec.h"
#include "gom_internal.h"
#include "gom.h"
#include "gedcom.h"

Gedcom_ctxt sub_chan_start(_ELT_PARAMS_)
{
  Gom_ctxt ctxt = (Gom_ctxt)parent;
  Gom_ctxt result = NULL;

  if (! ctxt)
    NO_CONTEXT;
  else {
    struct change_date *chan = SUB_MAKEFUNC(change_date)();
    if (chan) {
      switch (ctxt->ctxt_type) {
	case REC_FAM:
	  ADDFUNC2_NOLIST(family,change_date)(ctxt, chan); break;
	case REC_INDI:
	  ADDFUNC2_NOLIST(individual,change_date)(ctxt, chan); break;
	case REC_OBJE:
	  ADDFUNC2_NOLIST(multimedia,change_date)(ctxt, chan); break;
	case REC_NOTE:
	  ADDFUNC2_NOLIST(note,change_date)(ctxt, chan); break;
	case REC_REPO:
	  ADDFUNC2_NOLIST(repository,change_date)(ctxt, chan); break;
	case REC_SOUR:
	  ADDFUNC2_NOLIST(source,change_date)(ctxt, chan); break;
	case REC_SUBM:
	  ADDFUNC2_NOLIST(submitter,change_date)(ctxt, chan); break;
	default:
	  UNEXPECTED_CONTEXT(ctxt->ctxt_type);
      }
      result = MAKE_GOM_CTXT(elt, change_date, chan);
    }
  }
  
  return (Gedcom_ctxt)result;
}

DEFINE_SUB_MAKEFUNC(change_date)
DEFINE_SUB_ADDFUNC(change_date)
DEFINE_SUB_DELETEFUNC(change_date)

DEFINE_DATE_CB(change_date, sub_chan_date_start, date)
DEFINE_STRING_CB(change_date, sub_chan_time_start, time)

DEFINE_ADDFUNC2(change_date, note_sub, note)
DEFINE_ADDFUNC2(change_date, user_data, extra)

void change_date_subscribe()
{
  gedcom_subscribe_to_element(ELT_SUB_CHAN, sub_chan_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_CHAN_DATE, sub_chan_date_start,
			      def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_CHAN_TIME, sub_chan_time_start,
			      def_elt_end);
}

void UNREFALLFUNC(change_date)(struct change_date* obj)
{
  if (obj) {
    UNREFALLFUNC(note_sub)(obj->note);
    UNREFALLFUNC(user_data)(obj->extra);
  }
}

void CLEANFUNC(change_date)(struct change_date *chan)
{
  if (chan) {
    SAFE_FREE(chan->date);
    SAFE_FREE(chan->time);
    DESTROY_CHAIN_ELTS(note_sub, chan->note);
    DESTROY_CHAIN_ELTS(user_data, chan->extra);
  }
  SAFE_FREE(chan);
}

int update_date(struct date_value** dv, struct tm* tm_ptr)
{
  int result;
  struct date_value* dval = gedcom_new_date_value(NULL);
  dval->type        = DV_NO_MODIFIER;
  dval->date1.cal   = CAL_GREGORIAN;
  dval->date1.day   = tm_ptr->tm_mday;
  dval->date1.month = tm_ptr->tm_mon + 1;
  dval->date1.year  = tm_ptr->tm_year + 1900;
  result = gedcom_normalize_date(DI_FROM_NUMBERS, dval);

  if (result == 0) {
    if (*dv) free(*dv);
    *dv = dval;
  }
  return result;
}

int update_time(char** tv, struct tm* tm_ptr)
{
  char tval[16];
  sprintf(tval, "%02d:%02d:%02d",
	  tm_ptr->tm_hour, tm_ptr->tm_min, tm_ptr->tm_sec);

  if (gom_set_string(tv, tval))
    return 0;
  else
    return 1;
}

int gom_update_timestamp(struct change_date** chan, time_t t)
{
  int result = 1;
  if (chan) {
    if (! *chan) gom_add_change_date(chan);
    if (*chan) {
      struct tm *tm_ptr = localtime(&t);
      result = 0;
      result |= update_date(&((*chan)->date), tm_ptr);
      result |= update_time(&((*chan)->time), tm_ptr);
    }
  }
  return result;
}

int write_change_date(Gedcom_write_hndl hndl, int parent,
		      struct change_date *chan)
{
  int result = 0;

  if (!chan) return 1;

  result |= gedcom_write_element_str(hndl, ELT_SUB_CHAN, 0, parent, NULL);
  if (chan->date)
    result |= gedcom_write_element_date(hndl, ELT_SUB_CHAN_DATE, 0,
					ELT_SUB_CHAN, chan->date);
  if (chan->time)
    result |= gedcom_write_element_str(hndl, ELT_SUB_CHAN_TIME, 0,
				       ELT_SUB_CHAN_DATE, chan->time);
  if (chan->note)
    result |= write_note_subs(hndl, ELT_SUB_CHAN, chan->note);
  if (chan->extra)
    result |= write_user_data(hndl, chan->extra);
  
  return result;
}
