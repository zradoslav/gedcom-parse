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

#include "gedcom.h"
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Sub-structures */

struct user_data {
  int level;
  char *tag;
  char *str_value;
  struct xref_value *xref_value;
  struct user_data *next;
  struct user_data *previous;
};

struct address {                      /* ADDRESS_STRUCTURE */
  char *full_label;                   /* ADDRESS_LINE */
  char *line1;                        /* ADDRESS_LINE1 */
  char *line2;                        /* ADDRESS_LINE2 */
  char *city;                         /* ADDRESS_CITY */
  char *state;                        /* ADDRESS_STATE */
  char *postal;                       /* ADDRESS_POSTAL_CODE */
  char *country;                      /* ADDRESS_COUNTRY */
  struct user_data *extra;
};

struct text {
  char *text;                         /* TEXT_FROM_SOURCE */
  struct user_data *extra;
  struct text *next;
  struct text *previous;
};

struct source_citation {              /* SOURCE_CITATION */
  char *description;                  /* SOURCE_DESCRIPTION */
  struct xref_value *reference;
  char *page;                         /* WHERE_WITHIN_SOURCE */
  char *event;                        /* EVENT_TYPE_CITED_FROM */
  char *role;                         /* ROLE_IN_EVENT */
  struct date_value *date;            /* ENTRY_RECORDING_DATE */
  struct text *text;
  char *quality;                      /* CERTAINTY_ASSESSMENT */
  struct multimedia_link *mm_link;
  struct note_sub *note;
  struct user_data *extra;
  struct source_citation *next;
  struct source_citation *previous;
};

struct note_sub {                     /* NOTE_STRUCTURE */
  char *text;                         /* SUBMITTER_TEXT */
  struct xref_value *reference;
  struct source_citation *citation;
  struct user_data *extra;
  struct note_sub *next;
  struct note_sub *previous;
};

struct place {                        /* PLACE_STRUCTURE */
  char *value;                        /* PLACE_VALUE */
  char *place_hierarchy;              /* PLACE_HIERARCHY */
  struct source_citation *citation;
  struct note_sub *note;
  struct user_data *extra;
};

struct multimedia_link {              /* MULTIMEDIA_LINK */
  struct xref_value *reference;
  char *form;                         /* MULTIMEDIA_FORMAT */
  char *title;                        /* DESCRIPTIVE_TITLE */
  char *file;                         /* MULTIMEDIA_FILE_REFERENCE */
  struct note_sub *note;
  struct user_data *extra;
  struct multimedia_link *next;
  struct multimedia_link *previous;
};

struct lds_event {                    /* LDS_INDIVIDUAL_ORDINANCE */
  int event;
  char *event_name;
  char *date_status;                  /* LDS_BAPTISM_DATE_STATUS */
  struct date_value *date;            /* DATE_LDS_ORD */
  char *temple_code;                  /* TEMPLE_CODE */
  char *place_living_ordinance;       /* PLACE_LIVING_ORDINANCE */
  struct xref_value *family;
  struct source_citation *citation;
  struct note_sub *note;
  struct user_data *extra;
  struct lds_event *next;
  struct lds_event *previous;
};

struct user_ref_number {
  char *value;                        /* USER_REFERENCE_NUMBER */
  char *type;                         /* USER_REFERENCE_TYPE */
  struct user_data *extra;
  struct user_ref_number *next;
  struct user_ref_number *previous;
};

struct change_date {                  /* CHANGE_DATE_STRUCTURE */
  struct date_value *date;            /* CHANGE_DATE */
  char *time;                         /* TIME_VALUE */
  struct note_sub *note;
  struct user_data *extra;
};

