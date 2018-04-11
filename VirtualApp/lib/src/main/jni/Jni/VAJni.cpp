#include <elf.h>//
// VirtualApp Native Project
//
#include <Foundation/IOUniformer.h>
#include <fb/include/fb/Build.h>
#include <fb/include/fb/ALog.h>
#include <fb/include/fb/fbjni.h>
#include "VAJni.h"
#include <android/log.h>
#include <Substrate/SubstrateHook.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "DexFile.h"


#define LOG_TAG "Tencent"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
using namespace facebook::jni;

const char *packagename;
const char *ex0 = "re-initialized>";
const char *ex1 = "zygote";
const char *ex2 = "app_process";
const char *ex3 = "/system/bin/dexopt";
const char *ex4 = "com.google.android.gms";
const char *ex5 = "com.google.android.gms.persistent";
const char *ex6 = "com.google.process.gapps";
const char *ex7 = "com.google.android.gms.wearable";
const char *ex8 = "com.android.phone";
const char *ex9 = "com.android.systemui";
const char *ex10 = "com.google.android.gms.unstable";
const char *ex11 = "android.process.acore";
const char *ex12 = "android.process.media";
const char *ex13 = "dexopt";
const char *globalPath;
#define BUF_SIZE 1024

static void jni_nativeLaunchEngine(alias_ref<jclass> clazz, JArrayClass<jobject> javaMethods,
                                   jstring packageName,
                                   jboolean isArt, jint apiLevel, jint cameraMethodType) {
    hookAndroidVM(javaMethods, packageName, isArt, apiLevel, cameraMethodType);
}

static void *lookup_symbol(const char *libraryname, char *symbolname) {
    void *handle = dlopen(libraryname, RTLD_GLOBAL | RTLD_NOW);
    if (handle != NULL) {
        void *symbol = dlsym(handle, symbolname);
        if (symbol != NULL) {
            return symbol;
        } else {
            LOGE("dl error: %s", dlerror());
            return NULL;
        }
    } else {
        return NULL;
    }
    LOGI("[Tencent]-------lookup_symblo END");
}

static void jni_nativeEnableIORedirect(alias_ref<jclass>, jstring selfSoPath, jint apiLevel,
                                       jint preview_api_level) {
    ScopeUtfString so_path(selfSoPath);
    IOUniformer::startUniformer(so_path.c_str(), apiLevel, preview_api_level);
}

static void jni_nativeIOWhitelist(alias_ref<jclass> jclazz, jstring _path) {
    ScopeUtfString path(_path);
    IOUniformer::whitelist(path.c_str());
}


static void jni_nativeIOForbid(alias_ref<jclass> jclazz, jstring _path) {
    ScopeUtfString path(_path);
    IOUniformer::forbid(path.c_str());
}

int *(*old_checksill)(void *L, int a, int *a1, int *a2);

int *new_checksill(void *L, int a, int *a1, int *a2) {
    return 0;
}

void getNameByPid(pid_t pid, char *task_name) {
    char proc_pid_path[BUF_SIZE];
    char buf[BUF_SIZE];
    sprintf(proc_pid_path, "/proc/%d/status", pid);
    FILE *fp = fopen(proc_pid_path, "r");
    if (NULL != fp) {
        if (fgets(buf, BUF_SIZE - 1, fp) == NULL) {
            fclose(fp);
        }
        fclose(fp);
        sscanf(buf, "%*s %s", task_name);
    }
}
int exclude(char *s) {
    int i = !strcmp(s, ex0) || !strcmp(s, ex1) || !strcmp(s, ex2) || !strcmp(s, ex3) ||
            !strcmp(s, ex4) || !strcmp(s, ex5) || !strcmp(s, ex6) || !strcmp(s, ex7) ||
            !strcmp(s, ex8) || !strcmp(s, ex9) || !strcmp(s, ex10) || !strcmp(s, ex11) ||
            !strcmp(s, ex12) || !strcmp(s, ex13);
    return i;
}
void *(*old_luaL_loadbuffer)(void *L, const char *buff, size_t size, const char *name);
void *new_luaL_loadbuffer(void *L, const char *buff, size_t size, const char *name) {
    if (name != NULL) {
        char *name_t = strdup(name);
        if (name_t != NULL) {
            FILE *file;
            char full_name[256];
            int name_len = strlen(name);
            if (8 < name_len <= 100) {
                char *base_dir = (char *) "/sdcard/lua/"; //dump 保存的文件夹
                char *base_dir1 = (char *) "/sdcard/lua01/";  //hook加载的文件夹
                int i = 0;
                while (i < name_len) {
                    if (name_t[i] == '/') {
                        name_t[i] = '.';
                    }
                    i++;
                }
                /*lua脚本保存*/
                /*
                 sprintf(full_name, "%s%s", base_dir, name_t);
                 if(full_name!=NULL){
                     file = fopen(full_name, "wb");
                     if(file!=NULL){
                       fwrite(buff,1,size,file);
                         fclose(file);
                         free(name_t);
                     }
                    }
                */


                /*lua脚本hook加载*/
                if (strstr(name_t, ".lua")) {
                    sprintf(full_name, "%s%s", base_dir1, name_t);
                    file = fopen(full_name, "r");
                    if (file != NULL) {
                        LOGE("[Tencent]-------path-----%s", full_name);
                        fseek(file, 0, SEEK_END);
                        size_t new_size = ftell(file);
                        fseek(file, 0, SEEK_SET);
                        char *new_buff = (char *) alloca(new_size + 1);
                        fread(new_buff, new_size, 1, file);
                        fclose(file);
                        return old_luaL_loadbuffer(L, new_buff, new_size, name);
                    }
                }
            }
        }
    }
    return old_luaL_loadbuffer(L, buff, size, name);
}

