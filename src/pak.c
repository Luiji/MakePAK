/*******************************************************************************
 * MakePAK: Creates Quake .PAK files.
 * Copyright (C) 2011 Entertaining Software, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <xalloc.h>
#include <getopt.h>
#include <isdir.h>
#include <dirent.h>
#include <error.h>
#include <byteswap.h>

/* truncate gettext to _ if gettext is supported, otherwise set _ to nothing */
#if ENABLE_NLS
# include <gettext.h>
# define _(str) gettext (str)
#else
# define _(str) str
#endif

/* .PAK is little-endian, this function ensures little-endian encoding */
#if ENCODE_BIG_ENDIAN
# define bswap(x) (bswap_32 (x))
#else
# define bswap(x) (x)
#endif

/* size of a filename (excluding NULL bit) */
#define FILENAME_SIZE 55
/* size of a filename (including NULL bit) */
#define FILENAME_TRUE_SIZE 56
/* size of a .PAK header */
#define HEADER_SIZE 12
/* size of a directory entry */
#define DIRECTORY_ENTRY_SIZE 64

/* input file data */
struct input_file
{
  /* file pointer */
  FILE *file;
  /* file name */
  char name[FILENAME_TRUE_SIZE];
  /* file size */
  uint32_t length;
  /* where file starts */
  uint32_t offset;
};

/* argv[0]; name of the program given at the command-line */
const char *program_name;
/* whether or not to recursively add directories */
static int recursive_flag = 0;
/* whether or not to print each file as it is added */
static int verbose_flag = 0;
/* name of the output file */
static char *output_filename = NULL;
/* pointer to the output file */
static FILE *output_file = NULL;
/* number of input files */
static uint32_t input_file_count = 0;
/* list of input file names */
static struct input_file *input_files = NULL;
/* size of the file before the directory comes in to play */
static uint32_t pre_directory_size = HEADER_SIZE;
/* size that the directory will take (each entry is 64 bytes) */
static uint32_t directory_size = 0;

/* long options for getopt_long */
static const struct option longopts[] =
{
  { "help", no_argument, NULL, 'h' },
  { "version", no_argument, NULL, 'v' },
  { "output", required_argument, NULL, 'o' },
  { "recursive", no_argument, &recursive_flag, 1 },
  { "verbose", no_argument, &verbose_flag, 1 },
  { NULL, 0, NULL, 0 }
};

/* print error message and exit */
static void
die (char *format, ...)
{
  char *message;
  va_list args;
  va_start (args, format);
  vasprintf (&message, format, args);
  va_end (args);
  error (EXIT_FAILURE, 0, "%s", message);
}

/* print error message with file name and exit */
static void
dief (char *filename, char *format, ...)
{
  char *message;
  va_list args;
  va_start (args, format);
  vasprintf (&message, format, args);
  va_end (args);
  error (EXIT_FAILURE, 0, "%s: %s", filename, message);
}

/* add input file */
static void
add_file (char *filename)
{
  if (verbose_flag)
    fprintf (stderr, "Adding file %s...\n", filename);

  /* verify directory entry size */
  directory_size += DIRECTORY_ENTRY_SIZE;
  if (directory_size > UINT32_MAX)
    dief (optarg, _("too many entries, directory exceeded 4 GiB max size"));

  /* add file entry */
  input_file_count++;
  input_files =
    xrealloc (input_files, input_file_count * sizeof (struct input_file) *
			   input_file_count);

  /* open file and set */
  FILE *file = fopen (filename, "rb");
  if (file == NULL)
    dief (filename, _("cannot open file"));
  input_files[input_file_count - 1].file = file;

  /* get filename and set */
  strcpy (input_files[input_file_count - 1].name, filename);

  /* get file size and set*/
  if (fseek (file, 0, SEEK_END))
    dief (filename, _("cannot get file size"));

  size_t filesize = ftell (file);

  if (filesize > UINT32_MAX)
    dief (filename, _("file too big (limit 4 GiB)"));

  input_files[input_file_count - 1].length = filesize;

  /* get file offset and set */
  input_files[input_file_count - 1].offset = pre_directory_size;

  /* verify file size */
  pre_directory_size += filesize;
  if (pre_directory_size > UINT32_MAX)
    dief (optarg, _("exceeded maximum of 4 GiB addressable space"));
}

/* add directory */
static void
add_directory (char *filename)
{
  if (verbose_flag)
    fprintf (stderr, "Entering directory %s...\n", filename);

  size_t filename_size = strlen (filename);
  char entryname[FILENAME_TRUE_SIZE];
  struct dirent *entry;
  DIR *dir;

  dir = opendir (filename);
  if (dir == NULL)
    dief (filename, "cannot open directory");

  while (entry = readdir (dir))
    {
      /* skip . and .. special directories */
      if (strcmp (entry->d_name, "..") == 0 || strcmp (entry->d_name, ".") == 0)
	continue;

      /* check filename size; +1 is for the path seperator */
      if (strlen (entry->d_name) + filename_size + 1 > FILENAME_SIZE)
	dief (entry->d_name, "filename too long (in directory %s)", filename);

      /* get full entry name */
      strcpy (entryname, filename);
      strcat (entryname, "/");
      strcat (entryname, entry->d_name);

      /* pass along */
      if (isdir (entryname))
	add_directory (entryname);
      else
	add_file (entryname);
    }

  closedir (dir);

  if (verbose_flag)
    fprintf (stderr, "Leaving directory %s...\n", filename);
}