struct event {                        /* FAMILY_EVENT_STRUCTURE */
  int event;
  char *event_name;
  char *val;
  char *type;                         /* EVENT_DESCRIPTOR */
  struct date_value *date;            /* DATE_VALUE */
  struct place *place;
  struct address *address;
  char *phone[3];                     /* PHONE_NUMBER */
  struct age_value *age;              /* AGE_AT_EVENT */
  char *agency;                       /* RESPONSIBLE_AGENCY */
  char *cause;                        /* CAUSE_OF_EVENT */
  struct source_citation *citation;
  struct multimedia_link *mm_link;
  struct note_sub *note;
  struct age_value *husband_age;
  struct age_value *wife_age;
  struct xref_value *family;
  char *adoption_parent;              /* ADOPTED_BY_WHICH_PARENT */
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

struct personal_name {                /* PERSONAL_NAME_STRUCTURE */
  char *name;                         /* NAME_PERSONAL */
  char *prefix;                       /* NAME_PIECE_PREFIX */
  char *given;                        /* NAME_PIECE_GIVEN */
  char *nickname;                     /* NAME_PIECE_NICKNAME */
  char *surname_prefix;               /* NAME_PIECE_SURNAME_PREFIX */
  char *surname;                      /* NAME_PIECE_SURNAME */
  char *suffix;                       /* NAME_PIECE_SUFFIX */
  struct source_citation *citation;
  struct note_sub *note;
  struct user_data *extra;
  struct personal_name *next;
  struct personal_name *previous;
};

struct pedigree {
  char *pedigree;                     /* PEDIGREE_LINKAGE_TYPE */
  struct user_data *extra;
  struct pedigree *next;
  struct pedigree *previous;
};

struct family_link {                  /* CHILD_TO_FAMILY_LINK */
  struct xref_value *family;
  struct pedigree *pedigree;
  struct note_sub *note;
  struct user_data *extra;
  struct family_link *next;
  struct family_link *previous;
};

struct association {                  /* ASSOCIATION_STRUCTURE */
  struct xref_value *to;
  char *type;                         /* RECORD_TYPE */
  char *relation;                     /* RELATION_IS_DESCRIPTOR */
  struct source_citation *citation;
  struct note_sub *note;
  struct user_data *extra;
  struct association *next;
  struct association *previous;
};

struct source_event {
  char *recorded_events;              /* EVENTS_RECORDED */
  struct date_value *date_period;     /* DATE_PERIOD */
  char *jurisdiction;                 /* SOURCE_JURISDICTION_PLACE */
  struct user_data *extra;
  struct source_event *next;
  struct source_event *previous;
};

struct source_description {
  char *call_number;                  /* SOURCE_CALL_NUMBER */
  char *media;                        /* SOURCE_MEDIA_TYPE */
  struct user_data *extra;
  struct source_description *next;
  struct source_description *previous;
};

/* Main structures */

struct header {                       /* HEADER */
  struct header_source {
    char *id;                         /* APPROVED_SYSTEM_ID */
    char *name;                       /* NAME_OF_PRODUCT */
    char *version;                    /* VERSION_NUMBER */
    struct header_corporation {
      char *name;                     /* NAME_OF_BUSINESS */
      struct address *address;
      char *phone[3];                 /* PHONE_NUMBER */
    } corporation;
    struct header_data {
      char *name;                     /* NAME_OF_SOURCE_DATA */
      struct date_value *date;        /* PUBLICATION_DATE */
      char *copyright;                /* COPYRIGHT_SOURCE_DATA */
    } data;
  } source;
  char *destination;                  /* RECEIVING_SYSTEM_NAME */
  struct date_value *date;            /* TRANSMISSION_DATE */
  char *time;                         /* TIME_VALUE */
  struct xref_value *submitter;
  struct xref_value *submission;
  char *filename;                     /* FILE_NAME */
  char *copyright;                    /* COPYRIGHT_GEDCOM_FILE */
  struct header_gedcom {
    char *version;                    /* VERSION_NUMBER */
    char *form;                       /* GEDCOM_FORM */
  } gedcom;
  struct header_charset {
    char *name;                       /* CHARACTER_SET */
    char *version;                    /* VERSION_NUMBER */
  } charset;
  char *language;                     /* LANGUAGE_OF_TEXT */
  char *place_hierarchy;              /* PLACE_HIERARCHY */
  char *note;                         /* GEDCOM_CONTENT_DESCRIPTION */
  struct user_data *extra;
};

struct submission {                   /* SUBMISSION_RECORD */
  char *xrefstr;
  struct xref_value *submitter;
  char *family_file;                  /* NAME_OF_FAMILY_FILE */
  char *temple_code;                  /* TEMPLE_CODE */
  char *nr_of_ancestor_gens;          /* GENERATIONS_OF_ANCESTORS */
  char *nr_of_descendant_gens;        /* GENERATIONS_OF_DESCENDANTS */
  char *ordinance_process_flag;       /* ORDINANCE_PROCESS_FLAG */
  char *record_id;                    /* AUTOMATED_RECORD_ID */
  struct user_data *extra;
};

struct family {                       /* FAM_RECORD */
  char *xrefstr;
  struct event *event;
  struct xref_value *husband;
  struct xref_value *wife;
  struct xref_list *children;
  char *nr_of_children;               /* COUNT_OF_CHILDREN */
  struct xref_list *submitters;
  struct lds_event *lds_spouse_sealing;
  struct source_citation *citation;
  struct multimedia_link *mm_link;
  struct note_sub *note;
  struct user_ref_number *ref;
  char *record_id;                    /* AUTOMATED_RECORD_ID */
  struct change_date *change_date;
  struct user_data *extra;
  struct family *next;
  struct family *previous;
};

struct individual {                   /* INDIVIDUAL_RECORD */
  char *xrefstr;
  char *restriction_notice;           /* RESTRICTION_NOTICE */
  struct personal_name *name;
  char *sex;                          /* SEX_VALUE */
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
  char *record_file_nr;               /* PERMANENT_RECORD_FILE_NUMBER */
  char *ancestral_file_nr;            /* ANCESTRAL_FILE_NUMBER */
  struct user_ref_number *ref;
  char *record_id;                    /* AUTOMATED_RECORD_ID */
  struct change_date *change_date;
  struct user_data *extra;
  struct individual *next;
  struct individual *previous;
};

struct multimedia {                   /* MULTIMEDIA_RECORD */
  char *xrefstr;
  char *form;                         /* MULTIMEDIA_FORMAT */
  char *title;                        /* DESCRIPTIVE_TITLE */
  struct note_sub *note;
  char *data;                         /* ENCODED_MULTIMEDIA_LINE */
  struct xref_value *continued;
  struct user_ref_number *ref;
  char *record_id;                    /* AUTOMATED_RECORD_ID */
  struct change_date *change_date;
  struct user_data *extra;
  struct multimedia *next;
  struct multimedia *previous;
};

struct note {                         /* NOTE_RECORD */
  char *xrefstr;
  char *text;                         /* SUBMITTER_TEXT */
  struct source_citation *citation;
  struct user_ref_number *ref;
  char *record_id;                    /* AUTOMATED_RECORD_ID */
  struct change_date *change_date;
  struct user_data *extra;
  struct note *next;
  struct note *previous;
};

struct repository {                   /* REPOSITORY_RECORD */
  char *xrefstr;
  char *name;                         /* NAME_OF_REPOSITORY */
  struct address *address;
  char *phone[3];                     /* PHONE_NUMBER */
  struct note_sub *note;
  struct user_ref_number *ref;
  char *record_id;                    /* AUTOMATED_RECORD_ID */
  struct change_date *change_date;
  struct user_data *extra;
  struct repository *next;
  struct repository *previous;
};

struct source {                       /* SOURCE_RECORD */
  char *xrefstr;
  struct source_data {
    struct source_event *event;
    char *agency;                     /* RESPONSIBLE_AGENCY */
    struct note_sub *note;
  } data;
  char *author;                       /* SOURCE_ORIGINATOR */
  char *title;                        /* SOURCE_DESCRIPTIVE_TITLE */
  char *abbreviation;                 /* SOURCE_FILED_BY_ENTRY */
  char *publication;                  /* SOURCE_PUBLICATION_FACTS */
  char *text;                         /* TEXT_FROM_SOURCE */
  struct repo_link {
    struct xref_value *link;
    struct note_sub *note;
    struct source_description *description;
  } repository;
  struct multimedia_link *mm_link;
  struct note_sub *note;
  struct user_ref_number *ref;
  char *record_id;                    /* AUTOMATED_RECORD_ID */
  struct change_date *change_date;
  struct user_data *extra;
  struct source *next;
  struct source *previous;
};

struct submitter {                    /* SUBMITTER_RECORD */
  char *xrefstr;
  char *name;                         /* SUBMITTER_NAME */
  struct address *address;
  char *phone[3];                     /* PHONE_NUMBER */
  struct multimedia_link *mm_link;
  char *language[3];                  /* LANGUAGE_PREFERENCE */
  char *record_file_nr;               /* SUBMITTER_REGISTERED_RFN */
  char *record_id;                    /* AUTOMATED_RECORD_ID */
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

