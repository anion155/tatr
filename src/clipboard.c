#include "clipboard.h"

#if defined(_WIN32)
#  error "Not implemented yet"
#elif defined(__APPLE__)

#include <stdbool.h>
#include <objc/message.h>
#include <objc/runtime.h>

bool copy_to_clipboard(char *text) {
    if (!text) return false;
    Class NSString = objc_getClass("NSString");
    Class NSPasteboard = objc_getClass("NSPasteboard");

    SEL stringWithUTF8String = sel_registerName("stringWithUTF8String:");
    SEL generalPasteboard = sel_registerName("generalPasteboard");
    SEL clearContents = sel_registerName("clearContents");
    SEL setStringForType = sel_registerName("setString:forType:");

    id nsStr = ((id (*)(id, SEL, const char *))objc_msgSend)((id)NSString, stringWithUTF8String, text);
    if (!nsStr) return false;
    id pasteboard = ((id (*)(id, SEL))objc_msgSend)((id)NSPasteboard, generalPasteboard);
    ((void (*)(id, SEL))objc_msgSend)(pasteboard, clearContents);
    id nsTypeString = ((id (*)(id, SEL, const char *))objc_msgSend)((id)NSString, stringWithUTF8String, "public.utf8-plain-text");
    ((bool (*)(id, SEL, id, id))objc_msgSend)(pasteboard, setStringForType, nsStr, nsTypeString);
    return true;
}

#elif defined(__linux__)
#  error "Not implemented yet"
#else

#include <nob.h>

bool copy_to_clipboard(char *text) {
  if (!text) return false;
  nob_log(NOB_INFO, "Newly created task id is: %s", text);
  return true;
}

#endif