/* add input entry */
static void
add_input (char *filename)
{
  /* verify length of filename */
  if (strlen (filename) > FILENAME_SIZE)
    dief (filename, "file name too long");

  /* pass along to sub-function */
  if (isdir (filename))
    {
      /* make sure --recursive is specified; called here since add_directory()
	 would call it multiple times */
      if (!recursive_flag)
	fprintf (stderr, "%s: %s: is directory; skipping (use -r to recurse)\n",
		 program_name, filename);
      else
	add_directory (filename);
    }
  else
    add_file (filename);
}

/* print a 32-bit unsigned integer to output */
#define print_uint32(i) \
{ \
  uint32_t buffer = bswap (i); \
  fwrite (&buffer, 4 /* sizeof (uint32_t) */, 1, output_file); \
}

/* print the --help message */
static void
print_help (void)
{
  printf (_("Usage: %s [OPTION]... [FILE]...\n"), program_name);
  printf ("\n");
  printf (_("Create Quake .PAK files.\n"));
  printf ("\n");
  printf (_("  -h, --help       display this help and exit\n"));
  printf (_("  -v, --version    display version information and exit\n"));
  printf (_("  -o, --output     set output file\n"));
  printf (_("  -r, --recursive  add directories recursively\n"));
  printf (_("  -V, --verbose    list each file as it is added\n"));
  printf ("\n");
  printf (_("Report %s bugs to: %s\n"), PACKAGE_NAME, PACKAGE_BUGREPORT);
  printf (_("%s home page: <%s>\n"), PACKAGE_NAME, PACKAGE_URL);
  exit (EXIT_SUCCESS);
}

/* print the --version message */
static void
print_version (void)
{
  printf ("%s\n", PACKAGE_STRING);
  /* copyright not translated because translation is not needed */
  printf ("Copyright (C) 2011 Entertaining Software, Inc.\n");
  printf (_("License GPLv3+: GNU GPL version 3 or later "
	  "<http://gnu.org/licenses/gpl.html>\n"));
  printf (_("This is free software: you are free to change and redistribute "
	    "it.\n"));
  printf (_("There is NO WARRANTY, to the extent permitted by law.\n"));
  exit (EXIT_SUCCESS);
}

/* program entry point */
int
main (int argc, char **argv)
{
  program_name = argv[0];

  /* set locale */
  setlocale (LC_ALL, "");

#if ENABLE_NLS
  /* initialize gettext */
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);
#endif

  /* parse command-line options */
  {
    int optc;
    while ((optc = getopt_long (argc, argv, "hvo:rV", longopts, NULL)) != -1)
      switch (optc)
	{
	case 0:
	  break;
	case 'h':
	  print_help ();
	  break;
	case 'v':
	  print_version ();
	  break;
	case 'o':
	  output_filename = optarg;
	  break;
	case 'r':
	  recursive_flag = 1;
	  break;
	case 'V':
	  verbose_flag = 1;
	  break;
	default:
	  exit (EXIT_FAILURE);
	}
  }

  /* parse rest of command-line arguments (input files) */
  while (optind < argc)
    add_input (argv[optind++]);

  /* check for input files */
  if (input_file_count == 0)
    die ("no input files");

  /* open output file */
  if (output_filename == NULL)
    {
      output_filename = "stdout";
      output_file = stdout;
    }
  else if ((output_file = fopen (output_filename, "wb")) == NULL)
    dief (output_filename, "cannot create file");

  /* output file header */
  fprintf (output_file, "PACK");
  print_uint32 (pre_directory_size);
  print_uint32 (directory_size);

  /* output all input files */
  {
    register uint32_t i;
    register int c;
    for (i = 0; i < input_file_count; i++)
      {
	if (verbose_flag)
	  fprintf (stderr, "Archiving %s...\n", input_files[i].name);
	FILE *file = input_files[i].file;
	fseek (file, 0, SEEK_SET);
	while ((c = fgetc (file)) != EOF)
	  fputc (c, output_file);
	if (fclose (file))
	  dief (input_files[i].name, "cannot close stream");
      }
  }

  /* burn directory */
  {
    if (verbose_flag)
      fprintf (stderr, "Burning directory...\n");
    register uint32_t i;
    for (i = 0; i < input_file_count; i++)
      {
	fwrite (input_files[i].name, FILENAME_TRUE_SIZE, 1, output_file);
	print_uint32 (input_files[i].offset);
	print_uint32 (input_files[i].length);
      }
  }

  /* close output file (even if its stdout) */
  if (fclose (output_file))
    dief (output_filename, "cannot close stream");

  /* exit with success status */
  exit (EXIT_SUCCESS);
}

/* vim:set ts=8 sts=2 sw=2 noet: */
