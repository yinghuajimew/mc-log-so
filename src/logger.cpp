#include <android/log.h>
#include <dlfcn.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdarg.h> // 修复可能的 va_list 未定义问题

#define TAG "MCNativeLog"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

// ------------------------------------------------
// 用 bhook 做 PLT hook（不需要 root）
// 引入 bytehook 头文件
// ------------------------------------------------
#include "bytehook.h"

// 原始函数指针
static int (*orig_log_write)(int, const char*, const char*) = nullptr;
static int (*orig_log_vprint)(int, const char*, const char*, va_list) = nullptr;

// Hook: __android_log_write
static int my_log_write(int prio, const char* tag, const char* msg) {
    // 强制加入 Stack 清理逻辑（ByteHook 必须保留，不可省略，否则引发 Crash）
    BYTEHOOK_STACK_SCOPE();

    // 过滤空内容
    if (msg && *msg) {
        __android_log_print(prio, tag ? tag : "MC", "%s", msg);
    }
    BYTEHOOK_CALL_PREV(my_log_write, prio, tag, msg);
    return 0;
}

// Hook: __android_log_vprint
static int my_log_vprint(int prio, const char* tag, const char* fmt, va_list ap) {
    // 同理必须加栈清理逻辑
    BYTEHOOK_STACK_SCOPE();

    if (fmt) {
        char buf[4096];
        vsnprintf(buf, sizeof(buf), fmt, ap);
        __android_log_print(prio, tag ? tag : "MC", "%s", buf);
    }
    // vprint 无法直接 CALL_PREV 传 va_list，直接返回
    return 0;
}

// ------------------------------------------------
// 自动构造函数，SO 加载时立即运行
// ------------------------------------------------
__attribute__((constructor))
static void init_logger() {
    LOGI("=== MC Logger SO loaded ===");

    bytehook_init(BYTEHOOK_MODE_AUTOMATIC, false);

    // hook libminecraftpe.so 里对 liblog 的调用
    bytehook_hook_single(
        "libminecraftpe.so",
        "liblog.so",
        "__android_log_write",
        (void*)my_log_write,
        nullptr, nullptr
    );

    bytehook_hook_single(
        "libminecraftpe.so",
        "liblog.so",
        "__android_log_vprint",
        (void*)my_log_vprint,
        nullptr, nullptr
    );

    LOGI("=== Hooks installed ===");
}