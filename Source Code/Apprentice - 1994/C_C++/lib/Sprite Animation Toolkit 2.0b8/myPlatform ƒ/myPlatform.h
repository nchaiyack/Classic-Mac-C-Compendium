/* PlayerSprite's interface */
void InitPlayerSprite();
  pascal void SetupPlayerSprite (SpritePtr);
 pascal void HandlePlayerSprite (SpritePtr);
  pascal void HitPlayerSprite (SpritePtr, SpritePtr);

/* PlatformSprite's interface */
 void InitPlatform();
  pascal void SetupPlatform (SpritePtr);
  pascal void HandlePlatform (SpritePtr);
  pascal void HitPlatform (SpritePtr, SpritePtr);

/* PlatformMovSprite's interface */
 void InitMovPlatMovform();
  pascal void SetupMovPlatform(SpritePtr);
  pascal void HandleMovPlatform(SpritePtr);
  pascal void HitMovPlatform(SpritePtr, SpritePtr);

/* PlatformHMovSprite's interface */
 void InitHMovPlatMovform();
  pascal void SetupHMovPlatform(SpritePtr);
  pascal void HandleHMovPlatform(SpritePtr);
  pascal void HitHMovPlatform(SpritePtr, SpritePtr);
