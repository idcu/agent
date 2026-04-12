#include "idcu/os/os.h"
#include <jni.h>
#include <android/log.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#if IDCU_OS_ANDROID

static JavaVM* g_jvm = NULL;
static jobject g_context = NULL;

int idcu_exe_path(char* buf, size_t buf_len)
{
    if (!buf || buf_len == 0) {
        return -EINVAL;
    }

    char path[1024];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len > 0) {
        path[len] = '\0';
        strncpy(buf, path, buf_len - 1);
        buf[buf_len - 1] = '\0';
        return 0;
    }

    return -1;
}

int idcu_data_dir(char* buf, size_t buf_len)
{
    if (!buf || buf_len == 0) {
        return -EINVAL;
    }

    JNIEnv* env;
    int get_env_status = (*g_jvm)->GetEnv(g_jvm, (void**)&env, JNI_VERSION_1_6);
    
    if (get_env_status == JNI_EDETACHED) {
        if ((*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL) != 0) {
            return -1;
        }
    } else if (get_env_status != JNI_OK) {
        return -1;
    }

    if (g_context) {
        jclass context_class = (*env)->GetObjectClass(env, g_context);
        jmethodID get_files_dir = (*env)->GetMethodID(env, context_class, "getFilesDir", "()Ljava/io/File;");
        jobject file_obj = (*env)->CallObjectMethod(env, g_context, get_files_dir);
        
        jclass file_class = (*env)->GetObjectClass(env, file_obj);
        jmethodID get_path = (*env)->GetMethodID(env, file_class, "getPath", "()Ljava/lang/String;");
        jstring path_str = (jstring)(*env)->CallObjectMethod(env, file_obj, get_path);
        
        const char* path_utf = (*env)->GetStringUTFChars(env, path_str, NULL);
        strncpy(buf, path_utf, buf_len - 1);
        buf[buf_len - 1] = '\0';
        (*env)->ReleaseStringUTFChars(env, path_str, path_utf);
        
        if (get_env_status == JNI_EDETACHED) {
            (*g_jvm)->DetachCurrentThread(g_jvm);
        }
        
        return 0;
    }

    const char* home = getenv("EXTERNAL_STORAGE");
    if (home) {
        strncpy(buf, home, buf_len - 1);
        buf[buf_len - 1] = '\0';
        return 0;
    }

    strncpy(buf, "/sdcard", buf_len - 1);
    buf[buf_len - 1] = '\0';
    return 0;
}

int idcu_home_dir(char* buf, size_t buf_len)
{
    if (!buf || buf_len == 0) {
        return -EINVAL;
    }

    const char* home = getenv("HOME");
    if (home) {
        strncpy(buf, home, buf_len - 1);
        buf[buf_len - 1] = '\0';
        return 0;
    }

    return idcu_data_dir(buf, buf_len);
}

int idcu_path_join(char* buf, size_t buf_len, const char* path1, const char* path2)
{
    if (!buf || buf_len == 0 || !path1 || !path2) {
        return -EINVAL;
    }

    size_t len1 = strlen(path1);
    size_t len2 = strlen(path2);
    int needs_sep = (len1 > 0 && path1[len1 - 1] != '/');
    
    if (len1 + (needs_sep ? 1 : 0) + len2 + 1 > buf_len) {
        return -ENOMEM;
    }

    strcpy(buf, path1);
    if (needs_sep) {
        strcat(buf, "/");
    }
    strcat(buf, path2);
    return 0;
}

#endif
