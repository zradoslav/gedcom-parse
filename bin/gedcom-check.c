/* Check program using the Gedcom library.
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

#include "config.h"
#include "gedcom.h"
#include "utf8tools.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <errno.h>
#include <string.h>
#include <locale.h>

#include <getopt.h>
#include <unistd.h>

#ifdef ENABLE_NLS
#include <libintl.h>

#define _(string) dgettext(PACKAGE, string)
#define N_(string) (string)
#endif

#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif

static void usage()
{
	printf("GEDCOM file validator\n");
	printf("\n");
	printf("usage:\tgedcom-check [options] file\n");
	printf("options:\n");
	printf("\t-v, --verbose=LEVEL  show messages from library (1) and parser (2)\n");
	printf("\t--compat             allow non-standard extensions\n");
//	printf("\t--strict             only standard, strict mode (default)\n");
	printf("\t-h, --help           print this message and exit\n");
	printf("\t-V, --version        print version and exit\n");

	exit(EXIT_SUCCESS);
}

static void version()
{
	printf("%s v%s\n", "gedcom-parse", gedcom_version());

	exit(EXIT_SUCCESS);
}

#ifdef ENABLE_NLS
static void localize()
{
	static bool init = false;

	if(!init)
	{
		setlocale(LC_ALL, "");
		bindtextdomain(PACKAGE, LOCALEDIR);
		textdomain(PACKAGE);
		init = true;
	}
}
#endif

void default_cb(Gedcom_elt elt UNUSED, Gedcom_ctxt ctxt UNUSED,
				int level UNUSED, char* tag UNUSED,
				char* raw_value UNUSED, int tag_value UNUSED)
{
	/* do nothing */
}

void gedcom_message_handler(Gedcom_msg_type type UNUSED, char* msg)
{
	const char* converted = NULL;

	int  conv_fails = 0;
	converted = convert_utf8_to_locale(msg, &conv_fails);
	printf("%s\n", converted);
}

int main(int argc, char* argv[])
{
#ifdef ENABLE_NLS
	localize();
#endif
	int retcode = EXIT_SUCCESS;

	static int compat = 0;
	int verbose = 0;
	const char* filepath = NULL;

	static struct option long_options[] = {
		{ "compat",  no_argument,       NULL,   'c' },
//		{ "strict",  no_argument,       &compat, 0  },
		{ "verbose", required_argument, NULL,   'v' },
		{ "help",    no_argument,       NULL,   'h' },
		{ "version", no_argument,      	NULL,   'V' },
		{ 0, 0, 0, 0}
	};

	while(true)
	{
		int option_index = 0;
		int c = getopt_long(argc, argv, "v:chV",
							long_options, &option_index);
		if(c == -1)
			break;

		switch(c)
		{
		case 'v':
			if(optarg)
				verbose = atoi(optarg);
			break;
		case 'c':
			compat = 1;
			break;
		case 'V':
			version();
			break;
		case 'h':
			usage();
			break;
		case '?':
		default:
			retcode = EINVAL;
			goto on_exit;
		}
	}
	if(optind == argc - 1)
		filepath = argv[optind++];
	else if(optind == argc)
	{
		fprintf(stderr, _("No file name given\n"));
		exit(EINVAL);
	}
	else
	{
		errno = E2BIG;
		fprintf(stderr, "%s\n", strerror(errno));
		retcode = errno;
		goto on_exit;
	}

	if(access(filepath, F_OK))
	{
		fprintf(stderr, "%s\n", strerror(errno));
		retcode = errno;
		goto on_exit;
	}

	gedcom_init();
	gedcom_set_debug_level(verbose, NULL);
	gedcom_set_compat_handling(compat);
	gedcom_set_compat_options(COMPAT_ALLOW_OUT_OF_CONTEXT);
	gedcom_set_error_handling(DEFER_FAIL);
	gedcom_set_message_handler(gedcom_message_handler);
	gedcom_set_default_callback(default_cb);

	retcode = gedcom_parse_file(filepath);
	if(retcode)
	{
		printf(_("Parse failed\n"));
		if(!compat)
			printf("Try to enable compatibility mode\n");
	}

on_exit:
	return retcode;
}