  /** \addtogroup gommain */
  /** @{ */
  /** \brief Parses an existing Gedcom file */
int  gom_parse_file(const char *file_name);
  /** \brief Starts a new Gedcom model */
int  gom_new_model();
  /** @} */

  /** \addtogroup gom_write */
  /** @{ */
  /** \brief Write a Gedcom file */
int  gom_write_file(const char* file_name, int *total_conv_fails);
  /** \brief Update the timestamp in a Gedcom model */
int  gom_header_update_timestamp(time_t t);
  /** @} */
  
  /** \addtogroup gomget */
  /** @{ */
  /** \brief Retrieve header object
      \return The header object */
struct header*     gom_get_header();
  /** \brief Retrieve submission object
      \return The submission object, or \c NULL if there is none */
struct submission* gom_get_submission();
  /** \brief Retrieve first family object
      \return The first family object, or \c NULL if there is none */
struct family*     gom_get_first_family();
  /** \brief Retrieve family object by cross-reference
      \param xref The cross-reference key of the object to retrieve
      \return The object referenced by \c xref, \c NULL if it doesn't exist
              or is not a family object */
struct family*     gom_get_family_by_xref(const char *xref);
  /** \brief Retrieve first individual object
      \return The first individual object, or \c NULL if there is none */
struct individual* gom_get_first_individual();
  /** \brief Retrieve individual object by cross-reference
      \param xref The cross-reference key of the object to retrieve
      \return The object referenced by \c xref, \c NULL if it doesn't exist
              or is not an individual object */
struct individual* gom_get_individual_by_xref(const char *xref);
  /** \brief Retrieve first multimedia object
      \return The first multimedia object, or \c NULL if there is none */
struct multimedia* gom_get_first_multimedia();
  /** \brief Retrieve multimedia object by cross-reference
      \param xref The cross-reference key of the object to retrieve
      \return The object referenced by \c xref, \c NULL if it doesn't exist
              or is not a multimedia object */
struct multimedia* gom_get_multimedia_by_xref(const char *xref);
  /** \brief Retrieve first note object
      \return The first note object, or \c NULL if there is none */
struct note*       gom_get_first_note();
  /** \brief Retrieve note object by cross-reference
      \param xref The cross-reference key of the object to retrieve
      \return The object referenced by \c xref, \c NULL if it doesn't exist
              or is not a note object */
struct note*       gom_get_note_by_xref(const char *xref);
  /** \brief Retrieve first repository object
      \return The first repository object, or \c NULL if there is none */
struct repository* gom_get_first_repository();
  /** \brief Retrieve repository object by cross-reference
      \param xref The cross-reference key of the object to retrieve
      \return The object referenced by \c xref, \c NULL if it doesn't exist
              or is not a repository object */
struct repository* gom_get_repository_by_xref(const char *xref);
  /** \brief Retrieve first source object
      \return The first source object, or \c NULL if there is none */
struct source*     gom_get_first_source();
  /** \brief Retrieve source object by cross-reference
      \param xref The cross-reference key of the object to retrieve
      \return The object referenced by \c xref, \c NULL if it doesn't exist
              or is not a source object */
struct source*     gom_get_source_by_xref(const char *xref);
  /** \brief Retrieve first submitter object
      \return The first submitter object, or \c NULL if there is none */
struct submitter*  gom_get_first_submitter();
  /** \brief Retrieve submitter object by cross-reference
      \param xref The cross-reference key of the object to retrieve
      \return The object referenced by \c xref, \c NULL if it doesn't exist
              or is not a submitter object */
struct submitter*  gom_get_submitter_by_xref(const char *xref);
  /** \brief Retrieve first user record object
      \return The first user record object, or \c NULL if there is none */
struct user_rec*   gom_get_first_user_rec();
  /** \brief Retrieve user record object by cross-reference
      \param xref The cross-reference key of the object to retrieve
      \return The object referenced by \c xref, \c NULL if it doesn't exist
              or is not a user record object */
struct user_rec*   gom_get_user_rec_by_xref(const char *xref);
  /** @} */

