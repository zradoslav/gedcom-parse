/* Test program for the Gedcom library.
   Copyright (C) 2001, 2002 The Genes Development Team
   This file is part of the Gedcom parser library.
   Contributed by Peter Verthez <Peter.Verthez@advalvas.be>, 2001.

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

#include <string.h>
#include "dump_gom.h"
#include "output.h"
#include "portability.h"
#include "gom.h"
#include "gedcom.h"

char* make_prefix(int depth)
{
  char* prefix = (char*)calloc(depth+1, sizeof(char));
  memset(prefix, ' ', depth);
  return prefix;
}

void dump_xref(int st, int prefix_depth, struct xref_value* xr);

void dump_user_data(int st, int prefix_depth, struct user_data* data)
{
  char* prefix = make_prefix(prefix_depth);
  if (data) {
    output(st, "\n");
    for (data; data; data = data->next) {
      output(st, "%sData: \n", prefix);
      output(st, "%s  %d, '%s', '%s'\n", prefix,
	     data->level, str_val(data->tag), str_val(data->str_value));
      output(st, "%s  reference: ", prefix);
      dump_xref(st, prefix_depth + 4, data->xref_value);
    }
  }
  else {
    output(st, "%s\n", ptr_val(data));
  }
  free(prefix);
}

void dump_address(int st, int prefix_depth, struct address* addr)
{
  char* prefix = make_prefix(prefix_depth);
  if (addr) {
    output(st, "\n");
    output(st, "%sFull label: '%s'\n", prefix, str_val(addr->full_label));
    output(st, "%sLine 1: '%s'\n", prefix, str_val(addr->line1));
    output(st, "%sLine 2: '%s'\n", prefix, str_val(addr->line2));
    output(st, "%sCity: '%s'\n", prefix, str_val(addr->city));
    output(st, "%sState: '%s'\n", prefix, str_val(addr->state));
    output(st, "%sPostal: '%s'\n", prefix, str_val(addr->postal));
    output(st, "%sCountry: '%s'\n", prefix, str_val(addr->country));
    output(st, "%sUser data: ", prefix);
    dump_user_data(st, prefix_depth + 2, addr->extra);
  }
  else {
    output(st, "%s\n", ptr_val(addr));
  }
  free(prefix);
}

void dump_date(int st, int prefix_depth, struct date_value* dv)
{
  char* prefix = make_prefix(prefix_depth);
  if (dv) {
    output(st, "\n");
    output(st, "%stype: %d\n", prefix, dv->type);
    output(st, "%sdate1:\n", prefix);
    output(st, "%s  calendar type: %d\n", prefix, dv->date1.cal);
    output(st, "%s  day: '%s', %d\n", prefix,
	   str_val(dv->date1.day_str), dv->date1.day);
    output(st, "%s  month: '%s', %d\n", prefix,
	   str_val(dv->date1.month_str), dv->date1.month);
    output(st, "%s  year: '%s', %d\n", prefix,
	   str_val(dv->date1.year_str), dv->date1.year);
    output(st, "%s  year type: %d\n", prefix, dv->date1.year_type);
    output(st, "%s  date type: %d\n", prefix, dv->date1.type);
    output(st, "%s  sdn1: %ld\n", prefix, dv->date1.sdn1);
    output(st, "%s  sdn2: %ld\n", prefix, dv->date1.sdn2);
    output(st, "%sdate2:\n", prefix);
    output(st, "%s  calendar type: %d\n", prefix, dv->date2.cal);
    output(st, "%s  day: '%s', %d\n", prefix,
	   str_val(dv->date2.day_str), dv->date2.day);
    output(st, "%s  month: '%s', %d\n", prefix,
	   str_val(dv->date2.month_str), dv->date2.month);
    output(st, "%s  year: '%s', %d\n", prefix,
	   str_val(dv->date2.year_str), dv->date2.year);
    output(st, "%s  year type: %d\n", prefix, dv->date2.year_type);
    output(st, "%s  date type: %d\n", prefix, dv->date2.type);
    output(st, "%s  sdn1: %ld\n", prefix, dv->date2.sdn1);
    output(st, "%s  sdn2: %ld\n", prefix, dv->date2.sdn2);
    output(st, "%sphrase: '%s'\n", prefix, str_val(dv->phrase));
  }
  else {
    output(st, "%s\n", ptr_val(dv));
  }
  free(prefix);
}

void show_date(struct date_value* dv)
{
  dump_date(0, 2, dv);
}

void dump_age(int st, int prefix_depth, struct age_value* age)
{
  char* prefix = make_prefix(prefix_depth);
  if (age) {
    output(st, "\n");
    output(st, "%stype: %d\n", prefix, age->type);
    output(st, "%smodifier: %d\n", prefix, age->mod);
    output(st, "%syears: %d\n", prefix, age->years);
    output(st, "%smonths: %d\n", prefix, age->months);
    output(st, "%sdays: %d\n", prefix, age->days);
    output(st, "%sphrase: '%s'\n", prefix, str_val(age->phrase));
  }
  else {
    output(st, "%s\n", ptr_val(age));
  }
  free(prefix);
}

void dump_xref(int st, int prefix_depth, struct xref_value* xr)
{
  char* prefix = make_prefix(prefix_depth);
  if (xr) {
    output(st, "\n");
    output(st, "%stype: %d\n", prefix, xr->type);
    output(st, "%sxref: '%s'\n", prefix, str_val(xr->string));
    output(st, "%sobject: %s\n", prefix, ptr_val(xr->object));
  }
  else {
    output(st, "%s\n", ptr_val(xr));
  }
  free(prefix);
}

void dump_xref_list(int st, int prefix_depth, struct xref_list* xr)
{
  char* prefix = make_prefix(prefix_depth);
  if (xr) {
    output(st, "\n");
    for (xr; xr; xr = xr->next) {
      output(st, "%sreference: ", prefix);
      dump_xref(st, prefix_depth + 2, xr->xref);
      output(st, "%sUser data: ", prefix);
      dump_user_data(st, prefix_depth + 2, xr->extra);
    }
  }
  else {
    output(st, "%s\n", ptr_val(xr));
  }
  free(prefix);
}

void dump_texts(int st, int prefix_depth, struct text* t)
{
  char* prefix = make_prefix(prefix_depth);
  if (t) {
    output(st, "\n");
    for (t; t; t = t->next) {
      output(st, "%sText: '%s'\n", prefix, str_val(t->text));
      output(st, "%sUser data: ", prefix);
      dump_user_data(st, prefix_depth + 2, t->extra);
    }
  }
  else {
    output(st, "%s\n", ptr_val(t));
  }
  free(prefix);
}

void dump_user_ref(int st, int prefix_depth, struct user_ref_number* ref)
{
  char* prefix = make_prefix(prefix_depth);
  if (ref) {
    output(st, "\n");
    for (ref; ref; ref = ref->next) {
      output(st, "%sValue: '%s'\n", prefix, str_val(ref->value));
      output(st, "%sType: '%s'\n", prefix, str_val(ref->type));
      output(st, "%sUser data: ", prefix);
      dump_user_data(st, prefix_depth + 2, ref->extra);
    }
  }
  else {
    output(st, "%s\n", ptr_val(ref));
  }
  free(prefix);
}

void dump_citations(int st, int prefix_depth, struct source_citation* cit);

void dump_note_sub(int st, int prefix_depth, struct note_sub* note)
{
  char* prefix = make_prefix(prefix_depth);
  if (note) {
    output(st, "\n");
    for (note; note; note = note->next) {
      output(st, "%sNote: \n", prefix);
      output(st, "%s  text: '%s'\n", prefix, str_val(note->text));
      output(st, "%s  reference: ", prefix);
      dump_xref(st, prefix_depth + 4, note->reference);
      output(st, "%s  citations: ", prefix);
      dump_citations(st, prefix_depth + 4, note->citation);
      output(st, "%s  User data: ", prefix);
      dump_user_data(st, prefix_depth + 4, note->extra);
    }
  }
  else {
    output(st, "%s\n", ptr_val(note));
  }
  free(prefix);
}

void dump_mm_links(int st, int prefix_depth, struct multimedia_link* link)
{
  char* prefix = make_prefix(prefix_depth);
  if (link) {
    output(st, "\n");
    for (link; link; link = link->next) {
      output(st, "%slink: \n", prefix);
      output(st, "%s  reference: ", prefix);
      dump_xref(st, prefix_depth + 4, link->reference);
      output(st, "%s  Form: '%s'\n", prefix, str_val(link->form));
      output(st, "%s  Title: '%s'\n", prefix, str_val(link->title));
      output(st, "%s  File: '%s'\n", prefix, str_val(link->file));      
      output(st, "%s  notes: ", prefix);
      dump_note_sub(st, prefix_depth + 4, link->note);
      output(st, "%s  User data: ", prefix);
      dump_user_data(st, prefix_depth + 4, link->extra);
    }
  }
  else {
    output(st, "%s\n", ptr_val(link));
  }
  free(prefix);
}

void dump_citations(int st, int prefix_depth, struct source_citation* cit)
{
  char* prefix = make_prefix(prefix_depth);
  if (cit) {
    output(st, "\n");
    for (cit; cit; cit = cit->next) {
      output(st, "%sCitation: \n", prefix);
      output(st, "%s  description: '%s'\n", prefix, str_val(cit->description));
      output(st, "%s  reference: ", prefix);
      dump_xref(st, prefix_depth + 4, cit->reference);
      output(st, "%s  page: '%s'\n", prefix, str_val(cit->page));
      output(st, "%s  event: '%s'\n", prefix, str_val(cit->event));
      output(st, "%s  role: '%s'\n", prefix, str_val(cit->role));
      output(st, "%s  Date: ", prefix);
      dump_date(st, prefix_depth + 4, cit->date);
      output(st, "%s  texts: ", prefix, prefix);
      dump_texts(st, prefix_depth + 4, cit->text);
      output(st, "%s  quality: '%s'\n", prefix, str_val(cit->quality));
      output(st, "%s  multimedia links: ", prefix);
      dump_mm_links(st, prefix_depth + 4, cit->mm_link);
      output(st, "%s  notes: ", prefix);
      dump_note_sub(st, prefix_depth + 4, cit->note);
      output(st, "%s  User data: ", prefix);
      dump_user_data(st, prefix_depth + 4, cit->extra);
    }
  }
  else {
    output(st, "%s\n", ptr_val(cit));
  }
  free(prefix);
}

void dump_lds(int st, int prefix_depth, struct lds_event* lds)
{
  char* prefix = make_prefix(prefix_depth);
  if (lds) {
    output(st, "\n");
    for (lds; lds; lds = lds->next) {
      output(st, "%sDate status: '%s'\n", prefix, str_val(lds->date_status));
      output(st, "%sDate: ", prefix);
      dump_date(st, prefix_depth + 2, lds->date);
      output(st, "%sTemple code: '%s'\n", prefix, str_val(lds->temple_code));
      output(st, "%sPlace living ordinance: '%s'\n", prefix,
	     str_val(lds->place_living_ordinance));
      output(st, "%scitations: ", prefix);
      dump_citations(st, prefix_depth + 2, lds->citation);
      output(st, "%snotes: ", prefix);
      dump_note_sub(st, prefix_depth + 2, lds->note);
      output(st, "%sfamily: ", prefix);
      dump_xref(st, prefix_depth + 2, lds->family);
      output(st, "%sUser data: ", prefix);
      dump_user_data(st, prefix_depth + 2, lds->extra);
    }
  }
  else {
    output(st, "%s\n", ptr_val(lds));
  }
  free(prefix);
}

void dump_change_date(int st, int prefix_depth, struct change_date* chan)
{
  char* prefix = make_prefix(prefix_depth);
  if (chan) {
    output(st, "\n");
    output(st, "%sDate: ", prefix);
    dump_date(st, prefix_depth + 2, chan->date);
    output(st, "%sTime: '%s'\n", prefix, str_val(chan->time));
    output(st, "%snotes: ", prefix);
    dump_note_sub(st, prefix_depth + 2, chan->note);
    output(st, "%sUser data: ", prefix);
    dump_user_data(st, prefix_depth + 2, chan->extra);
  }
  else {
    output(st, "%s\n", ptr_val(chan));
  }
  free(prefix);
}

void dump_personal_name(int st, int prefix_depth, struct personal_name* name)
{
  char* prefix = make_prefix(prefix_depth);
  if (name) {
    output(st, "\n");
    for (name; name; name = name->next) {
      output(st, "%sName: \n", prefix);
      output(st, "%s  Name: '%s'\n", prefix, str_val(name->name));
      output(st, "%s  Prefix: '%s'\n", prefix, str_val(name->prefix));
      output(st, "%s  Given: '%s'\n", prefix, str_val(name->given));
      output(st, "%s  Nickname: '%s'\n", prefix, str_val(name->nickname));
      output(st, "%s  Surname prefix: '%s'\n", prefix,
	     str_val(name->surname_prefix));
      output(st, "%s  Surname: '%s'\n", prefix, str_val(name->surname));
      output(st, "%s  Suffix: '%s'\n", prefix, str_val(name->suffix));
      output(st, "%s  citations: ", prefix);
      dump_citations(st, prefix_depth + 4, name->citation);
      output(st, "%s  notes: ", prefix);
      dump_note_sub(st, prefix_depth + 4, name->note);
      output(st, "%s  User data: ", prefix);
      dump_user_data(st, prefix_depth + 4, name->extra);
    }
  }
  else {
    output(st, "%s\n", ptr_val(name));
  }
  free(prefix);
}

void dump_pedigree(int st, int prefix_depth, struct pedigree* p)
{
  char* prefix = make_prefix(prefix_depth);
  if (p) {
    output(st, "\n");
    for (p; p; p = p->next) {
      output(st, "%sPedigree: '%s'\n", prefix, str_val(p->pedigree));
      output(st, "%sUser data: ", prefix);
      dump_user_data(st, prefix_depth + 2, p->extra);
    }
  }
  else {
    output(st, "%s\n", ptr_val(p));
  }
  free(prefix);
}

void dump_family_link(int st, int prefix_depth, struct family_link *link)
{
  char* prefix = make_prefix(prefix_depth);
  if (link) {
    output(st, "\n");
    for (link; link; link = link->next) {
      output(st, "%sFamily:\n", prefix);
      output(st, "%s  Family: ", prefix);
      dump_xref(st, prefix_depth + 4, link->family);
      output(st, "%s  pedigrees: ", prefix);
      dump_pedigree(st, prefix_depth + 4, link->pedigree);
      output(st, "%s  notes: ", prefix);
      dump_note_sub(st, prefix_depth + 4, link->note);
      output(st, "%s  User data: ", prefix);
      dump_user_data(st, prefix_depth + 4, link->extra);
    }
  }
  else {
    output(st, "%s\n", ptr_val(link));
  }
  free(prefix);
}

void dump_association(int st, int prefix_depth, struct association *assoc)
{
  char* prefix = make_prefix(prefix_depth);
  if (assoc) {
    output(st, "\n");
    for (assoc; assoc; assoc = assoc->next) {
      output(st, "%sAssociation:\n", prefix);
      output(st, "%s  To:\n", prefix);
      dump_xref(st, prefix_depth + 4, assoc->to);
      output(st, "%s  Type: '%s'\n", prefix, str_val(assoc->type));
      output(st, "%s  Relation: '%s'\n", str_val(assoc->relation));
      output(st, "%s  citations: ", prefix);
      dump_citations(st, prefix_depth + 4, assoc->citation);
      output(st, "%s  notes: ", prefix);
      dump_note_sub(st, prefix_depth + 4, assoc->note);
      output(st, "%s  User data: ", prefix);
      dump_user_data(st, prefix_depth + 4, assoc->extra);
    }
  }
  else {
    output(st, "%s\n", ptr_val(assoc));
  }
  free(prefix);
}

void dump_place(int st, int prefix_depth, struct place* place)
{
  char* prefix = make_prefix(prefix_depth);
  if (place) {
    output(st, "\n");
    output(st, "%svalue: '%s'\n", prefix, str_val(place->value));
    output(st, "%splace_hierarchy: '%s'\n", prefix,
	   str_val(place->place_hierarchy));
    output(st, "%scitations: ", prefix);
    dump_citations(st, prefix_depth + 2, place->citation);
    output(st, "%snotes: ", prefix);
    dump_note_sub(st, prefix_depth + 2, place->note);
    output(st, "%sUser data: ", prefix);
    dump_user_data(st, prefix_depth + 2, place->extra);
  }
  else {
    output(st, "%s\n", ptr_val(place));
  }
  free(prefix);
}

void dump_source_events(int st, int prefix_depth, struct source_event* evt)
{
  char* prefix = make_prefix(prefix_depth);
  if (evt) {
    output(st, "\n");
    for (evt; evt; evt = evt->next) {
      output(st, "%sEvent:\n", prefix);
      output(st, "%s  Recorded events: '%s'\n", prefix,
	     str_val(evt->recorded_events));
      output(st, "%s  Date period: ", prefix);
      dump_date(st, prefix_depth + 4, evt->date_period);
      output(st, "%s  Jurisdiction: '%s'\n", prefix, str_val(evt->jurisdiction));
      output(st, "%s  User data: ", prefix);
      dump_user_data(st, prefix_depth + 4, evt->extra);
    }
  }
  else {
    output(st, "%s\n", ptr_val(evt));
  }
  free(prefix);
}

void dump_source_descriptions(int st, int prefix_depth,
			      struct source_description* desc)
{
  char* prefix = make_prefix(prefix_depth);
  if (desc) {
    output(st, "\n");
    for (desc; desc; desc = desc->next) {
      output(st, "%sSource description:\n", prefix);
      output(st, "%s  Call number: '%s'\n", prefix, str_val(desc->call_number));
      output(st, "%s  Media: '%s'\n", prefix, str_val(desc->media));
      output(st, "%s  User data: ", prefix);
      dump_user_data(st, prefix_depth + 4, desc->extra);
    }
  }
  else {
    output(st, "%s\n", ptr_val(desc));
  }
  free(prefix);
}

void dump_events(int st, int prefix_depth, struct event *evt)
{
  char* prefix = make_prefix(prefix_depth);
  if (evt) {
    output(st, "\n");
    for (evt; evt; evt = evt->next) {
      output(st, "%sEvent: %d (%s)\n", prefix, evt->event,
	     str_val(evt->event_name));
      output(st, "%s  Value: '%s'\n", prefix, str_val(evt->val));
      output(st, "%s  Type: '%s'\n", prefix, str_val(evt->type));
      output(st, "%s  Date: ", prefix);
      dump_date(st, prefix_depth + 4, evt->date);
      output(st, "%s  Place: ", prefix);
      dump_place(st, prefix_depth + 4, evt->place);
      output(st, "%s  Address: ", prefix);
      dump_address(st, prefix_depth + 4, evt->address);
      output(st, "%s  Phone 1: '%s'\n", prefix, str_val(evt->phone[0]));
      output(st, "%s  Phone 2: '%s'\n", prefix, str_val(evt->phone[1]));
      output(st, "%s  Phone 3: '%s'\n", prefix, str_val(evt->phone[2]));
      output(st, "%s  Age: ", prefix);
      dump_age(st, prefix_depth + 4, evt->age);
      output(st, "%s  Agency: '%s'\n", prefix, str_val(evt->agency));
      output(st, "%s  Cause: '%s'\n", prefix, str_val(evt->cause));
      output(st, "%s  citations: ", prefix);
      dump_citations(st, prefix_depth + 4, evt->citation);
      output(st, "%s  multimedia links: ", prefix);
      dump_mm_links(st, prefix_depth + 4, evt->mm_link);
      output(st, "%s  notes: ", prefix);
      dump_note_sub(st, prefix_depth + 4, evt->note);
      output(st, "%s  Age of husband: ", prefix);
      dump_age(st, prefix_depth + 4, evt->husband_age);
      output(st, "%s  Age of wife: ", prefix);
      dump_age(st, prefix_depth + 4, evt->wife_age);
      output(st, "%s  Family: ", prefix);
      dump_xref(st, prefix_depth + 4, evt->family);
      output(st, "%s  Adoption parent: '%s'\n", prefix,
	     str_val(evt->adoption_parent));
      output(st, "%s  User data: ", prefix);
      dump_user_data(st, prefix_depth + 4, evt->extra);
    }
  }
  else {
    output(st, "%s\n", ptr_val(evt));
  }
  free(prefix);
}

void dump_header()
{
  struct header* header = gom_get_header();
  output(1, "=== HEADER ===\n");
  output(0, "Source:\n");
  output(0, "  ID: '%s'\n", str_val(header->source.id));
  output(0, "  Name: '%s'\n", str_val(header->source.name));
  output(0, "  Version: '%s'\n", str_val(header->source.version));
  output(0, "  Corporation:\n");
  output(0, "    Name: '%s'\n", str_val(header->source.corporation.name));
  output(0, "    Address: ");
  dump_address(0, 6, header->source.corporation.address);
  output(0, "    Phone 1: '%s'\n", str_val(header->source.corporation.phone[0]));
  output(0, "    Phone 2: '%s'\n", str_val(header->source.corporation.phone[1]));
  output(0, "    Phone 3: '%s'\n", str_val(header->source.corporation.phone[2]));
  output(0, "  Data:\n");
  output(0, "    Name: '%s'\n", str_val(header->source.data.name));
  output(0, "    Date: ");
  dump_date(0, 6, header->source.data.date);
  output(0, "    Copyright: '%s'\n", str_val(header->source.data.copyright));
  output(0, "Destination: '%s'\n", str_val(header->destination));
  output(0, "Date: ");
  dump_date(0, 2, header->date);
  output(0, "Time: '%s'\n", str_val(header->time));
  output(0, "Submitter: ");
  dump_xref(0, 2, header->submitter);
  output(0, "Submission: ");
  dump_xref(0, 2, header->submission);
  output(0, "File name: '%s'\n", str_val(header->filename));
  output(0, "Copyright: '%s'\n", str_val(header->copyright));
  output(0, "Gedcom:\n");
  output(0, "  Version: '%s'\n", str_val(header->gedcom.version));
  output(0, "  Form: '%s'\n", str_val(header->gedcom.form));
  output(0, "Character set:\n");
  output(0, "  Name: '%s'\n", str_val(header->charset.name));
  output(0, "  Version: '%s'\n", str_val(header->charset.version));
  output(0, "Language: '%s'\n", str_val(header->language));
  output(0, "Place hierarchy: '%s'\n", str_val(header->place_hierarchy));
  output(0, "Note:\n");
  output(0, "====\n");
  output(0, "%s\n", str_val(header->note));
  output(0, "====\n");
  output(0, "User data: ");
  dump_user_data(0, 2, header->extra);
}

void dump_submission()
{
  struct submission* subn = gom_get_submission();
  if (subn) {
    output(1, "=== SUBMISSION (%s) ===\n", str_val(subn->xrefstr));
    output(0, "Submitter: ");
    dump_xref(0, 2, subn->submitter);
    output(0, "Family file: '%s'\n", str_val(subn->family_file));
    output(0, "Temple code: '%s'\n", str_val(subn->temple_code));
    output(0, "Nr of ancestor generations: '%s'\n",
	   str_val(subn->nr_of_ancestor_gens));
    output(0, "Nr of descendant generations: '%s'\n",
	   str_val(subn->nr_of_descendant_gens));
    output(0, "Ordinance process flag: '%s'\n",
	   str_val(subn->ordinance_process_flag));
    output(0, "Record id: '%s'\n", str_val(subn->record_id));
    output(0, "User data: ");
    dump_user_data(0, 2, subn->extra);
  }
}

void dump_families()
{
  struct family* fam = gom_get_first_family();
  for (fam; fam; fam = fam->next) {
    output(1, "=== FAMILY (%s) ===\n", str_val(fam->xrefstr));
    output(0, "Family events: ");
    dump_events(0, 2, fam->event);
    output(0, "Husband: ");
    dump_xref(0, 2, fam->husband);
    output(0, "Wife: ");
    dump_xref(0, 2, fam->wife);
    output(0, "Children: ");
    dump_xref_list(0, 2, fam->children);
    output(0, "Number of children: '%s'\n", str_val(fam->nr_of_children));
    output(0, "Submitters: ");
    dump_xref_list(0, 2, fam->submitters);
    output(0, "LDS spouse sealings: ");
    dump_lds(0, 2, fam->lds_spouse_sealing);
    output(0, "citations: ");
    dump_citations(0, 2, fam->citation);
    output(0, "multimedia links: ");
    dump_mm_links(0, 2, fam->mm_link);
    output(0, "notes: ");
    dump_note_sub(0, 2, fam->note);
    output(0, "user refs: ");
    dump_user_ref(0, 2, fam->ref);
    output(0, "Record ID: '%s'\n", str_val(fam->record_id));
    output(0, "change date: ");
    dump_change_date(0, 2, fam->change_date);
    output(0, "User data: ");
    dump_user_data(0, 2, fam->extra);
  }
}

void dump_individuals()
{
  struct individual* indiv = gom_get_first_individual();
  for (indiv; indiv; indiv = indiv->next) {
    output(1, "=== INDIVIDUAL (%s) ===\n", str_val(indiv->xrefstr));
    output(0, "Restriction notice: '%s'\n", str_val(indiv->restriction_notice));
    output(0, "names: ");
    dump_personal_name(0, 2, indiv->name);
    output(0, "Sex: '%s'\n", str_val(indiv->sex));
    output(0, "Individual events: ");
    dump_events(0, 2, indiv->event);
    output(0, "Individual attributes: ");
    dump_events(0, 2, indiv->attribute);
    output(0, "LDS individual ordinance: ");
    dump_lds(0, 2, indiv->lds_individual_ordinance);
    output(0, "Child to family links: ");
    dump_family_link(0, 2, indiv->child_to_family);
    output(0, "Spouse to family links: ");
    dump_family_link(0, 2, indiv->spouse_to_family);
    output(0, "Submitters: ");
    dump_xref_list(0, 2, indiv->submitters);
    output(0, "Associations: ");
    dump_association(0, 2, indiv->association);
    output(0, "Aliases: ");
    dump_xref_list(0, 2, indiv->alias);
    output(0, "Ancestor interest: ");
    dump_xref_list(0, 2, indiv->ancestor_interest);
    output(0, "Descendant interest: ");
    dump_xref_list(0, 2, indiv->descendant_interest);
    output(0, "citations: ");
    dump_citations(0, 2, indiv->citation);
    output(0, "multimedia links: ");
    dump_mm_links(0, 2, indiv->mm_link);
    output(0, "notes: ");
    dump_note_sub(0, 2, indiv->note);
    output(0, "Record file nr: '%s'\n", str_val(indiv->record_file_nr));
    output(0, "Ancestral file nr: '%s'\n", str_val(indiv->ancestral_file_nr));
    output(0, "user refs: ");
    dump_user_ref(0, 2, indiv->ref);
    output(0, "Record ID: '%s'\n", str_val(indiv->record_id));
    output(0, "change date: ");
    dump_change_date(0, 2, indiv->change_date);
    output(0, "User data: ");
    dump_user_data(0, 2, indiv->extra);
  }
}

void dump_multimedia()
{
  struct multimedia* obj = gom_get_first_multimedia();
  for (obj; obj; obj = obj->next) {
    output(1, "=== MULTIMEDIA (%s) ===\n", str_val(obj->xrefstr));
    output(0, "Form: '%s'\n", str_val(obj->form));
    output(0, "Title: '%s'\n", str_val(obj->title));
    output(0, "notes: ");
    dump_note_sub(0, 2, obj->note);
    output(0, "Data: '%s'\n", str_val(obj->data));
    output(0, "Continued: ");
    dump_xref(0, 2, obj->continued);
    output(0, "user refs: ");
    dump_user_ref(0, 2, obj->ref);
    output(0, "Record ID: '%s'\n", str_val(obj->record_id));
    output(0, "change date: ");
    dump_change_date(0, 2, obj->change_date);
    output(0, "User data: ");
    dump_user_data(0, 2, obj->extra);
  }  
}

void dump_notes()
{
  struct note* note = gom_get_first_note();
  for (note; note; note = note->next) {
    output(1, "=== NOTE (%s) ===\n", str_val(note->xrefstr));
    output(0, "Text: '%s'\n", str_val(note->text));
    output(0, "citations: ");
    dump_citations(0, 2, note->citation);
    output(0, "user refs: ");
    dump_user_ref(0, 2, note->ref);
    output(0, "Record ID: '%s'\n", str_val(note->record_id));
    output(0, "change date: ");
    dump_change_date(0, 2, note->change_date);
    output(0, "User data: ");
    dump_user_data(0, 2, note->extra);
  }  
}

void dump_repositories()
{
  struct repository* repo = gom_get_first_repository();
  for (repo; repo; repo = repo->next) {
    output(1, "=== REPOSITORY (%s) ===\n", str_val(repo->xrefstr));
    output(0, "Name: '%s'\n", str_val(repo->name));
    output(0, "Address: ");
    dump_address(0, 2, repo->address);
    output(0, "Phone 1: '%s'\n", str_val(repo->phone[0]));
    output(0, "Phone 2: '%s'\n", str_val(repo->phone[1]));
    output(0, "Phone 3: '%s'\n", str_val(repo->phone[2]));
    output(0, "notes: ");
    dump_note_sub(0, 2, repo->note);
    output(0, "user refs: ");
    dump_user_ref(0, 2, repo->ref);
    output(0, "Record ID: '%s'\n", str_val(repo->record_id));
    output(0, "change date: ");
    dump_change_date(0, 2, repo->change_date);
    output(0, "User data: ");
    dump_user_data(0, 2, repo->extra);
  }  
}

void dump_sources()
{
  struct source* sour = gom_get_first_source();
  for (sour; sour; sour = sour->next) {
    output(1, "=== SOURCE (%s) ===\n", str_val(sour->xrefstr));
    output(0, "Data: \n");
    output(0, "  events: ");
    dump_source_events(0, 4, sour->data.event);
    output(0, "  Agency: '%s'\n", str_val(sour->data.agency));
    output(0, "  notes: ");
    dump_note_sub(0, 4, sour->data.note);
    output(0, "Author: '%s'\n", str_val(sour->author));
    output(0, "Title: '%s'\n", str_val(sour->title));
    output(0, "Abbreviation: '%s'\n", str_val(sour->abbreviation));
    output(0, "Publication: '%s'\n", str_val(sour->publication));
    output(0, "Text: '%s'\n", str_val(sour->text));
    output(0, "Repository:\n");
    output(0, "  Link: ");
    dump_xref(0, 4, sour->repository.link);
    output(0, "  notes: ");
    dump_note_sub(0, 4, sour->repository.note);
    output(0, "  source descriptions: ");
    dump_source_descriptions(0, 4, sour->repository.description);
    output(0, "multimedia links: ");
    dump_mm_links(0, 2, sour->mm_link);
    output(0, "notes: ");
    dump_note_sub(0, 2, sour->note);
    output(0, "user refs: ");
    dump_user_ref(0, 2, sour->ref);
    output(0, "Record ID: '%s'\n", str_val(sour->record_id));
    output(0, "change date: ");
    dump_change_date(0, 2, sour->change_date);
    output(0, "User data: ");
    dump_user_data(0, 2, sour->extra);
  }  
}

void dump_submitters()
{
  struct submitter* subm = gom_get_first_submitter();
  for (subm; subm; subm = subm->next) {
    output(1, "=== SUBMITTER (%s) ===\n", str_val(subm->xrefstr));
    output(0, "Name: '%s'\n", str_val(subm->name));
    output(0, "Address: ");
    dump_address(0, 2, subm->address);
    output(0, "Phone 1: '%s'\n", str_val(subm->phone[0]));
    output(0, "Phone 2: '%s'\n", str_val(subm->phone[1]));
    output(0, "Phone 3: '%s'\n", str_val(subm->phone[2]));
    output(0, "multimedia links: ");
    dump_mm_links(0, 2, subm->mm_link);
    output(0, "Language 1: '%s'\n", str_val(subm->language[0]));
    output(0, "Language 2: '%s'\n", str_val(subm->language[1]));
    output(0, "Language 3: '%s'\n", str_val(subm->language[2]));
    output(0, "Record file nr: '%s'\n", str_val(subm->record_file_nr));
    output(0, "Record ID: '%s'\n", str_val(subm->record_id));
    output(0, "change date: ");
    dump_change_date(0, 2, subm->change_date);
    output(0, "User data: ");
    dump_user_data(0, 2, subm->extra);
  }  
}

void dump_user_records()
{
  struct user_rec* rec = gom_get_first_user_rec();
  for (rec; rec; rec = rec->next) {
    output(1, "=== USER RECORD (%s) ===\n", str_val(rec->xrefstr));
    output(0, "Tag: '%s'\n", rec->tag);
    output(0, "String value: '%s'\n", str_val(rec->str_value));
    output(0, "Xref value: ");
    dump_xref(0, 2, rec->xref_value);
    output(0, "User data: ");
    dump_user_data(0, 2, rec->extra);
  }  
}

void show_data()
{
  dump_header();
  dump_submission();
  dump_families();
  dump_individuals();
  dump_multimedia();
  dump_notes();
  dump_repositories();
  dump_sources();
  dump_submitters();
  dump_user_records();
}
