
typedef struct {
	Boolean				replyReady;
	StandardFileReply		reply;
	short				dirID;
} GlobalsRec, *GlobalsPtr;

#define globalsSize sizeof(GlobalsRec)