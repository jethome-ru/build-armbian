# Boot from usb stick or microSD card script

echo "boot.src: run armbian_bootargs ..."
run armbian_bootargs

echo "boot.src: setenv bootargs ${bootargs} root=${armbian_root_device} ..."
setenv bootargs ${bootargs} root=${armbian_root_device}

echo "boot.src: run armbian_keyman ..."
run armbian_keyman

echo "boot.src: run armbian_addids ..."
run armbian_addids

echo "boot.src: ext4load usb 0 ${armbian_kernel_addr_r} ${armbian_LINUX} ..."
ext4load usb 0 ${armbian_kernel_addr_r} ${armbian_LINUX}

echo "boot.src: ext4load usb 0 ${armbian_ramdisk_addr_r} ${armbian_INITRD} ..."
ext4load usb 0 ${armbian_ramdisk_addr_r} ${armbian_INITRD}

echo "boot.src: ext4load usb 0 ${dtb_mem_addr} ${armbian_FDT} ..."
ext4load usb 0 ${dtb_mem_addr} ${armbian_FDT}

echo "boot.src: fdt addr ${dtb_mem_addr} ..."
fdt addr ${dtb_mem_addr}

echo "boot.src: booti ${armbian_kernel_addr_r} ${armbian_ramdisk_addr_r} ${dtb_mem_addr} ..."
booti ${armbian_kernel_addr_r} ${armbian_ramdisk_addr_r} ${dtb_mem_addr}

