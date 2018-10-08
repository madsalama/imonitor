#include <stdio.h>
#include <regex.h>
#include <string.h>

/*
report.c
-> used to track changes everytime a file is 'M' modified in inotify report (monitoring.c)
-> implements an in-memory comm-like function to momentarily compare original/modified file

1 - Remove all line breaks, comments (#/<!--/...) -> sort the files
2 - comm -3 -1 original modified  => remove common, supress from first file
3 - comm -3 -2 original modified  => remove common, supress from second file
4 - comm original modified        => required report below (visual comparison of changed lines)

        logger.simonAlarmLevel=debug
logger.simonAlarmLevel=info
        storeMissing = false
storeMissing = true
vfcenter.warn.threshold=80
        vfcenter.warn.threshold=8000
*/

void _cleanup_g_string(GString* string);
char* generate_report(char* str1, char* str2); 

void
_cleanup_g_string(GString *string)
{
  regex_t regex;
  gchar *read_ptr = string->str;
  GString *_string = g_string_new("");
  gint reti = regcomp(&regex, "^[#]", 0);  /* ignore comments */

/* remove white spaces from string */
gchar *write_ptr = string->str, *read_ptr = string->str;
do
{
if (*read_ptr != ' ')
*write_ptr++ = *read_ptr;
} while (*read_ptr++);

  if (reti == 0)
    {
      /* execute regex for each line of string (file) */
      while (read_ptr)
        {
          gchar *ch = strchr(read_ptr, '\n');         /* cut substring before a '\n' */
          if (ch) *ch = '\0';                         /* null-terminate substring    */

          /* ignore if ^[#] (comment) */
          if( ( reti = regexec(&regex, read_ptr, 0, NULL, 0) ) == REG_NOMATCH)
            g_string_append(_string, read_ptr);

          /* replace '\0' with any gchar
 *            * except: '#' '\n' = '-' chosen here */
          if (ch) *ch = '-';

          /* goto next line */
          read_ptr = ch ? ch+1 : NULL ;
        }

      g_string_assign(string, _string->str);
    }

  regfree(&regex);
}
