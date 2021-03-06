/*
 * $Id: string_edit.c,v 1.24 1998/12/23 16:11:18 fjoe Exp $
 */

/***************************************************************************
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "merc.h"

DECLARE_DO_FUN(do_replay);

const char *string_linedel(const char *, int);
const char *string_lineadd(const char *, char *, int);
char *numlines(const char *);

/*****************************************************************************
 Name:		string_edit
 Purpose:	Clears string and puts player into editing mode.
 Called by:	none
 ****************************************************************************/
void string_edit(CHAR_DATA *ch, const char **pString)
{
	char_puts("-========- Entering EDIT Mode -=========-\n"
		  "    Type :h on a new line for help\n"
		  " Terminate with a ~ or @ on a blank line.\n"
		  "-=======================================-\n", ch);
	free_string(*pString);
	*pString = str_empty;
	ch->desc->pString = pString;
}

/*****************************************************************************
 Name:		string_append
 Purpose:	Puts player into append mode for given string.
 Called by:	(many)olc_act.c
 ****************************************************************************/
void string_append(CHAR_DATA *ch, const char **pString)
{
    char_puts("-=======- Entering APPEND Mode -========-\n", ch);
    char_puts("    Type :h on a new line for help\n", ch);
    char_puts(" Terminate with a ~ or @ on a blank line.\n", ch);
    char_puts("-=======================================-\n", ch);

    if (*pString == NULL)
	*pString = str_dup(str_empty);
    char_puts(numlines(*pString), ch);

/* numlines entrega el string con \n */
/*  if (*(*pString + strlen(*pString) - 1) != '\r')
	char_puts("\n", ch); */

    ch->desc->pString = pString;

    return;
}



/*****************************************************************************
 Name:		string_replace
 Purpose:	Substitutes one string for another.
 Called by:	string_add(string.c) (aedit_builder)olc_act.c.
 ****************************************************************************/
const char * string_replace(const char * orig, char * old, char * new)
{
    char xbuf[MAX_STRING_LENGTH];
    int i;

    xbuf[0] = '\0';
    strcpy(xbuf, orig);
    if (strstr(orig, old) != NULL)
    {
        i = strlen(orig) - strlen(strstr(orig, old));
        xbuf[i] = '\0';
        strcat(xbuf, new);
        strcat(xbuf, &orig[i+strlen(old)]);
        free_string(orig);
    }

    return str_dup(xbuf);
}



/*****************************************************************************
 Name:		string_add
 Purpose:	Interpreter for string editing.
 Called by:	game_loop_xxxx(comm.c).
 ****************************************************************************/
