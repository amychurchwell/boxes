/*
 *  File:             tools.c
 *  Project Main:     boxes.c
 *  Date created:     June 20, 1999 (Sunday, 16:51h)
 *  Author:           Copyright (C) 1999 Thomas Jensen
 *                    tsjensen@stud.informatik.uni-erlangen.de
 *  Version:          $Id: tools.c,v 1.1 1999/06/23 11:19:30 tsjensen Exp tsjensen $
 *  Language:         ANSI C
 *  World Wide Web:   http://home.pages.de/~jensen/boxes/
 *  Purpose:          Provide tool functions for error reporting and some
 *                    string handling
 *
 *  Remarks: o This program is free software; you can redistribute it and/or
 *             modify it under the terms of the GNU General Public License as
 *             published by the Free Software Foundation; either version 2 of
 *             the License, or (at your option) any later version.
 *           o This program is distributed in the hope that it will be useful,
 *             but WITHOUT ANY WARRANTY; without even the implied warranty of
 *             MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *             GNU General Public License for more details.
 *           o You should have received a copy of the GNU General Public
 *             License along with this program; if not, write to the Free
 *             Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *             MA 02111-1307  USA
 *
 *  Revision History:
 *
 *    $Log: tools.c,v $
 *    Revision 1.1  1999/06/23 11:19:30  tsjensen
 *    Initial revision
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */


#include "config.h"
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "shape.h"
#include "boxes.h"
#include "tools.h"


static const char rcsid_tools_c[] =
    "$Id: tools.c,v 1.1 1999/06/23 11:19:30 tsjensen Exp tsjensen $";



int yyerror (const char *fmt, ...)
/*
 *  Print configuration file parser errors.
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    va_list ap;
    char buf[1024];

    va_start (ap, fmt);
    snprintf (buf, 1024-1, "%s: %s: line %d: ", PROJECT,
            yyfilename? yyfilename: "(null)", yylineno);
    vsnprintf (buf+strlen(buf), 1024-strlen(buf)-1, fmt, ap);
    strcat (buf, "\n");
    fprintf (stderr, buf);
    va_end (ap);

    return 0;
}



void regerror (char *msg)
/*
 *  Print regular expression andling error messages
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    fprintf (stderr, "%s: %s: line %d: %s\n",
            PROJECT, yyfilename? yyfilename: "(null)",
            opt.design->current_rule? opt.design->current_rule->line: 0,
            msg);
    errno = EINVAL;
}



int strisyes (const char *s)
/*
 *  Determine if the string s has a contents indicating "yes".
 *
 *     s   string to examine
 *
 *  RETURNS:  == 0  string does NOT indicate yes (including errors)
 *            != 0  string indicates yes
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    if (s == NULL)
        return 0;

    if (!strncasecmp ("on", optarg, 3))
        return 1;
    else if (!strncasecmp ("yes", optarg, 4))
        return 1;
    else if (!strncasecmp ("true", optarg, 5))
        return 1;
    else if (!strncmp ("1", optarg, 2))
        return 1;
    else if (!strncasecmp ("t", optarg, 2))
        return 1;
    else
        return 0;
}



int strisno (const char *s)
/*
 *  Determine if the string s has a contents indicating "no".
 *
 *     s   string to examine
 *
 *  RETURNS:  == 0  string does NOT indicate no (including errors)
 *            != 0  string indicates no
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    if (s == NULL)
        return 0;

    if (!strncasecmp ("off", optarg, 4))
        return 1;
    else if (!strncasecmp ("no", optarg, 3))
        return 1;
    else if (!strncasecmp ("false", optarg, 6))
        return 1;
    else if (!strncmp ("0", optarg, 2))
        return 1;
    else if (!strncasecmp ("f", optarg, 2))
        return 1;
    else
        return 0;
}



int empty_line (const line_t *line)
/*
 *  Return true if line is empty.
 *
 *  Empty lines either consist entirely of whitespace or don't exist.
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    char *p;
    size_t j;

    if (!line)
        return 1;
    if (line->text == NULL || line->len <= 0)
        return 1;

    for (p=line->text, j=0; *p && j<line->len; ++j, ++p) {
        if (*p != ' ' && *p != '\t' && *p != '\r' && *p != '\n')
            return 0;
    }
    return 1;
}



size_t expand_tabs_into (const char *input_buffer, const int in_len,
      const int tabstop, char **text)
/*
 *  Expand tab chars in input_buffer and store result in text.
 *
 *  input_buffer   Line of text with tab chars
 *  in_len         length of the string in input_buffer
 *  tabstop        tab stop distance
 *  text           address of the pointer that will take the result
 *
 *  Memory will be allocated for the result.
 *  Should only be called for lines of length > 0;
 *
 *  RETURNS:  Success: Length of the result line in characters (> 0)
 *            Error:   0       (e.g. out of memory)
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
   static char temp [LINE_MAX*MAX_TABSTOP+1];  /* work string */
   int ii;                               /* position in input string */
   int io;                               /* position in work string */
   int jp;                               /* tab expansion jump point */

   *text = NULL;

   for (ii=0, io=0; ii<in_len && io<(LINE_MAX*tabstop-1); ++ii) {
      if (input_buffer[ii] == '\t') {
         for (jp=io+tabstop-(io%tabstop); io<jp; ++io)
            temp[io] = ' ';
      }
      else {
         temp[io] = input_buffer[ii];
         ++io;
      }
   }
   temp[io] = '\0';

   *text = (char *) strdup (temp);
   if (*text == NULL) return 0;

   return io;
}



void btrim (char *text, size_t *len)
/*
 *  Remove trailing whitespace from line.
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    long idx = (long) (*len) - 1;

    while (idx >= 0 && (text[idx] == '\n' || text[idx] == '\r'
                     || text[idx] == '\t' || text[idx] == ' '))
    {
        text[idx--] = '\0';
    }

    *len = idx + 1;
}



char *my_strnrstr (const char *s1, const char *s2, const size_t s2_len, int skip)
/*
 *  Return pointer to last occurrence of string s2 in string s1.
 *
 *      s1       string to search
 *      s2       string to search for in s1
 *      s2_len   length in characters of s2
 *      skip     number of finds to ignore before returning anything
 *
 *  RETURNS: pointer to last occurrence of string s2 in string s1
 *           NULL if not found or error
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    char *p;
    int comp;

    if (!s2 || *s2 == '\0')
        return (char *) s1;
    if (!s1 || *s1 == '\0')
        return NULL;
    if (skip < 0)
        skip = 0;

    p = strrchr (s1, s2[0]);
    if (!p)
        return NULL;

    while (p >= s1) {
        comp = strncmp (p, s2, s2_len);
        if (comp == 0) {
            if (skip--)
                --p;
            else
                return p;
        }
        else {
            --p;
        }
    }

    return NULL;
}



/*EOF*/                                                  /* vim: set sw=4: */