#include <jni.h>
#include <string>
#include <os_helpers.h>
#include <TestGLSL.h>
#include <tinyxml2.h>
#include <chess_helpers.h>

JavaVM *jvm = nullptr;
jobject activity = nullptr;
jclass gameActivity = nullptr;
jclass myclass = nullptr;
jmethodID performMove = nullptr;
jmethodID performAchievementWon = nullptr;

void *get_jvm() { return jvm; }
void *get_activity() {return activity; }

void detach_main_thread()
{
    jvm->DetachCurrentThread();
}

bool checkExc(JNIEnv* env) {
    if(env->ExceptionCheck()) {
        env->ExceptionDescribe(); // writes to logcat
        env->ExceptionClear();
        return true;
    }
    return false;
}

void javaPerformWon(int opponent_is_cpu, int opponent_is_external, int opponent_is_local, int player_is_white) {
    JNIEnv *env = nullptr;
    if (nullptr == jvm) {
        return;
    }
    int getEnvStat = jvm->GetEnv((void**)&env,JNI_VERSION_1_6);
    if (getEnvStat == JNI_EDETACHED) {
        Base::log("GetEnv: not attached");
        if (jvm->AttachCurrentThread(&env, NULL) != 0) {
            Base::log("Failed to attach");
        }
    } else if (getEnvStat == JNI_OK) {
        //
    } else if (getEnvStat == JNI_EVERSION) {
        Base::log("GetEnv: version not supported");
    }
    performAchievementWon = env->GetMethodID(myclass, "performAchievementWon", "(IIII)V");
    if(checkExc(env))
    {
        return;
    }
    env->CallVoidMethod(gameActivity, performAchievementWon, opponent_is_cpu, opponent_is_external, opponent_is_local, player_is_white);
}

void javaPerformMove(int from_x, int from_y, int to_x, int to_y) {
    JNIEnv *env = nullptr;
    if (nullptr == jvm) {
        return;
    }
    int getEnvStat = jvm->GetEnv((void**)&env,JNI_VERSION_1_6);
    if (getEnvStat == JNI_EDETACHED) {
        Base::log("GetEnv: not attached");
        if (jvm->AttachCurrentThread(&env, NULL) != 0) {
            Base::log("Failed to attach");
        }
    } else if (getEnvStat == JNI_OK) {
        //
    } else if (getEnvStat == JNI_EVERSION) {
        Base::log("GetEnv: version not supported");
    }
    performMove = env->GetMethodID(myclass, "performMove", "(IIII)V");
    if(checkExc(env))
    {
        return;
    }
    env->CallVoidMethod(gameActivity, performMove, from_x, from_y, to_x, to_y);
    /* jvm->DetachCurrentThread(); */
}

extern "C"
void
Java_com_eskilsund_etchess3d_etchess3d_NativeChessActivity_initNative(
        JNIEnv *env,
        jobject thiz) {
    env->GetJavaVM(&jvm);
    activity = (env)->NewGlobalRef(thiz);
    jclass localClass = env->FindClass("com/eskilsund/etchess3d/etchess3d/GameActivity");
    if(checkExc(env))
    {
        return;
    }
    myclass = reinterpret_cast<jclass>(env->NewGlobalRef(localClass));
    release_chess_board();
}

extern "C"
void Java_com_eskilsund_etchess3d_etchess3d_NativeChessActivity_whiteIsCpu(
        JNIEnv *env,
        jobject thiz) {
    set_white_is_cpu();
}

extern "C"
void Java_com_eskilsund_etchess3d_etchess3d_NativeChessActivity_blackIsCpu(
        JNIEnv *env,
        jobject thiz) {
    set_black_is_cpu();
}


extern "C"
void Java_com_eskilsund_etchess3d_etchess3d_NativeChessActivity_whiteIsPlayer(
        JNIEnv *env,
        jobject thiz) {
    set_white_is_player();
}

extern "C"
void Java_com_eskilsund_etchess3d_etchess3d_NativeChessActivity_blackIsPlayer(
        JNIEnv *env,
        jobject thiz) {
    set_black_is_player();
}

extern "C"
void Java_com_eskilsund_etchess3d_etchess3d_NativeChessActivity_blackIsExternalPlayer(
        JNIEnv *env,
        jobject thiz) {
    set_black_is_external_player();
}

extern "C"
void Java_com_eskilsund_etchess3d_etchess3d_NativeChessActivity_whiteIsExternalPlayer(
        JNIEnv *env,
        jobject thiz) {
    set_white_is_external_player();
}

extern "C"
void Java_com_eskilsund_etchess3d_etchess3d_NativeChessActivity_setSearchDepth(
        JNIEnv *env,
        jobject thiz,
        jint depth) {
    set_engine_search_depth(depth);
}

extern "C"
void Java_com_eskilsund_etchess3d_etchess3d_NativeChessActivity_nativePerformMove(
        JNIEnv *env,
        jobject thiz,
        jint from_x,
        jint from_y,
        jint to_x,
        jint to_y) {
    external_user_moved(from_x, from_y, to_x, to_y);
}

extern "C"
void Java_com_eskilsund_etchess3d_etchess3d_NativeChessActivity_setNativeGameActivity(
        JNIEnv* env,
        jobject thiz,
        jclass jGameActivity)
{
    gameActivity = reinterpret_cast<jclass>(env->NewGlobalRef(jGameActivity));
}