void string_add(CHAR_DATA *ch, const char *argument)
{
	const char *p;

    /*
     * Thanks to James Seng
     */

    if (*argument == ':')
    {
        char arg1 [MAX_INPUT_LENGTH];
        char arg2 [MAX_INPUT_LENGTH];
        char arg3 [MAX_INPUT_LENGTH];
        char tmparg3 [MAX_INPUT_LENGTH];

        argument = one_argument(argument, arg1);
        argument = first_arg(argument, arg2, FALSE);
	strcpy(tmparg3, argument);
        argument = first_arg(argument, arg3, FALSE);

        if (!str_cmp(arg1+1, "c"))
        {
            char_puts("String cleared.\n", ch);
	    free_string(*ch->desc->pString);
	    *ch->desc->pString = str_dup(str_empty);
            return;
        }

        if (!str_cmp(arg1+1, "s"))
        {
            char_printf(ch, "String so far:\n%s",
            		numlines(*ch->desc->pString));
            return;
        }

        if (!str_cmp(arg1+1, "r"))
        {
            if (arg2[0] == '\0')
            {
                char_puts(
                    "usage:  :r \"old string\" \"new string\"\n", ch);
                return;
            }

            *ch->desc->pString =
                string_replace(*ch->desc->pString, arg2, arg3);
            char_printf(ch, "'%s' replaced with '%s'.\n", arg2, arg3);
            return;
        }

        if (!str_cmp(arg1+1, "f"))
        {
            *ch->desc->pString = format_string(*ch->desc->pString);
            char_puts("String formatted.\n", ch);
            return;
        }
        
	if (!str_cmp(arg1+1, "ld"))
	{
		*ch->desc->pString = string_linedel(*ch->desc->pString, atoi(arg2));
		char_puts("Line deleted.\n", ch);
		return;
	}

	if (!str_cmp(arg1+1, "li"))
	{
		*ch->desc->pString = string_lineadd(*ch->desc->pString, tmparg3, atoi(arg2));
		char_puts("Line inserted.\n", ch);
		return;
	}

	if (!str_cmp(arg1+1, "lr"))
	{
		*ch->desc->pString = string_linedel(*ch->desc->pString, atoi(arg2));
		*ch->desc->pString = string_lineadd(*ch->desc->pString, tmparg3, atoi(arg2));
		char_puts("Line replaced.\n", ch);
		return;
	}

	if (!str_cmp(arg1+1, "x")
	||  !str_cmp(arg1+1, "q")
	||  !str_cmp(arg1+1, "wq")) {
        	ch->desc->pString = NULL;
        	return;
	}

        if (!str_cmp(arg1+1, "h"))
        {
            char_puts("Sedit help (commands on blank line):   \n", ch);
            char_puts(":r 'old' 'new'   - replace a substring \n", ch);
            char_puts("                   (requires '', \"\") \n", ch);
            char_puts(":h               - get help (this info)\n", ch);
            char_puts(":s               - show string so far  \n", ch);
            char_puts(":f               - (word wrap) string  \n", ch);
            char_puts(":c               - clear string so far \n", ch);
            char_puts(":ld <num>        - delete line #num\n", ch);
            char_puts(":li <num> <str>  - insert <str> before line #num\n", ch);
	    char_puts(":lr <num> <str>  - replace line #num with <str>\n", ch);
            char_puts("@                - end string          \n", ch);
            char_puts(":x, :q, :wq      - end string          \n", ch);
            return;
        }

        char_puts("SEdit: Invalid command.\n", ch);
        return;
    }

    if (*argument == '~' || *argument == '@')
    {
        ch->desc->pString = NULL;
	if (IS_SET(ch->comm, COMM_QUIET_EDITOR))
		do_replay(ch, str_empty);
        return;
    }

	if (*argument == '.')
		argument++;

    /*
     * Truncate strings to MAX_STRING_LENGTH.
     * --------------------------------------
     */
    if (strlen(*ch->desc->pString)+strlen(argument) >= (MAX_STRING_LENGTH - 4))
    {
        char_puts("String too long, last line skipped.\n", ch);

	/* Force character out of editing mode. */
        ch->desc->pString = NULL;
        return;
    }

	p = *ch->desc->pString;
	if (p[strlen(p)-1] != '\\') {
		*ch->desc->pString = str_add(*ch->desc->pString, argument,
					     "\n", NULL);
		free_string(p);
	}
}

/*
 * Thanks to Kalgen for the new procedure (no more bug!)
 * Original wordwrap() written by Surreality.
 */
/*****************************************************************************
 Name:		format_string
 Purpose:	Special string formating and word-wrapping.
 Called by:	string_add(string.c) (many)olc_act.c
 ****************************************************************************/
