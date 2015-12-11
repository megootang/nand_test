LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	nand_test.c

LOCAL_SHARED_LIBRARIES := \
    libhardware_legacy \
    libc \
    libutils 
LOCAL_STATIC_LIBRARIES := libcutils libc
LOCAL_STATIC_LIBRARIES += libcutils libpixelflinger_static
LOCAL_STATIC_LIBRARIES += libunz  libpng libcutils libz
LOCAL_STATIC_LIBRARIES += libstdc++ libc

LOCAL_MODULE := nand_test
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)
