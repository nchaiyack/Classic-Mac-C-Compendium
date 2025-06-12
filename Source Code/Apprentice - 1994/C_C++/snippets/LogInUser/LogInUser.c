/*** This function takes a username as input, and reads server and volume 
information from resources  It was designed for a very specific purpose,
but should give enough of the form in order to change/re-create the work
itself.. 
***/

/*** Submitted by:  Christopher Roberts  
(Internet address: ctr@pyrite.som.cwru.edu)  ***/

LogInUser(Str255 userName)  
{
        AFPVolMountInfo         *myMount;   /* mounting record  - used to mount the volumes*/
        ParmBlkPtr                      pb;   /* used to hold the mounting record */
        Str255                          sName, sVol;   /*  server name & server volume name  */
        short                           strNum, myErr;   /* string number for enumerated strings & errrorpasser */
        StringHandle            zoneName, hdName, applUser, applServ, applVol;   /*  name of zone for all servers to be in   */
        OSErr                           errorCode;   /*  error code returned by pbmountvol */
        Ptr                                     volumename;  /*  pointer to volume name in myMount..  used for passing  */
        char                            *path;  /*  returned by GetHD  the pathname of users home dir. */
        char                            homeDirPath[60]; /* used to hold the path of the home directory */
        Str255                          volName;
        
        strNum=1;   /* initial setting for string number   */

        myMount=(AFPVolMountInfo *)NewPtr(sizeof(AFPVolMountInfo));  /* allocate space for mount record */

        myMount->media = AppleShareMediaType;  /*  fill in mounting information - general */
        myMount->flags = 0;
        myMount->nbpInterval = 0;
        myMount->nbpCount = 0;
        myMount->uamType = kPassword;
        
        myMount->zoneNameOffset = 24;  /* mounting info -  my offsets for the data from the */
        myMount->serverNameOffset = 57;  /* beginning of the 'myMount' record   */
        myMount->userNameOffset = 117;
        myMount->userPasswordOffset = 149;
        myMount->volPasswordOffset = 158; /* this appears to be unused for simple mounting purposes */
        myMount->volNameOffset = 89;

/**** Load in the actual mounting information.. ****/   

        zoneName=GetString(BASE_RES_ID); /* get the zone name from the appropriate 'STR ' resource */
        
/*** Notice that I do no error checking here..  
                You may include it if you wish by using ResError() ***/
        
        strcpy(myMount->AFPData, PtoCstr(*((Str255 *)*zoneName))); /* copy the zone name in */
        ReleaseResource((Handle)zoneName);
        CtoPstr(myMount->AFPData);

/* I realize that all of this PtoCstr and CtoPstr stuff is not very good
                 and is probably confusing  I did it so I could use the strcpy function.. I would suggest
                 using memcpy myself */

        strcpy(myMount->AFPData+UNAMEOFFSET, PtoCstr(userName)); /* copy the username in */
        CtoPstr(myMount->AFPData+UNAMEOFFSET);
        
        strcpy(myMount->AFPData+UPASSOFFSET, gTempString); /* transfer user password in */
        CtoPstr(myMount->AFPData+UPASSOFFSET); /* in this case it was stored in a global */
        
        strcpy(myMount->AFPData+VOLPASSOFFSET, "\0"); /* null volume password (it doesn't matter)
                CtoPstr(myMount->AFPData+VOLPASSOFFSET);
        
        myMount->length = sizeof(AFPVolMountInfo);  /*compute and enter the length*/
        
        pb=(ParmBlkPtr)NewPtr(sizeof(ParamBlockRec));
        pb->ioParam.ioBuffer = (Ptr)myMount;  /* must have a paramblock in order to mount using 
                                                                           
            the pbvolumemount call */
                
        do    /*   used because of multiple server compatibility   */
        {
                GetIndString(sName, BASE_RES_ID, strNum);       /*read server name from resource */
                myErr=ResError();
                if (sName[0] == NIL_POINTER)   /* if it didn't load a string */
                        {
                                DisposPtr((Ptr)pb);
                                DisposPtr((Ptr)myMount);
                                return (OSErr)42;   /* no more strings - mount unsuccessful */
                        }
                strcpy(myMount->AFPData+SERVEROFFSET, PtoCstr(sName)); /* place server name in data block */
                CtoPstr(myMount->AFPData+SERVEROFFSET); 

                GetIndString(sVol, BASE_RES_ID+1, strNum);  /* read volume name from resource */
                myErr=ResError();  
                if (myErr!=0)
                        HandleError((long)myErr);

                strcpy(myMount->AFPData+VOLOFFSET, PtoCstr(sVol)); /* place it in data block */
                CtoPstr(myMount->AFPData+VOLOFFSET);

                errorCode=PBVolumeMount(pb);   /* mount the actual volume */
                strNum++;
        }
        while (errorCode!=0); /* close to the do statement */
}


/**  unmounting can be done with a call to unmountvol with the name of the volume to unmount. **/

/** any questions/comments, please email me.  
Thanks,
   Chris **/
