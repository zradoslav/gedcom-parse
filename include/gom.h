/* External header for the Gedcom parser library.
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

#ifndef __GEDCOM_GOM_H
#define __GEDCOM_GOM_H

#include <sys/cdefs.h>

__BEGIN_DECLS

/* Sub-structures */

struct user_data {
  int level;
  char *tag;
  char *str_value;
  struct xref_value *xref_value;
  struct user_data *next;
  struct user_data *previous;
};

struct address {
  char *full_label;
  char *line1;
  char *line2;
  char *city;
  char *state;
  char *postal;
  char *country;
  struct user_data *extra;
};

struct text {
  char *text;
  struct user_data *extra;
  struct text *next;
  struct text *previous;
};

struct source_citation {
  char *description;
  struct xref_value *reference;
  char *page;
  char *event;
  char *role;
  struct date_value *date;
  struct text *text;
  char *quality;
  struct multimedia_link *mm_link;
  struct note_sub *note;
  struct user_data *extra;
  struct source_citation *next;
  struct source_citation *previous;
};

struct note_sub {
  char *text;
  struct xref_value *reference;
  struct source_citation *citation;
  struct user_data *extra;
  struct note_sub *next;
  struct note_sub *previous;
};

struct place {
  char *value;
  char *place_hierarchy;
  struct user_data *extra;
  struct source_citation *citation;
  struct note_sub *note;
};

struct multimedia_link {
  struct xref_value *reference;
  char *form;
  char *title;
  char *file;
  struct note_sub *note;
  struct user_data *extra;
  struct multimedia_link *next;
  struct multimedia_link *previous;
};

struct lds_event {
  char *date_status;
  struct date_value *date;
  char *temple_code;
  char *place_living_ordinance;
  struct xref_value *family;
  struct source_citation *citation;
  struct note_sub *note;
  struct user_data *extra;
  struct lds_event *next;
  struct lds_event *previous;
};

struct user_ref_number {
  char *value;
  char *type;
  struct user_data *extra;
  struct user_ref_number *next;
  struct user_ref_number *previous;
};

struct change_date {
  struct date_value *date;
  char *time;
  struct note_sub *note;
  struct user_data *extra;
};

struct event {
  int event;
  char *event_name;
  char *val;
  char *type;
  struct date_value *date;
  struct place *place;
  struct address *address;
  char *phone[3];
  struct age_value *age;
  char *agency;
  char *cause;
  struct source_citation *citation;
  struct multimedia_link *mm_link;
  struct note_sub *note;
  struct age_value *husband_age;
  struct age_value *wife_age;
  struct xref_value *family;
  char *adoption_parent;
  struct user_data *extra;
  struct event *next;
  struct event *previous;
};

struct xref_list {
  struct xref_value *xref;
  struct user_data *extra;
  struct xref_list *next;
  struct xref_list *previous;
};

struct personal_name {
  char *name;
  char *prefix;
  char *given;
  char *nickname;
  char *surname_prefix;
  char *surname;
  char *suffix;
  struct source_citation *citation;
  struct note_sub *note;
  struct user_data *extra;
  struct personal_name *next;
  struct personal_name *previous;
};

struct pedigree {
  char *pedigree;
  struct user_data *extra;
  struct pedigree *next;
  struct pedigree *previous;
};

struct family_link {
  struct xref_value *family;
  struct pedigree *pedigree;
  struct note_sub *note;
  struct user_data *extra;
  struct family_link *next;
  struct family_link *previous;
};

struct association {
  struct xref_value *to;
  char *type;
  char *relation;
  struct source_citation *citation;
  struct note_sub *note;
  struct user_data *extra;
  struct association *next;
  struct association *previous;
};

struct source_event {
  char *recorded_events;
  struct date_value *date_period;
  char *jurisdiction;
  struct user_data *extra;
  struct source_event *next;
  struct source_event *previous;
};

struct source_description {
  char *call_number;
  char *media;
  struct user_data *extra;
  struct source_description *next;
  struct source_description *previous;
};

/* Main structures */

struct header {
  struct header_source {
    char *id;
    char *name;
    char *version;
    struct header_corporation {
      char *name;
      struct address *address;
      char *phone[3];
    } corporation;
    struct header_data {
      char *name;
      struct date_value *date;
      char *copyright;
    } data;
  } source;
  char *destination;
  struct date_value *date;
  char *time;
  struct xref_value *submitter;
  struct xref_value *submission;
  char *filename;
  char *copyright;
  struct header_gedcom {
    char *version;
    char *form;
  } gedcom;
  struct header_charset {
    char *name;
    char *version;
  } charset;
  char *language;
  char *place_hierarchy;
  char *note;
  struct user_data *extra;
};

