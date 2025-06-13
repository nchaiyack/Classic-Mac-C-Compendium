// Header.h
// Copyright ©1994 David Kreindler. All rights reserved.

// universal #defines and #pragmas

// revision history
//	940319 DK: created

#pragma once
#pragma extended_errorcheck on
#pragma profile off

#define OLDROUTINENAMES 0

#undef DEBUG

#ifdef DEBUG
#	undef NDEBUG
#	define OUTLINE_INLINES
#else
#	define NDEBUG
#	undef OUTLINE_INLINES
#endif
