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

DEFINE_MAKEFUNC(repository, gom_first_repository)
DEFINE_DESTROYFUNC(repository, gom_first_repository)
DEFINE_ADDFUNC(repository, XREF_REPO)
DEFINE_DELETEFUNC(repository)
DEFINE_GETXREFFUNC(repository, XREF_REPO)
     
DEFINE_REC_CB(repository, repo_start)
DEFINE_STRING_CB(repository, repo_name_start, name)

DEFINE_ADDFUNC2(repository, note_sub, note)
DEFINE_ADDFUNC2(repository, user_ref_number, ref)
DEFINE_ADDFUNC2(repository, user_data, extra)
DEFINE_ADDFUNC2_NOLIST(repository, address, address)
DEFINE_ADDFUNC2_NOLIST(repository, change_date, change_date)
DEFINE_ADDFUNC2_STRN(repository, phone, 3)
DEFINE_ADDFUNC2_STR(repository, record_id)

void repository_subscribe()
{
  gedcom_subscribe_to_record(REC_REPO, repo_start, def_rec_end);
  gedcom_subscribe_to_element(ELT_REPO_NAME, repo_name_start, def_elt_end);
}

void UNREFALLFUNC(repository)(struct repository *obj)
{
  if (obj) {
    UNREFALLFUNC(address)(obj->address);
    UNREFALLFUNC(note_sub)(obj->note);
    UNREFALLFUNC(user_ref_number)(obj->ref);
    UNREFALLFUNC(change_date)(obj->change_date);
    UNREFALLFUNC(user_data)(obj->extra);
  }
}

void CLEANFUNC(repository)(struct repository* repo)
{
  if (repo) {
    SAFE_FREE(repo->xrefstr);
    SAFE_FREE(repo->name);
    CLEANFUNC(address)(repo->address);
    SAFE_FREE(repo->phone[0]);
    SAFE_FREE(repo->phone[1]);
    SAFE_FREE(repo->phone[2]);
    DESTROY_CHAIN_ELTS(note_sub, repo->note);
    DESTROY_CHAIN_ELTS(user_ref_number, repo->ref);
    SAFE_FREE(repo->record_id);
    CLEANFUNC(change_date)(repo->change_date);
    DESTROY_CHAIN_ELTS(user_data, repo->extra);
  }
}

void repositories_cleanup()
{
  DESTROY_CHAIN_ELTS(repository, gom_first_repository);
}

struct repository* gom_get_first_repository()
{
  return gom_first_repository;
}

int write_repositories(Gedcom_write_hndl hndl)
{
  int result = 0;
  int i;
  struct repository* obj;

  for (obj = gom_first_repository; obj; obj = obj->next) {
    result |= gedcom_write_record_str(hndl, REC_REPO, obj->xrefstr, NULL);
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
