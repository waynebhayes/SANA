#include "utils.h"

//#ifndef PRINTFERR
//void printferr(const char *fmt, ...) {
//    char s[1024];
//    va_list argptr;
//    va_start(argptr, fmt);
//    vsnprintf(s, 1024, fmt, argptr);
//    fprintf(stderr,"In function %s. Tentative Error: %s.", s, strerror(errno));
//    va_end(argptr);
//}
//#endif

#define read_line_n 1024
char *read_line(FILE* fid) {
    if (feof(fid)) return NULL;
    char buf[read_line_n];
    char *line;
    line = (char*)malloc(read_line_n*sizeof(char));
    line[0] = '\0';
    int j=0;    
    while(fgets(buf,read_line_n,fid)!=NULL) {
        strcpy(line+(read_line_n-1)*j,buf);
        if(buf[strlen(buf)-1]=='\n') break;
        j++;
        line = (char*)realloc(line,(j+1)*read_line_n*sizeof(char));
    }
    if (strlen(trimwhitespace(line))==0) { free(line); return NULL; }
    return line;
}

// Note: This function returns a pointer to a substring of the original string.
// If the given string was allocated dynamically, the caller must not overwrite
// that pointer with the returned value, since the original pointer must be
// deallocated using the same allocator with which it was allocated.  The return
// value must NOT be deallocated using free() etc.
char *trimwhitespace(char *str)
{
  char *end;

  // Trim leading space
  while(isspace(*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace(*end)) end--;

  // Write new null terminator
  *(end+1) = 0;

  return str;
}

#if defined(_WIN32) && !defined(__CYGWIN__)
int ISWIN32 = 1;
#else
int ISWIN32 = 0;
#endif

int file_exists(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp) { fclose(fp); return(1); }
    else    { return(0); }
}    

int is_dir(const char *fname) {
struct stat s;
int err = stat(fname, &s);
if(-1 == err) {
    if(ENOENT == errno) {
        return -1; /* does not exist */
    } else {
        return -1;
    }
} else {
    if(S_ISDIR(s.st_mode)) {
        return 1; /* it's a dir */
    } else {
        return 0; /* exists but is no dir */
    }
 }
 return -1;
}

