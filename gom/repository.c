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
  if (repo)
    repo->address = address;
}

void repository_add_phone(Gom_ctxt ctxt, const char *phone)
{
  struct repository *repo = SAFE_CTXT_CAST(repository, ctxt);
  if (repo) {
    int i = 0;
    while (i<2 && repo->phone[i]) i++;
    if (! repo->phone[i]) {
      repo->phone[i] = strdup(phone);
      if (! repo->phone[i]) MEMORY_ERROR;
    }
  }
}

void repository_add_note(Gom_ctxt ctxt, struct note_sub* note)
{
  struct repository *repo = SAFE_CTXT_CAST(repository, ctxt);
  if (repo)
    LINK_CHAIN_ELT(note_sub, repo->note, note);
}

void repository_add_user_ref(Gom_ctxt ctxt, struct user_ref_number* ref)
{
  struct repository *repo = SAFE_CTXT_CAST(repository, ctxt);
  if (repo)
    LINK_CHAIN_ELT(user_ref_number, repo->ref, ref);
}

void repository_set_record_id(Gom_ctxt ctxt, const char *rin)
{
  struct repository *repo = SAFE_CTXT_CAST(repository, ctxt);
  if (repo) {
    repo->record_id = strdup(rin);
    if (! repo->record_id) MEMORY_ERROR;
  }
}

void repository_set_change_date(Gom_ctxt ctxt, struct change_date* chan)
{
  struct repository *repo = SAFE_CTXT_CAST(repository, ctxt);
  if (repo)
    repo->change_date = chan;
}

void repository_add_user_data(Gom_ctxt ctxt, struct user_data* data)
{
  struct repository *obj = SAFE_CTXT_CAST(repository, ctxt);
  if (obj)
    LINK_CHAIN_ELT(user_data, obj->extra, data);
}

void repository_cleanup(struct repository* repo)
{
  if (repo) {
    SAFE_FREE(repo->xrefstr);
    SAFE_FREE(repo->name);
    address_cleanup(repo->address);
    SAFE_FREE(repo->phone[0]);
    SAFE_FREE(repo->phone[1]);
    SAFE_FREE(repo->phone[2]);
    DESTROY_CHAIN_ELTS(note_sub, repo->note, note_sub_cleanup);
    DESTROY_CHAIN_ELTS(user_ref_number, repo->ref, user_ref_cleanup);
    SAFE_FREE(repo->record_id);
    change_date_cleanup(repo->change_date);
    DESTROY_CHAIN_ELTS(user_data, repo->extra, user_data_cleanup);
  }
}

void repositories_cleanup()
{
  DESTROY_CHAIN_ELTS(repository, gom_first_repository, repository_cleanup);
}

struct repository* gom_get_first_repository()
{
  return gom_first_repository;
}

struct repository* make_repository_record(const char* xrefstr)
{
  struct repository* repo = NULL;
  MAKE_CHAIN_ELT(repository, gom_first_repository, repo);
  if (repo) {
    repo->xrefstr = strdup(xrefstr);
    if (! repo->xrefstr) MEMORY_ERROR;
  }
  return repo;
}

int write_repositories(Gedcom_write_hndl hndl)
{
  int result = 0;
  int i;
  struct repository* obj;

  for (obj = gom_first_repository; obj; obj = obj->next) {
    result |= gedcom_write_record_str(hndl, REC_REPO, 0,
				      obj->xrefstr, NULL);
    if (obj->name)
      result |= gedcom_write_element_str(hndl, ELT_REPO_NAME, 0,
					 REC_REPO, obj->name);
    if (obj->address)
      result |= write_address(hndl, REC_REPO, obj->address);
    for (i = 0; i < 3 && obj->phone[i]; i++)
      result |= gedcom_write_element_str(hndl, ELT_SUB_PHON, 0, REC_REPO,
					 obj->phone[i]);
    if (obj->note)
      result |= write_note_subs(hndl, REC_REPO, obj->note);
    if (obj->ref)
      result |= write_user_refs(hndl, REC_REPO, obj->ref);
    if (obj->record_id)
      result |= gedcom_write_element_str(hndl, ELT_SUB_IDENT_RIN, 0,
					 REC_REPO, obj->record_id);
    if (obj->change_date)
      result |= write_change_date(hndl, REC_REPO, obj->change_date);
    if (obj->extra)
      result |= write_user_data(hndl, obj->extra);
  }
  
  return result;
}
