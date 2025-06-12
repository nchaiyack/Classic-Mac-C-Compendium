
/*
 *  profile.h
 *
 *  Copyright (c) 1991 Symantec Corporation.  All rights reserved.
 *
 */

#pragma once

void InitProfile(unsigned, unsigned);
void DumpProfile(void);

long VIA_ticks(void);
void start_VIA_timer(void);
void stop_VIA_timer(void);

extern int _profile, _trace;
