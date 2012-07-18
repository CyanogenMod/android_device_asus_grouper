# Release name
PRODUCT_RELEASE_NAME := N7

# Boot animation
TARGET_BOOTANIMATION_NAME := vertical-800x1280

# Inherit some common cyanogenmod stuff.
$(call inherit-product, vendor/cm/config/common_full_tablet_wifionly.mk)

# Enhanced NFC
$(call inherit-product, vendor/cm/config/nfc_enhanced.mk)

# Inherit device configuration
$(call inherit-product, device/asus/grouper/full_grouper.mk)

#
# Setup device specific product configuration.
#

PRODUCT_DEVICE := grouper
PRODUCT_NAME := cm_grouper
PRODUCT_BRAND := google
PRODUCT_MODEL := Nexus 7
PRODUCT_MANUFACTURER := asus

PRODUCT_BUILD_PROP_OVERRIDES += PRODUCT_NAME=nakasi BUILD_FINGERPRINT=google/nakasi/grouper:4.1.1/JRO03D/402395:user/release-keys PRIVATE_BUILD_DESC="nakasi-user 4.1.1 JRO03D 402395 release-keys"
