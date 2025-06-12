//	Copyright 1993 Ralph Gonzalez

/*
*   FILE:    class.c
*   AUTHOR:  R.G.
*   CREATED: January 20, 1992
*   
*   Define methods for Generic_Class
*/

# include "class.h"

/******************************************************************
*   Generic constructor.  The constructor of each derived class
*   should first check whether the parent class' constructor
*   succeeded (using is_initialized()).  If so, then the constructor
*   may attempt to allocate instance variables, setting "initalized"
*   to FALSE if unable to do so.
******************************************************************/
Generic_Class::Generic_Class(void)
{
    initialized = TRUE;
}

/******************************************************************
*   is_initialized() method returns value of "initialized" instance
*   variable.  The instance variable should be set in a class'
*   constructor.  No need to override.
******************************************************************/
boolean Generic_Class::is_initialized(void)
{
    return initialized;
}

