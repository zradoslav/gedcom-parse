check_exe = executable('gedcom-check', [
		'gedcom-check.c',
		gtags_dep
	],
	dependencies: [gedcom_dep, utf8tools_dep],
	include_directories: include_dir,
	install: true
)

sanitize_exe = executable('gedcom-sanitize', [
		'gedcom-sanitize.c',
		gtags_dep
	],
	dependencies: [gedcom_dep, gom_dep, utf8tools_dep],
	include_directories: include_dir,
	install: true
)
