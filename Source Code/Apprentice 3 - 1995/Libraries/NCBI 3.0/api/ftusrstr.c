/***********************************
* ftusrstr.c
*
*  All processing of User Error String handled in this file
*
* $Log: ftusrstr.c,v $
 * Revision 1.6  1995/05/15  21:46:05  ostell
 * added Log line
 *
*
*
********************************/
#include <ncbi.h>
#include "ftusrstr.h"

static Uint1 Last_magic_locus_cookie=0;
static Uint1 Last_magic_accession_cookie=0;
static Uint1 Last_magic_feature_cookie=0;

/*-------flat2asn_install_locus_user_string()-------*/

void
flat2asn_install_locus_user_string (CharPtr locus)
{ 
   char buf[30];
                
    StringNCpy(buf,locus, sizeof (buf) - 3);
    StringCat(buf,":");
    buf[29]='\0';
	Last_magic_locus_cookie = 
		Nlm_ErrUserInstall ( buf, Last_magic_locus_cookie);
}

/*------flat2asn_delete_locus_user_string()---*/

void
flat2asn_delete_locus_user_string(void)
{

	Nlm_ErrUserDelete(Last_magic_locus_cookie);
	Last_magic_locus_cookie = 0;
}


/*-------flat2asn_install_accession_user_string()-------*/

void
flat2asn_install_accession_user_string (CharPtr accession)
{
   char buf[30];
                
    StringNCpy(buf,accession, sizeof (buf) - 3);
    StringCat(buf,":");
    buf[29]='\0';

	Last_magic_accession_cookie = 
		Nlm_ErrUserInstall ( buf, Last_magic_accession_cookie);
}

/*------flat2asn_delete_accession_user_string()---*/

void
flat2asn_delete_accession_user_string(void)
{

	Nlm_ErrUserDelete(Last_magic_accession_cookie);
	Last_magic_accession_cookie = 0;
}

/*-------flat2asn_install_feature_user_string()-------*/

void
flat2asn_install_feature_user_string (CharPtr key, CharPtr location)
{
   char buf[130];
   char install[160];
   CharPtr temp = install;
    
    temp = StringMove(temp,"FEAT=");            
    StringNCpy(buf,key, 20);
    buf[19]='\0';
    temp = StringMove(temp,buf);
    temp = StringMove(temp,"[");
    StringNCpy(buf,location,sizeof (buf) - 3);
    buf[127]='\0';
    temp = StringMove(temp,buf);
    temp = StringMove(temp,"]");

	Last_magic_feature_cookie = 
		Nlm_ErrUserInstall ( install, Last_magic_feature_cookie);
}

/*------flat2asn_delete_feature_user_string()---*/

void
flat2asn_delete_feature_user_string(void)
{

	Nlm_ErrUserDelete(Last_magic_feature_cookie);
	Last_magic_feature_cookie = 0;
}
