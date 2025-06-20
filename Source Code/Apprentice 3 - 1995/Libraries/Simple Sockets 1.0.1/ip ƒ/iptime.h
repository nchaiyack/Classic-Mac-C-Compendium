/*
 * time.h
 *
 * unixlike time stuff
 *
 */

#ifndef __IPTIMEH__
#define __IPTIMEH__

struct timeval {
     long  tv_sec;       /* seconds since Jan. 1, 1970 */
     long  tv_usec;      /* and microseconds */
};

struct timezone {
        int             tz_minuteswest; /* minutes west of Greenwich */
        int             tz_dsttime;     /* type of dst correction */
};

#endif	/* __IPTIMEH__ */