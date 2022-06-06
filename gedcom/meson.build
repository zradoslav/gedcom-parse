# flex & bison
flex = find_program('flex')
bison = find_program('bison')

lexer_gen = generator(flex,
    output: '@BASENAME@.yy.c',
    arguments: ['-o', '@OUTPUT@', '-P@BASENAME@_', '@INPUT@']
)
parser_gen = generator(bison,
    output: ['@BASENAME@.tabgen.c', '@BASENAME@.tabgen.h'],
    arguments: [
        '@INPUT@',
        '--defines=@OUTPUT1@', '--name-prefix=@BASENAME@_',
        '--output=@OUTPUT0@'
    ]
)

# GEDCOM parser grammar
lexer_1byte = lexer_gen.process('gedcom_1byte.lex')
lexer_hilo = lexer_gen.process('gedcom_hilo.lex')
lexer_lohi = lexer_gen.process('gedcom_lohi.lex')

parser = custom_target('parser',
    input: 'gedcom.y',
    output: ['gedcom.tabgen.c', 'gedcom.tabgen.h'],
    command: [bison,
        '@INPUT@',
        '--defines=@OUTPUT1@', '--name-prefix=gedcom_',
        '--output=@OUTPUT0@'
    ]
)

# GEDCOM date parser
date_lexer = lexer_gen.process('gedcom_date.lex')
date_parser = parser_gen.process('gedcom_date.y')

# extract tag names from bison output
perl = find_program('perl')
tagnames_dep = custom_target('tag_names.h',
    input: parser[1],
    output: 'tag_names.h',
    command: [perl,
        meson.current_source_dir() / 'tag_names.pl', '@INPUT@', '@OUTPUT@'
    ]
)
# extract tags from bison output
gtags_dep = custom_target('gedcom-tags.h',
    input: parser[1],
    output: 'gedcom-tags.h',
    command: [perl,
        meson.current_source_dir() / 'gedcom_tags.pl', '@INPUT@', '@OUTPUT@'
    ],
    install: true,
    install_dir: get_option('includedir')
)

src = files(
    'calendar/dow.c',
    'calendar/french.c',
    'calendar/gregor.c',
    'calendar/jewish.c',
    'calendar/julian.c',

    'age.c',
    'buffer.c',
    'compat.c',
    'date.c',
    'encoding.c',
    'encoding_state.c',
    'hash.c',
    'interface.c',
    'message.c',
    'multilex.c',
    'write.c',
    'xref.c'
)

gedcom_lib = shared_library('gedcom', [
        src,
        lexer_1byte, lexer_hilo, lexer_lohi,
        parser[0],
        date_lexer,
        date_parser,
        tagnames_dep,
        gtags_dep
    ],
    dependencies: utf8tools_dep,
    include_directories: [include_dir, 'calendar'],
    install: true,
    version: meson.project_version()
)
gedcom_dep = declare_dependency(
    link_with: gedcom_lib
)