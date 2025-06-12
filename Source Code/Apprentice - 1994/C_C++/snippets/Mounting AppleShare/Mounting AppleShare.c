	myMount=(AFPVolMountInfo *)NewPtr(sizeof(AFPVolMountInfo));  /* allocate
space for mount record */

	myMount->media = AppleShareMediaType;  /*  fill in mounting information - general */
	myMount->flags = 0;
	myMount->nbpInterval = 0;
	myMount->nbpCount = 0;
	myMount->uamType = kPassword;
	
	myMount->zoneNameOffset = 24;  /* mounting info -  my offsets for the data from the */
	myMount->serverNameOffset = 57;  /* beginning of the 'myMount' record */
	myMount->userNameOffset = 117;
	myMount->userPasswordOffset = 149;
	myMount->volPasswordOffset = 158;
	myMount->volNameOffset = 89;
	
	zoneName=GetString(BASE_RES_ID);    /*  actual mounting information.. must be in pascal strings */
	
	errorCode=(long)ResError();
	if (errorCode != 0)
		HandleError((long)errorCode);

	strcpy(myMount->AFPData, PtoCstr(*((Str255 *)*zoneName)));
	ReleaseResource((Handle)zoneName);
	CtoPstr(myMount->AFPData);

	strcpy(myMount->AFPData+UNAMEOFFSET, PtoCstr(userName));
	CtoPstr(myMount->AFPData+UNAMEOFFSET);
	
	strcpy(myMount->AFPData+UPASSOFFSET, gTempString);
	CtoPstr(myMount->AFPData+UPASSOFFSET);
	
	strcpy(myMount->AFPData+VOLPASSOFFSET, "\0");
	
	CtoPstr(myMount->AFPData+VOLPASSOFFSET);
	
	myMount->length = sizeof(AFPVolMountInfo);
	
	pb=(ParmBlkPtr)NewPtr(sizeof(ParamBlockRec));
	pb->ioParam.ioBuffer = (Ptr)myMount;
		
	do    /*   used because of multiple server compatibility   */
	{
		GetIndString(sName, BASE_RES_ID, strNum);	/*read server name from resource */
		myErr=ResError();
		if (sName[0] == NIL_POINTER)   /* if it didn't load a string */
			{
				DisposPtr((Ptr)pb);
				DisposPtr((Ptr)myMount);
				return (OSErr)42;   /* tells program that the strings have run out */
			}
		strcpy(myMount->AFPData+SERVEROFFSET, PtoCstr(sName));
		CtoPstr(myMount->AFPData+SERVEROFFSET); 

		GetIndString(sVol, BASE_RES_ID+1, strNum);  /* read volume name from resource */
		myErr=ResError();  
		if (myErr!=0)
			HandleError((long)myErr);

		strcpy(myMount->AFPData+VOLOFFSET, PtoCstr(sVol));
		CtoPstr(myMount->AFPData+VOLOFFSET);

		errorCode=PBVolumeMount(pb);   /* mount the actual volume */
		strNum++;
	}
	while (errorCode!=0); /* close to the do statement */
	volumename = (myMount->AFPData+VOLOFFSET);  /*set value of volumename to pass it */
	
	strcpy((char *)(volName), PtoCstr(*(Str255 *)volumename)); /* stores the volume name */
	strcat((char *)(volName), ":");   /* for later unmounting....   */
	CtoPstr((char *)volumename);
	CtoPstr((char *)(volName));