const char *format_string(const char *oldstring /*, bool fSpace */)
{
  char xbuf[MAX_STRING_LENGTH];
  char xbuf2[MAX_STRING_LENGTH];
  const char *rdesc;
  int i=0;
  bool cap=TRUE;
  
	if (IS_NULLSTR(oldstring))
		return oldstring;

  xbuf[0]=xbuf2[0]=0;
  
  i=0;
  
  for (rdesc = oldstring; *rdesc; rdesc++)
  {
    if (*rdesc=='\n')
    {
      if (xbuf[i-1] != ' ')
      {
        xbuf[i]=' ';
        i++;
      }
    }
    else if (*rdesc=='\r') ;
    else if (*rdesc==' ')
    {
      if (xbuf[i-1] != ' ')
      {
        xbuf[i]=' ';
        i++;
      }
    }
    else if (*rdesc==')')
    {
      if (xbuf[i-1]==' ' && xbuf[i-2]==' ' && 
          (xbuf[i-3]=='.' || xbuf[i-3]=='?' || xbuf[i-3]=='!'))
      {
        xbuf[i-2]=*rdesc;
        xbuf[i-1]=' ';
        xbuf[i]=' ';
        i++;
      }
      else
      {
        xbuf[i]=*rdesc;
        i++;
      }
    }
    else if (*rdesc=='.' || *rdesc=='?' || *rdesc=='!') {
      if (xbuf[i-1]==' ' && xbuf[i-2]==' ' && 
          (xbuf[i-3]=='.' || xbuf[i-3]=='?' || xbuf[i-3]=='!')) {
        xbuf[i-2]=*rdesc;
        if (*(rdesc+1) != '\"')
        {
          xbuf[i-1]=' ';
          xbuf[i]=' ';
          i++;
        }
        else
        {
          xbuf[i-1]='\"';
          xbuf[i]=' ';
          xbuf[i+1]=' ';
          i+=2;
          rdesc++;
        }
      }
      else
      {
        xbuf[i]=*rdesc;
        if (*(rdesc+1) != '\"')
        {
          xbuf[i+1]=' ';
          xbuf[i+2]=' ';
          i += 3;
        }
        else
        {
          xbuf[i+1]='\"';
          xbuf[i+2]=' ';
          xbuf[i+3]=' ';
          i += 4;
          rdesc++;
        }
      }
      cap = TRUE;
    }
    else
    {
      xbuf[i]=*rdesc;
      if (cap)
        {
          cap = FALSE;
          xbuf[i] = UPPER(xbuf[i]);
        }
      i++;
    }
  }
  xbuf[i]=0;
  strcpy(xbuf2,xbuf);
  
  rdesc=xbuf2;
  
  xbuf[0]=0;
  
  for (; ;)
  {
    for (i=0; i<70; i++)
    {
      if (!*(rdesc+i)) break;
    }
    if (i<70)
    {
      break;
    }
    for (i=(xbuf[0]?76:73) ; i ; i--)
    {
      if (*(rdesc+i)==' ') break;
    }
    if (i)
    {
      strnzncat(xbuf,rdesc, sizeof(xbuf), i);
      strcat(xbuf,"\n");
      rdesc += i+1;
      while (*rdesc == ' ') rdesc++;
    }
    else
    {
      bug ("No spaces", 0);
      strnzncat(xbuf,rdesc, sizeof(xbuf), 75);
      strcat(xbuf,"-\n");
      rdesc += 76;
    }
  }
  while (*(rdesc+i) && (*(rdesc+i)==' '||
                        *(rdesc+i)=='\n'||
                        *(rdesc+i)=='\r'))
    i--;
  strnzncat(xbuf,rdesc, sizeof(xbuf), i+1);
  if (xbuf[strlen(xbuf)-2] != '\n')
    strcat(xbuf,"\n");

  free_string(oldstring);
  return(str_dup(xbuf));
}

const char *string_linedel(const char *string, int line)
{
	const char *strtmp = string;
	char buf[MAX_STRING_LENGTH];
	int cnt = 1, tmp = 0;

	buf[0] = '\0';

	for (; *strtmp != '\0'; strtmp++) {
		if (cnt != line)
			buf[tmp++] = *strtmp;

		if (*strtmp == '\n')
			cnt++;
	}

	buf[tmp] = '\0';

	free_string(string);
	return str_dup(buf);
}

const char *string_lineadd(const char *string, char *newstr, int line)
{
	const char *strtmp = string;
	int cnt = 1, tmp = 0;
	bool done = FALSE;
	char buf[MAX_STRING_LENGTH];

	buf[0] = '\0';

	if (newstr[0] == '.')
		newstr++;

	for (; *strtmp != '\0' || (!done && cnt == line); strtmp++) {
		if (cnt == line && !done) {
			strcat(buf, newstr);
			strcat(buf, "\n");
			tmp += strlen(newstr) + 1;
			cnt++;
			done = TRUE;
		}

		buf[tmp++] = *strtmp;

		if (done && *strtmp == '\0')
			break;

		if (*strtmp == '\n')
			cnt++;

		buf[tmp] = '\0';
	}

	free_string(string);
	return str_dup(buf);
}

/* buf queda con la linea sin \n */
const char *getline(const char *str, char *buf)
{
	int tmp = 0;
	bool found = FALSE;

	while (*str) {
		if (*str == '\n') {
			found = TRUE;
			break;
		}

		buf[tmp++] = *(str++);
	}

	if (found) {
		if (*(str + 1) == '\r')
			str += 2;
		else
			str += 1;
	} /* para que quedemos en el inicio de la prox linea */

	buf[tmp] = '\0';

	return str;
}

char *numlines(const char *string)
{
	int cnt = 1;
	static char buf[MAX_STRING_LENGTH*2];
	char buf2[MAX_STRING_LENGTH], tmpb[MAX_STRING_LENGTH];

	buf[0] = '\0';

	while (*string) {
		string = getline(string, tmpb);
		sprintf(buf2, "%2d. %s\n", cnt++, tmpb);
		strcat(buf, buf2);
	}

	return buf;
}
