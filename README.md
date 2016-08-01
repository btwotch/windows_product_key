# windows_product_key
A simple kernel module to show the windows product key of your machine via ACPI

# USAGE
```
make
insmod windows_product_key.ko
cat /sys/devices/platform/windows_product_key/key
rmmod windows_product_key
```