struct submission {
  char *xrefstr;
  struct xref_value *submitter;
  char *family_file;
  char *temple_code;
  char *nr_of_ancestor_gens;
  char *nr_of_descendant_gens;
  char *ordinance_process_flag;
  char *record_id;
  struct user_data *extra;
};

struct family {
  char *xrefstr;
  struct event *event;
  struct xref_value *husband;
  struct xref_value *wife;
  struct xref_list *children;
  char *nr_of_children;
  struct xref_list *submitters;
  struct lds_event *lds_spouse_sealing;
  struct source_citation *citation;
  struct multimedia_link *mm_link;
  struct note_sub *note;
  struct user_ref_number *ref;
  char *record_id;
  struct change_date *change_date;
  struct user_data *extra;
  struct family *next;
  struct family *previous;
};

struct individual {
  char *xrefstr;
  char *restriction_notice;
  struct personal_name *name;
  char *sex;
  struct event *event;
  struct event *attribute;
  struct lds_event *lds_individual_ordinance;
  struct family_link *child_to_family;
  struct family_link *spouse_to_family;
  struct xref_list *submitters;
  struct association *association;
  struct xref_list *alias;
  struct xref_list *ancestor_interest;
  struct xref_list *descendant_interest;
  struct source_citation *citation;
  struct multimedia_link *mm_link;
  struct note_sub *note;
  char *record_file_nr;
  char *ancestral_file_nr;
  struct user_ref_number *ref;
  char *record_id;
  struct change_date *change_date;
  struct user_data *extra;
  struct individual *next;
  struct individual *previous;
};

struct multimedia {
  char *xrefstr;
  char *form;
  char *title;
  struct note_sub *note;
  char *data;
  struct xref_value *continued;
  struct user_ref_number *ref;
  char *record_id;
  struct change_date *change_date;
  struct user_data *extra;
  struct multimedia *next;
  struct multimedia *previous;
};

struct note {
  char *xrefstr;
  char *text;
  struct source_citation *citation;
  struct user_ref_number *ref;
  char *record_id;
  struct change_date *change_date;
  struct user_data *extra;
  struct note *next;
  struct note *previous;
};

struct repository {
  char *xrefstr;
  char *name;
  struct address *address;
  char *phone[3];
  struct note_sub *note;
  struct user_ref_number *ref;
  char *record_id;
  struct change_date *change_date;
  struct user_data *extra;
  struct repository *next;
  struct repository *previous;
};

struct source {
  char *xrefstr;
  struct source_data {
    struct source_event *event;
    char *agency;
    struct note_sub *note;
  } data;
  char *author;
  char *title;
  char *abbreviation;
  char *publication;
  char *text;
  struct repo_link {
    struct xref_value *link;
    struct note_sub *note;
    struct source_description *description;
  } repository;
  struct multimedia_link *mm_link;
  struct note_sub *note;
  struct user_ref_number *ref;
  char *record_id;
  struct change_date *change_date;
  struct user_data *extra;
  struct source *next;
  struct source *previous;
};

struct submitter {
  char *xrefstr;
  char *name;
  struct address *address;
  char *phone[3];
  struct multimedia_link *mm_link;
  char *language[3];
  char *record_file_nr;
  char *record_id;
  struct change_date *change_date;
  struct user_data *extra;
  struct submitter *next;
  struct submitter *previous;
};

struct user_rec {
  char *xrefstr;
  char *tag;
  char *str_value;
  struct xref_value *xref_value;
  struct user_data *extra;
  struct user_rec *next;
  struct user_rec *previous;
};

/* Functions */

int  gom_parse_file(char *file_name);

struct header*     gom_get_header();
struct submission* gom_get_submission();

struct family*     gom_get_first_family();
struct family*     gom_get_family_by_xref(char *xref);

struct individual* gom_get_first_individual();
struct individual* gom_get_individual_by_xref(char *xref);

struct multimedia* gom_get_first_multimedia();
struct multimedia* gom_get_multimedia_by_xref(char *xref);

struct note*       gom_get_first_note();
struct note*       gom_get_note_by_xref(char *xref);

struct repository* gom_get_first_repository();
struct repository* gom_get_repository_by_xref(char *xref);

struct source*     gom_get_first_source();
struct source*     gom_get_source_by_xref(char *xref);

struct submitter*  gom_get_first_submitter();
struct submitter*  gom_get_submitter_by_xref(char *xref);

struct user_rec*   gom_get_first_user_rec();
struct user_rec*   gom_get_user_rec_by_xref(char *xref);

__END_DECLS

#endif /* __GEDCOM_GOM_H */
