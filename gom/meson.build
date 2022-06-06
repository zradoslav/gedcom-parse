src = files(
    'address.c',
    'association.c',
    'change_date.c',
    'event.c',
    'family.c',
    'family_link.c',
    'gom.c',
    'gom_internal.c',
    'gom_modify.c',
    'header.c',
    'individual.c',
    'lds_event.c',
    'multimedia.c',
    'multimedia_link.c',
    'note.c',
    'note_sub.c',
    'personal_name.c',
    'place.c',
    'repository.c',
    'source.c',
    'source_citation.c',
    'source_description.c',
    'source_event.c',
    'submission.c',
    'submitter.c',
    'user_rec.c',
    'user_ref.c'
)

gom_lib = shared_library('gom',
    sources: [
        src,
        gtags_dep
    ],
    dependencies: [gedcom_dep, utf8tools_dep],
    include_directories: include_dir,
    install: true,
    version: meson.project_version()
)
gom_dep = declare_dependency(
    link_with: gom_lib
)