  /** \addtogroup gom_add_rec */
  /** @{ */
  /** \brief Add a submission record
      \param xrefstr The cross-reference key for the new object
      \return The new object if successful, \c NULL if an error happened (e.g.
      the cross-reference is already in use */
struct submission* gom_new_submission(const char* xrefstr);
  /** \brief Delete the submission record
      \retval 0 if successful
      \retval 1 if error */
int                gom_delete_submission();

  /** \brief Add a new family record
      \param xrefstr The cross-reference key for the new object
      \return The new object if successful, \c NULL if an error happened (e.g.
      the cross-reference is already in use */
struct family*     gom_new_family(const char* xrefstr);
  /** \brief Delete an existing family record
      \param obj The object to be deleted
      \retval 0 if successful
      \retval 1 if error */
int                gom_delete_family(struct family* obj);

  /** \brief Add a new individual record
      \param xrefstr The cross-reference key for the new object
      \return The new object if successful, \c NULL if an error happened (e.g.
      the cross-reference is already in use */
struct individual* gom_new_individual(const char* xrefstr);
  /** \brief Delete an existing individual record
      \param obj The object to be deleted
      \retval 0 if successful
      \retval 1 if error */
int                gom_delete_individual(struct individual* obj);

  /** \brief Add a new multimedia record
      \param xrefstr The cross-reference key for the new object
      \return The new object if successful, \c NULL if an error happened (e.g.
      the cross-reference is already in use */
struct multimedia* gom_new_multimedia(const char* xrefstr);
  /** \brief Delete an existing multimedia record
      \param obj The object to be deleted
      \retval 0 if successful
      \retval 1 if error */
int                gom_delete_multimedia(struct multimedia* obj);

