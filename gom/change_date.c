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
    struct change_date *chan
      = (struct change_date *)malloc(sizeof(struct change_date));
    if (! chan)
      MEMORY_ERROR;
    else {
      memset (chan, 0, sizeof(struct change_date));
      
      switch (ctxt->ctxt_type) {
	case REC_FAM:
	  family_set_change_date(ctxt, chan); break;
	case REC_INDI:
	  individual_set_change_date(ctxt, chan); break;
	case REC_OBJE:
	  multimedia_set_change_date(ctxt, chan); break;
	case REC_NOTE:
	  note_set_change_date(ctxt, chan); break;
	case REC_REPO:
	  repository_set_change_date(ctxt, chan); break;
	case REC_SOUR:
	  source_set_change_date(ctxt, chan); break;
	case REC_SUBM:
	  submitter_set_change_date(ctxt, chan); break;
	default:
	  UNEXPECTED_CONTEXT(ctxt->ctxt_type);
      }
      result = MAKE_GOM_CTXT(elt, change_date, chan);
    }
  }
  
  return (Gedcom_ctxt)result;
}

DATE_CB(change_date, sub_chan_date_start, date)
STRING_CB(change_date, sub_chan_time_start, time)

void change_date_subscribe()
{
  gedcom_subscribe_to_element(ELT_SUB_CHAN, sub_chan_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_CHAN_DATE, sub_chan_date_start,
			      def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_CHAN_TIME, sub_chan_time_start,
			      def_elt_end);
}

void change_date_add_note(Gom_ctxt ctxt, struct note_sub* note)
{
  struct change_date *chan = SAFE_CTXT_CAST(change_date, ctxt);
  if (chan)
    LINK_CHAIN_ELT(note_sub, chan->note, note);
}

void change_date_add_user_data(Gom_ctxt ctxt, struct user_data* data)
{
  struct change_date *obj = SAFE_CTXT_CAST(change_date, ctxt);
  if (obj)
    LINK_CHAIN_ELT(user_data, obj->extra, data);
}

void change_date_cleanup(struct change_date *chan)
{
  if (chan) {
    SAFE_FREE(chan->date);
    SAFE_FREE(chan->time);
    DESTROY_CHAIN_ELTS(note_sub, chan->note, note_sub_cleanup);
    DESTROY_CHAIN_ELTS(user_data, chan->extra, user_data_cleanup);
  }
  SAFE_FREE(chan);
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
