/* Address sub-structure in the gedcom object model.
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
#include "gom_internal.h"
#include "header.h"
#include "event.h"
#include "address.h"
#include "repository.h"
#include "submitter.h"
#include "user_rec.h"
#include "gom.h"
#include "gedcom.h"

Gedcom_ctxt sub_addr_start(_ELT_PARAMS_)
{
  Gom_ctxt ctxt = (Gom_ctxt)parent;
  Gom_ctxt result = NULL;

  if (!ctxt)
    NO_CONTEXT;
  else {
    struct address *addr = SUB_MAKEFUNC(address)();
    if (addr) {
      int type = ctxt_type(ctxt);
      switch (type) {
	case ELT_HEAD_SOUR_CORP:
	  ADDFUNC2_NOLIST(header,address)(ctxt, addr); break;
	case ELT_SUB_FAM_EVT:
	case ELT_SUB_FAM_EVT_EVEN:
	case ELT_SUB_INDIV_ATTR:
	case ELT_SUB_INDIV_RESI:
	case ELT_SUB_INDIV_BIRT:
	case ELT_SUB_INDIV_GEN:
	case ELT_SUB_INDIV_ADOP:
	case ELT_SUB_INDIV_EVEN:
	  ADDFUNC2_NOLIST(event,address)(ctxt, addr); break;
	case REC_REPO:
	  ADDFUNC2_NOLIST(repository,address)(ctxt, addr); break;
	case REC_SUBM:
	  ADDFUNC2_NOLIST(submitter,address)(ctxt, addr); break;
	default:
	  UNEXPECTED_CONTEXT(type);
      }
      result = MAKE_GOM_CTXT(elt, address, addr);
    }
  }
  
  return (Gedcom_ctxt)result;
}

Gedcom_ctxt sub_addr_cont_start(_ELT_PARAMS_)
{
  Gom_ctxt ctxt = (Gom_ctxt)parent;
  Gom_ctxt result = NULL;
  if (! ctxt)
    NO_CONTEXT;
  else {
    result = dup_gom_ctxt(ctxt, elt);
  }
  return (Gedcom_ctxt)result;
}

DEFINE_SUB_MAKEFUNC(address)
DEFINE_SUB_SETFUNC(address)
DEFINE_SUB_DELETEFUNC(address)

DEFINE_STRING_END_CB(address, sub_addr_end, full_label)
DEFINE_STRING_CB(address, sub_addr_adr1_start, line1)
DEFINE_STRING_CB(address, sub_addr_adr2_start, line2)
DEFINE_STRING_CB(address, sub_addr_city_start, city)
DEFINE_STRING_CB(address, sub_addr_stae_start, state)
DEFINE_STRING_CB(address, sub_addr_post_start, postal)
DEFINE_STRING_CB(address, sub_addr_ctry_start, country)

DEFINE_ADDFUNC2(address, user_data, extra)

Gedcom_ctxt sub_phon_start(_ELT_PARAMS_)
{
  Gom_ctxt ctxt = (Gom_ctxt)parent;
  Gom_ctxt result = NULL;

  if (! ctxt)
    NO_CONTEXT;
  else {
    char *str = GEDCOM_STRING(parsed_value);
    int type = ctxt_type(ctxt);
    switch (type) {
      case ELT_HEAD_SOUR_CORP:
	header_add_phone(ctxt, str); break;
      case ELT_SUB_FAM_EVT:
      case ELT_SUB_INDIV_ATTR:
      case ELT_SUB_INDIV_RESI:
      case ELT_SUB_INDIV_BIRT:
      case ELT_SUB_INDIV_GEN:
      case ELT_SUB_INDIV_ADOP:
      case ELT_SUB_INDIV_EVEN:
	event_add_phone(ctxt, str); break;
      case REC_REPO:
	repository_add_phone(ctxt, str); break;
      case REC_SUBM:
	submitter_add_phone(ctxt, str); break;
      default:
	UNEXPECTED_CONTEXT(type);
    }
    result = dup_gom_ctxt(ctxt, elt);
  }
  return (Gedcom_ctxt)result;
}

void address_subscribe()
{
  gedcom_subscribe_to_element(ELT_SUB_ADDR, sub_addr_start, sub_addr_end);
  gedcom_subscribe_to_element(ELT_SUB_ADDR_CONT,
			      sub_addr_cont_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_ADDR_ADR1,
			      sub_addr_adr1_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_ADDR_ADR2,
			      sub_addr_adr2_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_ADDR_CITY,
			      sub_addr_city_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_ADDR_STAE,
			      sub_addr_stae_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_ADDR_POST,
			      sub_addr_post_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_ADDR_CTRY,
			      sub_addr_ctry_start, def_elt_end);
  gedcom_subscribe_to_element(ELT_SUB_PHON, sub_phon_start, def_elt_end);
}

void UNREFALLFUNC(address)(struct address *address)
{
  if (address) {
    UNREFALLFUNC(user_data)(address->extra);
  }
}

void CLEANFUNC(address)(struct address *address)
{
  if (address) {
    SAFE_FREE(address->full_label);
    SAFE_FREE(address->line1);
    SAFE_FREE(address->line2);
    SAFE_FREE(address->city);
    SAFE_FREE(address->state);
    SAFE_FREE(address->postal);
    SAFE_FREE(address->country);
    DESTROY_CHAIN_ELTS(user_data, address->extra);
  }
  SAFE_FREE(address);
}

int write_address(Gedcom_write_hndl hndl, int parent, struct address *address)
{
  int result = 0;

  if (!address) return 1;
  
  if (address->full_label)
    result |= gedcom_write_element_str(hndl, ELT_SUB_ADDR, 0, parent,
				       address->full_label);
  if (address->line1)
    result |= gedcom_write_element_str(hndl, ELT_SUB_ADDR_ADR1, 0,
				       ELT_SUB_ADDR, address->line1);
  if (address->line2)
    result |= gedcom_write_element_str(hndl, ELT_SUB_ADDR_ADR2, 0,
				       ELT_SUB_ADDR, address->line2);
  if (address->city)
    result |= gedcom_write_element_str(hndl, ELT_SUB_ADDR_CITY, 0,
				       ELT_SUB_ADDR, address->city);
  if (address->state)
    result |= gedcom_write_element_str(hndl, ELT_SUB_ADDR_STAE, 0,
				       ELT_SUB_ADDR, address->state);
  if (address->postal)
    result |= gedcom_write_element_str(hndl, ELT_SUB_ADDR_POST, 0,
				       ELT_SUB_ADDR, address->postal);
  if (address->country)
    result |= gedcom_write_element_str(hndl, ELT_SUB_ADDR_CTRY, 0,
				       ELT_SUB_ADDR, address->country);
  if (address->extra)
    result |= write_user_data(hndl, address->extra);

  return result;
}