  /** \brief Add a new note record
      \param xrefstr The cross-reference key for the new object
      \return The new object if successful, \c NULL if an error happened (e.g.
      the cross-reference is already in use */
struct note*       gom_new_note(const char* xrefstr);
  /** \brief Delete an existing note record
      \param obj The object to be deleted
      \retval 0 if successful
      \retval 1 if error */
int                gom_delete_note(struct note* obj);

  /** \brief Add a new repository record
      \param xrefstr The cross-reference key for the new object
      \return The new object if successful, \c NULL if an error happened (e.g.
      the cross-reference is already in use */
struct repository* gom_new_repository(const char* xrefstr);
  /** \brief Delete an existing repository record
      \param obj The object to be deleted
      \retval 0 if successful
      \retval 1 if error */
int                gom_delete_repository(struct repository* obj);

  /** \brief Add a new source record
      \param xrefstr The cross-reference key for the new object
      \return The new object if successful, \c NULL if an error happened (e.g.
      the cross-reference is already in use */
struct source*     gom_new_source(const char* xrefstr);
  /** \brief Delete an existing source record
      \param obj The object to be deleted
      \retval 0 if successful
      \retval 1 if error */
int                gom_delete_source(struct source* obj);

  /** \brief Add a new submitter record
      \param xrefstr The cross-reference key for the new object
      \return The new object if successful, \c NULL if an error happened (e.g.
      the cross-reference is already in use */
struct submitter*  gom_new_submitter(const char* xrefstr);
  /** \brief Delete an existing submitter record
      \param obj The object to be deleted
      \retval 0 if successful
      \retval 1 if error */
int                gom_delete_submitter(struct submitter* obj);

  /** \brief Add a new user record
      \param xrefstr The cross-reference key for the new object
      \param tag The tag for the new object
      \return The new object if successful, \c NULL if an error happened (e.g.
      the cross-reference is already in use */
struct user_rec*   gom_new_user_rec(const char* xrefstr, const char* tag);
  /** \brief Delete an existing user record
      \param obj The object to be deleted
      \retval 0 if successful
      \retval 1 if error */
int                gom_delete_user_rec(struct user_rec* obj);
  /** @} */

  /** \addtogroup gom_mod_string */
  /** @{ */
  /** \brief Get string in UTF-8 encoding */
char* gom_get_string(char* data);
  /** \brief Modify string using UTF-8 encoding */
char* gom_set_string(char** data, const char* utf8_str);

