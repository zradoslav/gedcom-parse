project('gedcom-parse',
    'c',
    version: '0.90.0',
    license: 'GPLv2+',
    meson_version: '>= 0.62',
    default_options: [
        'prefix=/usr',
        'c_std=gnu99',
        'warning_level=3'
    ]
)

cc = meson.get_compiler('c')

PREFIX = get_option('prefix')
DATAROOTDIR = join_paths(PREFIX, get_option('datadir'))
LOCALEDIR = join_paths(PREFIX, get_option('localedir'))
BINDIR = join_paths(PREFIX, get_option('bindir'))
MANDIR = join_paths(PREFIX, get_option('mandir'))
PKGDATADIR = join_paths(PREFIX, DATAROOTDIR, meson.project_name())

conf = configuration_data()
conf.set_quoted('PACKAGE',  meson.project_name())
conf.set_quoted('PACKAGE_VERSION', meson.project_version())
version_parts = meson.project_version().split('.')
conf.set('PACKAGE_VERSION_MAJOR', version_parts[0])
conf.set('PACKAGE_VERSION_MINOR', version_parts[1])
conf.set('PACKAGE_VERSION_PATCH', version_parts[2])
conf.set_quoted('LOCALEDIR', LOCALEDIR)
conf.set_quoted('PKGDATADIR', PKGDATADIR)
conf.set('_XOPEN_SOURCE', 700)

if host_machine.system() == 'windows'
    conf.set_quoted('SYS_NEWLINE', '\\r\\n')
else
    conf.set_quoted('SYS_NEWLINE', '\\n')
endif

foreach symbol: ['vsprintf', 'vsnprintf']
    found = cc.has_function(symbol, prefix: '#include <stdio.h>')
    conf.set('HAVE_' + symbol.to_upper(), found.to_int())
endforeach

intl_dep = dependency('', required: false)
if get_option('i18n')
    if not cc.has_function('ngettext')
        libintl = cc.find_library('intl', required : false)
        if not libintl.found()
            error('ngettext not found but NLS support requested')
        endif
    endif
    conf.set('ENABLE_NLS', 1)
endif

config_h = configure_file(
    output : 'config.h',
    configuration : conf
)

include_dir = include_directories('include', '.')

subdir('utf8')
subdir('gedcom')
subdir('gom')
subdir('bin')
# subdir('po')
subdir('t')

install_headers(
    'include/gedcom.h',
    'include/gom.h'
)
