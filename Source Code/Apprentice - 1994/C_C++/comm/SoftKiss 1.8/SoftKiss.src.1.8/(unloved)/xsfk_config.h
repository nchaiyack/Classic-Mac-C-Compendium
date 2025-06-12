/*
 * configuration constants
 */

#pragma once

/*
 * If the connection closes or can't be open because of an error
 * retry this often
 */
#define sfk_RESTART_ON_ERROR_TIME (5*sfk_MINS)

/*
 * consider all errors transient and don't tell the user
 * till this much time has elapsed
 */
#define sfk_ERROR_ANNOUNCE_TIME (15*sfk_MINS)


/*
 * ask the server this often (default, server can set new minimum)
 */
#define sfk_POLL_TIME (5*sfk_MINS)

/*
 * Retransmit unaswered polls this often at first
 */
#define sfk_RETRANS_MIN_TIME (1*sfk_MINS)

/*
 * after this many fast RETRANS_MIN_TIME retransmits, go back to
 * using POLL_TIME.  Also queue up a timeout error to happen
 * in the usual ERROR_ANOUNCE_TIME seconds.
 */
#define sfk_NUM_FAST_RETRIES (4)

/*
 * Sleep this long when user wacks the snooze button
 */
#define sfk_SNOOZE_TIME (45*sfk_MINS)

#ifndef sfk_DEBUG
#define sfk_ABS_MIN_POLL (sfk_MINS*5)
#define sfk_ABS_MAX_POLL (60*sfk_MINS)
#else
#define sfk_ABS_MIN_POLL (30*sfk_SECS)
#define sfk_ABS_MAX_POLL (30*sfk_SECS)
#endif

/*
 * don't load sounds if it would leave less than this much space
 * in system heap
 */
#define sfk_SYS_RESERVE (16000)