extern "C"
{
JNIEXPORT void JNICALL
Java_com_tencent_Native_hook(JNIEnv *env, jclass jclazz, jstring p, jstring s) {
    const char *path = env->GetStringUTFChars(p, NULL);
    globalPath = path;
   if(strstr(path,"com.KingOfTank.hjtkdg3k")) {
       if (MSHookFunction != NULL) {
           LOGI("[Tencent]-------HookfUNC");
           void *target2 = lookup_symbol(path, "luaL_loadbuffer");
           if (target2) {
               LOGI(": MSHookFunction %p,symbol %p, MSHookFunction, target");
               MSHookFunction(target2, (void *) new_luaL_loadbuffer,
                              (void **) &old_luaL_loadbuffer);
               LOGI("[Tencent]-------End");
           }
       }
   }
}
}


static void jni_nativeIORedirect(alias_ref<jclass> jclazz, jstring origPath, jstring newPath) {
    ScopeUtfString orig_path(origPath);
    ScopeUtfString new_path(newPath);
    IOUniformer::redirect(orig_path.c_str(), new_path.c_str());

}

static jstring jni_nativeGetRedirectedPath(alias_ref<jclass> jclazz, jstring origPath) {
    ScopeUtfString orig_path(origPath);
    const char *redirected_path = IOUniformer::query(orig_path.c_str());
    if (redirected_path != NULL) {
        return Environment::current()->NewStringUTF(redirected_path);
    }
    return NULL;
}

static jstring jni_nativeReverseRedirectedPath(alias_ref<jclass> jclazz, jstring redirectedPath) {
    ScopeUtfString redirected_path(redirectedPath);
    const char *orig_path = IOUniformer::reverse(redirected_path.c_str());
    return Environment::current()->NewStringUTF(orig_path);
}


alias_ref<jclass> nativeEngineClass;


JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *) {
    return initialize(vm, [] {


        nativeEngineClass = findClassStatic("com/lody/virtual/client/NativeEngine");
        nativeEngineClass->registerNatives({
                                                   makeNativeMethod("nativeEnableIORedirect",
                                                                    jni_nativeEnableIORedirect),
                                                   makeNativeMethod("nativeIOWhitelist",
                                                                    jni_nativeIOWhitelist),
                                                   makeNativeMethod("nativeIOForbid",
                                                                    jni_nativeIOForbid),
                                                   makeNativeMethod("nativeIORedirect",
                                                                    jni_nativeIORedirect),
                                                   makeNativeMethod("nativeGetRedirectedPath",
                                                                    jni_nativeGetRedirectedPath),
                                                   makeNativeMethod("nativeReverseRedirectedPath",
                                                                    jni_nativeReverseRedirectedPath),
                                                   makeNativeMethod("nativeLaunchEngine",
                                                                    jni_nativeLaunchEngine),
                                           }

        );
    });

}

extern "C" __attribute__((constructor)) void _init(void) {
    IOUniformer::init_env_before_all();
}


