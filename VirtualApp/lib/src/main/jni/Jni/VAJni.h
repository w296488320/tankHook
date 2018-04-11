//
// VirtualApp Native Project
//

#ifndef NDK_CORE_H
#define NDK_CORE_H

#include <jni.h>
#include <stdlib.h>


#include "Helper.h"
#include "Foundation/VMPatch.h"
#include "Foundation/IOUniformer.h"

extern alias_ref<jclass> nativeEngineClass;
extern "C" {
#endif
/*
 * Class:     com_tencent_Native
 * Method:    hook
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_tencent_Native_hook
        (JNIEnv *, jclass, jstring,jstring);

#ifdef __cplusplus
}
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved);
JNIEXPORT void JNICALL JNI_OnUnload(JavaVM* vm, void* reserved);


#endif //NDK_CORE_H