  /** \brief Get string in encoding defined by current locale */
char* gom_get_string_for_locale(char* data, int* conversion_failures);
  /** \brief Modify string using encoding defined by current locale */
char* gom_set_string_for_locale(char** data, const char* locale_str);
  /** \brief Set the replacement string for conversion failures */
void  gom_set_unknown(const char* unknown);
  /** @} */

  /** \addtogroup gom_add_xref */
  /** @{ */
  /** \brief Direction of moving cross-references or sub-structures. */
enum _DIR {
  MOVE_UP,    /**< Move up */
  MOVE_DOWN   /**< Move down */
};

  /** \brief Direction of moving cross-references or sub-structures. */
  typedef enum _DIR Gom_direction;

  /** \brief Modify a cross-reference */
struct xref_value* gom_set_xref(struct xref_value** data, const char* xref);
  
  /** \brief Add a cross-reference in a cross-reference list */
struct xref_list*  gom_add_xref(struct xref_list** data, const char* xref);
  /** \brief Remove a cross-reference from a cross-reference list */
int                gom_remove_xref(struct xref_list** data, const char* xref);
  /** \brief Move a cross-reference up or down in a cross-reference list */
int                gom_move_xref(Gom_direction dir, struct xref_list** data,
				 const char* xref);
  /** @} */

  /** \addtogroup gom_add_sub */
  /** @{ */
  /** \brief Create new address substructure */
struct address*    gom_set_new_address(struct address** obj);
  /** \brief Delete an address substructure */
int                gom_delete_address(struct address** obj);

  /** \brief Add new association substructure to the list */
struct association* gom_add_new_association(struct association** data);
  /** \brief Remove an association substructure to the list */
int                gom_remove_association(struct association** data,
					  struct association* obj);
  /** \brief Move an association substructure up or down in the list */
int                gom_move_association(Gom_direction dir,
					struct association** data,
					struct association* obj);
  
  /** \brief Create new change_date substructure */
struct change_date* gom_set_new_change_date(struct change_date** obj);
  /** \brief Delete a change_date substructure */
int                 gom_delete_change_date(struct change_date** obj);
  /** \brief Update the timestamp in a change_date substructure */
int                 gom_update_timestamp(struct change_date** obj, time_t t);

  /** \brief Add new event substructure to the list */
struct event*      gom_add_new_event(struct event** data);
  /** \brief Remove an event substructure to the list */
int                gom_remove_event(struct event** data, struct event* obj);
  /** \brief Move an event substructure up or down in the list */
int                gom_move_event(Gom_direction dir, struct event** data,
				  struct event* obj);
  
  /** \brief Add new family_link substructure to the list */
struct family_link* gom_add_new_family_link(struct family_link** data);
  /** \brief Remove a family_link substructure to the list */
int                gom_remove_family_link(struct family_link** data,
					  struct family_link* obj);
  /** \brief Move a family_link substructure up or down in the list */
int                gom_move_family_link(Gom_direction dir,
					struct family_link** data,
					struct family_link* obj);
  
  /** \brief Add new lds_event substructure to the list */
struct lds_event*  gom_add_new_lds_event(struct lds_event** data);
  /** \brief Remove an lds_event substructure to the list */
int                gom_remove_lds_event(struct lds_event** data,
					struct lds_event* obj);
  /** \brief Move an lds_event substructure up or down in the list */
int                gom_move_lds_event(Gom_direction dir,
				      struct lds_event** data,
				      struct lds_event* obj);
  
  /** \brief Add new multimedia_link substructure to the list */
struct multimedia_link*
                   gom_add_new_multimedia_link(struct multimedia_link** data);
  /** \brief Remove a multimedia_link substructure to the list */
int                gom_remove_multimedia_link(struct multimedia_link** data,
					      struct multimedia_link* obj);
  /** \brief Move a multimedia_link substructure up or down in the list */
int                gom_move_multimedia_link(Gom_direction dir,
					    struct multimedia_link** data,
					    struct multimedia_link* obj);
  
