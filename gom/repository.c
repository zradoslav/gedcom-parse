/* Repository object in the gedcom object model.
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
#include "repository.h"
#include "address.h"
#include "note_sub.h"
#include "user_ref.h"
#include "change_date.h"
#include "user_rec.h"
#include "gom.h"
#include "gedcom.h"
#include "gom_internal.h"

struct repository* gom_first_repository = NULL;

REC_CB(repository, repo_start, make_repository_record)
GET_REC_BY_XREF(repository, XREF_REPO, gom_get_repository_by_xref)
STRING_CB(repository, repo_name_start, name)

void repository_subscribe()
{
  gedcom_subscribe_to_record(REC_REPO, repo_start, def_rec_end);
  gedcom_subscribe_to_element(ELT_REPO_NAME, repo_name_start, def_elt_end);
}

void repository_add_address(Gom_ctxt ctxt, struct address* address)
{
  struct repository *repo = SAFE_CTXT_CAST(repository, ctxt);
  repo->address = address;
}

void repository_add_phone(Gom_ctxt ctxt, char *phone)
{
  struct repository *repo = SAFE_CTXT_CAST(repository, ctxt);
  if (! repo->phone[0])
    repo->phone[0] = strdup(phone);
  else if (! repo->phone[1])
    repo->phone[1] = strdup(phone);
  else if (! repo->phone[2])
    repo->phone[2] = strdup(phone);
}

void repository_add_note(Gom_ctxt ctxt, struct note_sub* note)
{
  struct repository *repo = SAFE_CTXT_CAST(repository, ctxt);
  LINK_CHAIN_ELT(note_sub, repo->note, note)
}

void repository_add_user_ref(Gom_ctxt ctxt, struct user_ref_number* ref)
{
  struct repository *repo = SAFE_CTXT_CAST(repository, ctxt);
  LINK_CHAIN_ELT(user_ref_number, repo->ref, ref)
}

void repository_set_record_id(Gom_ctxt ctxt, char *rin)
{
  struct repository *repo = SAFE_CTXT_CAST(repository, ctxt);
  repo->record_id = strdup(rin);
}

void repository_set_change_date(Gom_ctxt ctxt, struct change_date* chan)
{
  struct repository *repo = SAFE_CTXT_CAST(repository, ctxt);
  repo->change_date = chan;
}

void repository_add_user_data(Gom_ctxt ctxt, struct user_data* data)
{
  struct repository *obj = SAFE_CTXT_CAST(repository, ctxt);
  LINK_CHAIN_ELT(user_data, obj->extra, data)
}

void repository_cleanup(struct repository* repo)
{
  SAFE_FREE(repo->xrefstr);
  SAFE_FREE(repo->name);
  address_cleanup(repo->address);
  SAFE_FREE(repo->phone[0]);
  SAFE_FREE(repo->phone[1]);
  SAFE_FREE(repo->phone[2]);
  DESTROY_CHAIN_ELTS(note_sub, repo->note, note_sub_cleanup)
  DESTROY_CHAIN_ELTS(user_ref_number, repo->ref, user_ref_cleanup)
  SAFE_FREE(repo->record_id);
  change_date_cleanup(repo->change_date);
  DESTROY_CHAIN_ELTS(user_data, repo->extra, user_data_cleanup)
}

void repositories_cleanup()
{
  DESTROY_CHAIN_ELTS(repository, gom_first_repository, repository_cleanup);
}

struct repository* gom_get_first_repository()
{
  return gom_first_repository;
}

struct repository* make_repository_record(char* xrefstr)
{
  struct repository* repo;
  MAKE_CHAIN_ELT(repository, gom_first_repository, repo);
  repo->xrefstr = strdup(xrefstr);
  return repo;
}
