#ifndef PBVR__KEY_FRAME_ANIMATION_H_INCLUDE
#define PBVR__KEY_FRAME_ANIMATION_H_INCLUDE

extern void KeyFrameAnimationInit();
extern int  KeyFrameAnimationAdd( kvs::Xform xform);
extern void KeyFrameAnimationStart();
extern void KeyFrameAnimationStop();
extern void KeyFrameAnimationToggle();
extern void KeyFrameAnimationDelete();
extern int  KeyFrameAnimationErase();
extern void KeyFrameAnimationWrite();
extern int  KeyFrameAnimationRead();
extern std::vector<kvs::Xform>* KeyFrameAnimationGetXforms();
extern std::vector<int>* KeyFrameAnimationGetTimeSteps();

#endif    // PBVR__KEY_FRAME_ANIMATION_H_INCLUDE
