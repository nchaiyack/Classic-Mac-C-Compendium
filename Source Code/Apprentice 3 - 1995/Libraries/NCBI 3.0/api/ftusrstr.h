/****************************************************************************
*   ftusrstr.h:
*
*
* $Log: ftusrstr.h,v $
 * Revision 1.2  1995/05/15  21:46:05  ostell
 * added Log line
 *
*
*                                     
****************************************************************************/
#ifndef _FTUSRSTR_
#define _FTUSRSTR_

/* used to install the user error strings (locus and accession)
   so that they will appear in ErrPost or ErrPostEx error
   messages, without the need to pass the 'pp' pointer. 
     -Karl */
     
void flat2asn_install_locus_user_string PROTO ((CharPtr locus));
void flat2asn_delete_locus_user_string PROTO ((void));
void flat2asn_install_accession_user_string PROTO ((CharPtr accession));
void flat2asn_delete_accession_user_string PROTO ((void));
void flat2asn_install_feature_user_string PROTO ((CharPtr key, CharPtr location));
void flat2asn_delete_feature_user_string PROTO ((void));

#endif
