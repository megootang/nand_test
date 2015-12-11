#!/bin/sh
adb root
sleep 2
adb remount
echo press any key to continue...
read a
adb shell rm /data/*.txt
adb shell rm /data/test/*.txt
adb shell rm -r /data/test
adb push nand_test /system/bin/
adb push preloadapp.sh /system/xbin
adb shell chmod 4755 /system/bin/nand_test
adb shell chmod 4755 /system/xbin/preloadapp.sh
adb shell sync
adb shell reboot
