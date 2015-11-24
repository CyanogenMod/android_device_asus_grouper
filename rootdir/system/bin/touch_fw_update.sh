#!/system/bin/sh

echo /system/etc/firmware/touch_fw.ekt > /sys/bus/i2c/drivers/elan-ktf3k/1-0010/update_fw