  /** \brief Add new note substructure to the list */
struct note_sub*   gom_add_new_note_sub(struct note_sub** data);
  /** \brief Remove a note substructure to the list */
int                gom_remove_note_sub(struct note_sub** data,
				       struct note_sub* obj);
  /** \brief Move a note substructure up or down in the list */
int                gom_move_note_sub(Gom_direction dir,
				     struct note_sub** data,
				     struct note_sub* obj);
  
  /** \brief Add new pedigree substructure to the list */
struct pedigree*   gom_add_new_pedigree(struct pedigree** data);
  /** \brief Remove a pedigree substructure to the list */
int                gom_remove_pedigree(struct pedigree** data,
				       struct pedigree* obj);
  /** \brief Move a pedigree substructure up or down in the list */
int                gom_move_pedigree(Gom_direction dir,
				     struct pedigree** data,
				     struct pedigree* obj);
  
  /** \brief Add new personal_name substructure to the list */
struct personal_name* gom_add_new_personal_name(struct personal_name** data);
  /** \brief Remove a personal_name substructure to the list */
int                gom_remove_personal_name(struct personal_name** data,
					    struct personal_name* obj);
  /** \brief Move a personal_name substructure up or down in the list */
int                gom_move_personal_name(Gom_direction dir,
					  struct personal_name** data,
					  struct personal_name* obj);
  
  /** \brief Create new place substructure */
struct place*      gom_set_new_place(struct place** obj);
  /** \brief Delete a place substructure */
int                gom_delete_place(struct place** obj);

  /** \brief Add new source_citation substructure to the list */
struct source_citation*
                   gom_add_new_source_citation(struct source_citation** data);
  /** \brief Remove a source_citation substructure to the list */
int                gom_remove_source_citation(struct source_citation** data,
					      struct source_citation* obj);
  /** \brief Move a source_citation substructure up or down in the list */
int                gom_move_source_citation(Gom_direction dir,
					    struct source_citation** data,
					    struct source_citation* obj);
  
  /** \brief Add new source_description substructure to the list */
struct source_description*
              gom_add_new_source_description(struct source_description** data);
  /** \brief Remove a source_description substructure to the list */
int           gom_remove_source_description(struct source_description** data,
					    struct source_description* obj);
  /** \brief Move a source_description substructure up or down in the list */
int           gom_move_source_description(Gom_direction dir,
					  struct source_description** data,
					  struct source_description* obj);
  
  /** \brief Add new source_event substructure to the list */
struct source_event* gom_add_new_source_event(struct source_event** data);
  /** \brief Remove a source_event substructure to the list */
int                gom_remove_source_event(struct source_event** data,
					   struct source_event* obj);
  /** \brief Move a source_event substructure up or down in the list */
int                gom_move_source_event(Gom_direction dir,
					 struct source_event** data,
					 struct source_event* obj);
  
  /** \brief Add new text substructure to the list */
struct text*       gom_add_new_text(struct text** data);
  /** \brief Remove a text substructure to the list */
int                gom_remove_text(struct text** data,
				   struct text* obj);
  /** \brief Move a text substructure up or down in the list */
int                gom_move_text(Gom_direction dir,
				 struct text** data,
				 struct text* obj);
  
  /** \brief Add new user_data substructure to the list */
struct user_data*  gom_add_new_user_data(struct user_data** data);
  /** \brief Remove a user_data substructure to the list */
int                gom_remove_user_data(struct user_data** data,
					struct user_data* obj);
  /** \brief Move a user_data substructure up or down in the list */
int                gom_move_user_data(Gom_direction dir,
				      struct user_data** data,
				      struct user_data* obj);
  
  /** \brief Add new user_ref_number substructure to the list */
struct user_ref_number*
                   gom_add_new_user_ref_number(struct user_ref_number** data);
  /** \brief Remove a user_ref_number substructure to the list */
int                gom_remove_user_ref_number(struct user_ref_number** data,
					      struct user_ref_number* obj);
  /** \brief Move a user_ref_number substructure up or down in the list */
int                gom_move_user_ref_number(Gom_direction dir,
					    struct user_ref_number** data,
					    struct user_ref_number* obj);
  /** @} */
  
#ifdef __cplusplus
}
#endif

#endif /* __GEDCOM_GOM_H */
