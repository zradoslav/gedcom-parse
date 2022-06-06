unistring_dep = cc.find_library('unistring')

# simple utf8 tools
utf8tools_lib = shared_library('utf8tools', [
	    'utf8-convert.c',
	    'utf8-locale.c',
	    'utf8.c'
	],
	c_args: '-DICONV_CONST=', # todo: handle iconv / ICONV_CONST
	dependencies: unistring_dep,
	include_directories: [include_dir, '.', 'libcharset'],
	version: '0.20.0',
	install: true
)
utf8tools_dep = declare_dependency(
    link_with: utf8tools_lib,
    include_directories: include_directories('.')
)
install_headers('utf8tools.h')