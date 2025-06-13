/* ClipWarnDialog.c */
/*****************************************************************************/
/*                                                                           */
/*    Out Of Phase:  Digital Music Synthesis on General Purpose Computers    */
/*    Copyright (C) 1994  Thomas R. Lawrence                                 */
/*                                                                           */
/*    This program is free software; you can redistribute it and/or modify   */
/*    it under the terms of the GNU General Public License as published by   */
/*    the Free Software Foundation; either version 2 of the License, or      */
/*    (at your option) any later version.                                    */
/*                                                                           */
/*    This program is distributed in the hope that it will be useful,        */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          */
/*    GNU General Public License for more details.                           */
/*                                                                           */
/*    You should have received a copy of the GNU General Public License      */
/*    along with this program; if not, write to the Free Software            */
/*    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.              */
/*                                                                           */
/*    Thomas R. Lawrence can be reached at tomlaw@world.std.com.             */
/*                                                                           */
/*****************************************************************************/

#include "MiscInfo.h"
#include "Audit.h"
#include "Debug.h"
#include "Definitions.h"

#include "ClipWarnDialog.h"
#include "Numbers.h"
#include "DataMunging.h"
#include "Alert.h"
#include "Memory.h"


/* present a dialog box detailing how much clipping occurred.  the DontTellHowToFix */
/* parameter determines whether the inverse volume correction will be displayed. */
void									ClipWarnDialog(long ClippedSampleCount, long TotalSampleCount,
												double MaxExtent, double Correction, MyBoolean DontTellHowToFix)
	{
		char*								Clipped;

		Clipped = IntegerToString(ClippedSampleCount);
		if (Clipped != NIL)
			{
				char*								OutOf;

				OutOf = StringToBlockCopy(" out of ");
				if (OutOf != NIL)
					{
						char*								Concatted;

						Concatted = ConcatBlockCopy(Clipped,OutOf);
						if (Concatted != NIL)
							{
								char*								Total;

								Total = IntegerToString(TotalSampleCount);
								if (Total != NIL)
									{
										char*								ClipString;

										ClipString = ConcatBlockCopy(Concatted,Total);
										if (ClipString != NIL)
											{
												char*								MiddleInfo;

												MiddleInfo = StringToBlockCopy(" samples were clipped, "
													"with a maximum overextent of ");
												if (MiddleInfo != NIL)
													{
														char*								MostOfIt;

														MostOfIt = ConcatBlockCopy(ClipString,MiddleInfo);
														if (MostOfIt != NIL)
															{
																char*								OverExtent;

																OverExtent = LongDoubleToString(MaxExtent,6,.1,1e6);
																if (OverExtent != NIL)
																	{
																		char*								FinalString;

																		FinalString = ConcatBlockCopy(MostOfIt,
																			OverExtent);
																		if (FinalString != NIL)
																			{
																				char*								Blurb;

																				if (DontTellHowToFix)
																					{
																						Blurb = StringToBlockCopy(".");
																					}
																				 else
																					{
																						Blurb = StringToBlockCopy(".  Set the "
																							"inverse volume to be greater than ");
																					}
																				if (Blurb != NIL)
																					{
																						char*								PutTogether;

																						PutTogether = ConcatBlockCopy(FinalString,Blurb);
																						if (PutTogether != NIL)
																							{
																								char*								CorrectionStr;

																								if (DontTellHowToFix)
																									{
																										CorrectionStr = AllocPtrCanFail(0,"");
																									}
																								 else
																									{
																										CorrectionStr = LongDoubleToString(Correction,6,.1,1e6);
																									}
																								if (CorrectionStr != NIL)
																									{
																										char*								Corrected;

																										Corrected = ConcatBlockCopy(PutTogether,CorrectionStr);
																										if (Corrected != NIL)
																											{
																												char*								NullTerminated;

																											NullTerminated = BlockToStringCopy(Corrected);
																											if (NullTerminated != NIL)
																												{
																													if (DontTellHowToFix)
																														{
																															AlertInfo("_",NullTerminated);
																														}
																													 else
																														{
																															AlertInfo("_ to eliminate the clipping.",NullTerminated);
																														}
																													ReleasePtr(NullTerminated);
																												}
																												ReleasePtr(Corrected);
																											}
																										ReleasePtr(CorrectionStr);
																									}
																								ReleasePtr(PutTogether);
																							}
																						ReleasePtr(Blurb);
																					}
																				ReleasePtr(FinalString);
																			}
																		ReleasePtr(OverExtent);
																	}
																ReleasePtr(MostOfIt);
															}
														ReleasePtr(MiddleInfo);
													}
												ReleasePtr(ClipString);
											}
										ReleasePtr(Total);
									}
								ReleasePtr(Concatted);
							}
						ReleasePtr(OutOf);
					}
				ReleasePtr(Clipped);
			}
	}


/* present a dialog box detailing how much clamping occurred and how to fix it. */
void									ClampWarnDialog(float Clamping, double Correction)
	{
		char*								Clamped;

		Clamped = LongDoubleToString(Clamping,5,1e-4,1e6);
		if (Clamped != NIL)
			{
				char*								Lead;

				Lead = StringToBlockCopy("Some volumes were clamped.  The maximum "
					"overextent was ");
				if (Lead != NIL)
					{
						char*								LeadPlusNum;

						LeadPlusNum = ConcatBlockCopy(Lead,Clamped);
						if (LeadPlusNum != NIL)
							{
								char*								Middle;

								Middle = StringToBlockCopy(".  Clamping can be eliminated by "
									"setting the inverse volume to be greater than _.");
								if (Middle != NIL)
									{
										char*								InvVol;

										InvVol = LongDoubleToString(Correction,5,1e-4,1e6);
										if (InvVol != NIL)
											{
												char*								Most;

												Most = ConcatBlockCopy(LeadPlusNum,Middle);
												if (Most != NIL)
													{
														char*								MostNullTerm;

														MostNullTerm = BlockToStringCopy(Most);
														if (MostNullTerm != NIL)
															{
																char*								InvVolNullTerm;

																InvVolNullTerm = BlockToStringCopy(InvVol);
																if (InvVolNullTerm != NIL)
																	{
																		AlertInfo(MostNullTerm,InvVolNullTerm);
																		ReleasePtr(InvVolNullTerm);
																	}
																ReleasePtr(MostNullTerm);
															}
														ReleasePtr(Most);
													}
												ReleasePtr(InvVol);
											}
										ReleasePtr(Middle);
									}
								ReleasePtr(LeadPlusNum);
							}
						ReleasePtr(Lead);
					}
				ReleasePtr(Clamped);
			}
	}
