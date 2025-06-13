typedef long time_t;
typedef long clock_t;

struct tm {
  short tm_sec;
  short tm_min;
  short tm_hour;
  short tm_mday;
  short tm_mon;
  short tm_year;
  short tm_wday;
  short tm_yday;
  short tm_isdst; /* Not available on Macintosh */
  short tm_hsec;  /* Not available on Macintosh */
};

struct tm *gmtime(), *localtime();
char *asctime(), *ctime();
time_t time();
