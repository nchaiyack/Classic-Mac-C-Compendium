/*  AB_Alert										 Handle this alert */

/* File name:  AB_Alert.h  */
/* Function:  Handle this alert. */
/* This is a NOTE alert, it is used to inform the user of some general information. */
/* This alert is not used if there is a possibility of losing any data. */
/* This alert is called when:    */
/*     */
/* The choices in this alert allow for:    */
/*  
History: 8/18/93 Original by George Warner
   */


int check_stop(void);

void I_A_Alert(void);
void AB_Alert(char *err_msg);


