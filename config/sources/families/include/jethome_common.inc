LINUXFAMILY=meson64
ARCH=arm64
SERIALCON=ttyAML0
SRC_LOADADDR='LOADADDR=0x1080000'
BOOTBRANCH="tag:v2021.07"
BOOTPATCHDIR=${BOOTPATCHDIR:-"u-boot-jethub"}
OVERLAY_PREFIX='meson'

# this family does not need it
ATF_COMPILE="no"


UBOOT_TARGET_MAP="u-boot-dtb.img;;u-boot.bin.sd.bin:u-boot.bin u-boot-dtb.img"
#BOOTSCRIPT="boot-meson64.cmd:boot.cmd"
BOOTENV_FILE='meson.txt'
OVERLAY_PREFIX='meson'

# Set CPUMIN et al if not already set in the board config.
CPUMIN=${CPUMIN:-100000}
CPUMAX=${CPUMAX:-1200000}
GOVERNOR=${GOVERNOR:-conservative}


JETHUB_BLUETOOTH_PACKAGES="rfkill bluetooth bluez bluez-tools"
PACKAGE_LIST_BOARD="libubootenv-tool apparmor $JETHUB_BLUETOOTH_PACKAGES python3-pip"

# this family does not need it
ATF_COMPILE="no"

# !!!! TODO
case "$BOARD" in
	jethub-j100)
		;;
	jethub-j80)
		;;
esac


case $BRANCH in
	legacy)
        KERNELBRANCH='branch:linux-5.9.y'
        KERNELPATCHDIR='jethome-legacy'
	;;
    current)
        KERNELBRANCH='branch:linux-5.10.y'
        KERNELPATCHDIR='jethome-current'
    ;;
    edge)
		KERNELBRANCH='branch:linux-5.13.y'
		KERNELPATCHDIR='jethome-edge'
    ;;
esac

# TODO build on aarch64 ??
if [ "$(uname -m)" = "aarch64" ]; then
	if [[ "$(lsb_release -sc)" == "focal" || "$(lsb_release -sc)" == "hirsute" ]]; then
		PKG_PREFIX="qemu-x86_64-static -L /usr/x86_64-linux-gnu "
	else
		PKG_PREFIX="qemu-x86_64 "
	fi
else
	PKG_PREFIX=""
fi


# TODO: rework postprocess 

# this helper function includes postprocess for p212 and its variants.
# $1 PATH for uboot blob repo
# $2 dir name in uboot blob repo
uboot_gxl_postprocess()
{
	echo !!! !!!
	echo $(pwd)
	echo $(ls)
	echo !!!! !!!!
	mkdir -p $1/u-boot/build
	mv u-boot.bin $1/u-boot/build
	$1/mk_new.sh gxl
	cp $1/build/u-boot.bin.sd.bin u-boot.bin
	#mv u-boot.bin bl33.bin
	#$1/blx_fix.sh 	$1/$2/bl30.bin \
	#		$1/$2/zero_tmp \
	#		$1/$2/bl30_zero.bin \
	#		$1/$2/bl301.bin \
	#		$1/$2/bl301_zero.bin \
	#		$1/$2/bl30_new.bin bl30

	#python $1/acs_tool.pyc $1/$2/bl2.bin \
	#		$1/$2/bl2_acs.bin \
	#		$1/$2/acs.bin 0

	#$1/blx_fix.sh 	$1/$2/bl2_acs.bin \
	#		$1/$2/zero_tmp \
	#		$1/$2/bl2_zero.bin \
	#		$1/$2/bl21.bin \
	#		$1/$2/bl21_zero.bin \
	#		$1/$2/bl2_new.bin bl2

	#$PKG_PREFIX$1/$2/aml_encrypt_gxl 	--bl3enc --input $1/$2/bl30_new.bin
	#$PKG_PREFIX$1/$2/aml_encrypt_gxl 	--bl3enc --input $1/$2/bl31.img
	#$PKG_PREFIX$1/$2/aml_encrypt_gxl 	--bl3enc --input bl33.bin

	#$PKG_PREFIX$1/$2/aml_encrypt_gxl 	--bl2sig --input $1/$2/bl2_new.bin \
	#			--output bl2.n.bin.sig

	#$PKG_PREFIX$1/$2/aml_encrypt_gxl 	--bootmk \
	#			--output u-boot.bin \
	#			--bl2 bl2.n.bin.sig \
	#			--bl30 $1/$2/bl30_new.bin.enc \
	#			--bl31 $1/$2/bl31.img.enc \
	#			--bl33 bl33.bin.enc
}

# this helper function includes postprocess for u200 and its variants.
# $1 PATH for uboot blob repo
# $2 family g12a or g12b
uboot_g12_postprocess()
{
	mv u-boot.bin bl33.bin

	$1/blx_fix.sh $1/bl30.bin \
		      zero_tmp \
		      bl30_zero.bin \
		      $1/bl301.bin \
		      bl301_zero.bin \
		      bl30_new.bin bl30

	$1/blx_fix.sh $1/bl2.bin \
		      zero_tmp \
		      bl2_zero.bin \
		      $1/acs.bin \
		      bl21_zero.bin \
		      bl2_new.bin bl2

	$PKG_PREFIX$1/aml_encrypt_$2 --bl30sig \
			    --input bl30_new.bin \
			    --output bl30_new.bin.g12.enc \
			    --level v3
	$PKG_PREFIX$1/aml_encrypt_$2 --bl3sig \
			    --input bl30_new.bin.g12.enc \
			    --output bl30_new.bin.enc \
			    --level v3 --type bl30
	$PKG_PREFIX$1/aml_encrypt_$2 --bl3sig \
			    --input $1/bl31.img \
			    --output bl31.img.enc \
			    --level v3 --type bl31
	$PKG_PREFIX$1/aml_encrypt_$2 --bl3sig \
			    --input bl33.bin \
			    --compress lz4 \
			    --output bl33.bin.enc \
			    --level v3 --type bl33
	$PKG_PREFIX$1/aml_encrypt_$2 --bl2sig \
			    --input bl2_new.bin \
			    --output bl2.n.bin.sig
	if [ -e $1/lpddr3_1d.fw ]
		then
			$PKG_PREFIX$1/aml_encrypt_$2 --bootmk --output u-boot.bin \
			--bl2 bl2.n.bin.sig \
			--bl30 bl30_new.bin.enc \
			--bl31 bl31.img.enc \
			--bl33 bl33.bin.enc \
			--ddrfw1 $1/ddr4_1d.fw \
			--ddrfw2 $1/ddr4_2d.fw \
			--ddrfw3 $1/ddr3_1d.fw \
			--ddrfw4 $1/piei.fw \
			--ddrfw5 $1/lpddr4_1d.fw \
			--ddrfw6 $1/lpddr4_2d.fw \
			--ddrfw7 $1/diag_lpddr4.fw \
			--ddrfw8 $1/aml_ddr.fw \
			--ddrfw9 $1/lpddr3_1d.fw \
			--level v3
	else
		$PKG_PREFIX$1/aml_encrypt_$2 --bootmk  --output u-boot.bin \
			--bl2 bl2.n.bin.sig \
			--bl30 bl30_new.bin.enc \
			--bl31 bl31.img.enc \
			--bl33 bl33.bin.enc \
			--ddrfw1 $1/ddr4_1d.fw \
			--ddrfw2 $1/ddr4_2d.fw \
			--ddrfw3 $1/ddr3_1d.fw \
			--ddrfw4 $1/piei.fw \
			--ddrfw5 $1/lpddr4_1d.fw \
			--ddrfw6 $1/lpddr4_2d.fw \
			--ddrfw7 $1/diag_lpddr4.fw \
			--ddrfw8 $1/aml_ddr.fw \
			--level v3
	fi

}

# this helper function includes postprocess for meson gxl and gxm.
# $1 PATH for uboot blob repo
uboot_gxl_postprocess_ng()
{
    mv u-boot.bin bl33.bin

    $1/blx_fix.sh $1/bl30.bin \
                       $1/zero_tmp \
                       $1/bl30_zero.bin \
                       $1/bl301.bin \
                       $1/bl301_zero.bin \
                       $1/bl30_new.bin bl30

    python3 $1/acs_tool.py $1/bl2.bin $1/bl2_acs.bin $1/acs.bin 0

    $1/blx_fix.sh $1/bl2_acs.bin \
                       $1/zero_tmp \
                       $1/bl2_zero.bin \
                       $1/bl21.bin \
                       $1/bl21_zero.bin \
                       $1/bl2_new.bin bl2

    $1/aml_encrypt_gxl --bl3enc --input $1/bl30_new.bin
    $1/aml_encrypt_gxl --bl3enc --input $1/bl31.img
    $1/aml_encrypt_gxl --bl3enc --input bl33.bin
    $1/aml_encrypt_gxl --bl2sig --input $1/bl2_new.bin \
			--output bl2.n.bin.sig

    $1/aml_encrypt_gxl --bootmk --output u-boot.bin \
			--bl2 $1/bl2.n.bin.sig \
			--bl30 $1/bl30_new.bin.enc \
			--bl31 $1/bl31.img.enc \
			--bl33 bl33.bin.enc
}

# write u-boot.sd.bin to image (with skip MBR records)
write_uboot_platform()
{
	dd if=$1/u-boot.bin of=$2 bs=1 count=442 conv=fsync > /dev/null 2>&1
	dd if=$1/u-boot.bin of=$2 bs=512 skip=1 seek=1 conv=fsync > /dev/null 2>&1
}

# begin custom functions ----------------------------------------------------
qcomment_default_rsyslog_rules()
{
	local conf_file="$SDCARD/etc/rsyslog.d/50-default.conf"
	local debug=

	local rule_file_arr=(\
/var/log/syslog \
/var/log/auth.log \
/var/log/kern.log \
/var/log/mail.log \
/var/log/mail.err\
)

	rule_file_escaped_arr=("${rule_file_arr[@]}")
	for (( i=0;i<${#rule_file_escaped_arr[*]};i++ )); do
		rule_file_escaped_arr[$i]="${rule_file_arr[$i]//\//\\/}";
	done

	for (( i=0;i<${#rule_file_escaped_arr[*]};i++ )); do
		rule_file_escaped="${rule_file_escaped_arr[$i]}"
		regexp="^[^#][^ \t]+[ \t]+.?${rule_file_escaped}.*$"
		if grep -Pq "$regexp" $conf_file; then
			sed -i -E -e "s/$regexp/#\0/g" $conf_file || { echo "sed 's/$regexp/#\0/g' $conf_file failed"; return 1; }
		else
			echo "grep '$regexp' $conf_file failed"
			return 2
		fi
	done
}

buildjethomecmds ()
{
	# Board Identifiers
	local gpp=${SRC}/cache/toolchain/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-g++
	local gpp_options="-Os -s -Wall -Wextra -std=c++17 -I$SRC/packages/bsp/jethub"
	local gcc=${SRC}/cache/toolchain/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-gcc

	$gpp $gpp_options $SRC/packages/bsp/jethub/jethub_get_cmdline_key.cpp -o $destination/usr/bin/jethub_get_cmdline_key \
	|| exit_with_error "Unable to compile jethub_get_cmdline_key.cpp"
	$gpp $gpp_options $SRC/packages/bsp/jethub/jethub_get_cmdline_key_cpuid.cpp -o $destination/usr/bin/jethub_get_cpuid \
	|| exit_with_error "Unable to compile jethub_get_cmdline_key_cpuid.cpp"

	if [[ $BOARD == jethub-j80 ]]; then
		# Bluetooth
		install -m 755 $SRC/packages/bsp/jethub/jethub-j80/bluetooth/rtk_hciattach $destination/usr/bin/ \
		|| exit_with_error "Unable to install rtk_hciattach"
		
		cp $SRC/packages/bsp/jethub/jethub-j80/bluetooth/jethub-rtk-hciattach.service $destination/lib/systemd/system/ \
		|| exit_with_error "Unable to copy jethub-rtk-hciattach.service"

		install -m 755 $SRC/packages/bsp/jethub/jethub-j80/bluetooth/jethub-rtk-hciattach-starter $destination/usr/lib/armbian/ \
		|| exit_with_error "Unable to install jethub-rtk-hciattach-starter"

		mkdir -p --mode=755 $destination/lib/firmware/ || exit_with_error "Unable to mkdir firmware"
		mkdir --mode=775 $destination/lib/firmware/rtlbt/ || exit_with_error "Unable to mkdir rtlbt"
		cp $SRC/packages/blobs/rtl8822bt_fw/{rtl8822cs_config,rtl8822cs_fw} $destination/lib/firmware/rtlbt/ \
		|| exit_with_error "Unable to copy rtl8822cs_config, rtl8822cs_fw files"

		# Board eth led setup
		$gcc $SRC/packages/bsp/jethub/jethub-j80/jethub_set_eth_leds.c -o $destination/usr/sbin/jethub_set-eth_leds \
		|| exit_with_error "Unable to compile jethub_set_eth_leds.c"
		# Board identifiers
		$gpp $gpp_options $SRC/packages/bsp/jethub/jethub-j80/jethub_get_efuse_raw_key.cpp -o $destination/usr/bin/jethub_get_efuse_raw_key \
		|| exit_with_error "Unable to compile jethub_get_efuse_raw_key.cpp"
		$gpp $gpp_options $SRC/packages/bsp/jethub/jethub-j80/jethub_get_efuse_key_mac.cpp -o $destination/usr/bin/jethub_get_mac \
		|| exit_with_error "Unable to compile jethub_get_efuse_key_mac.cpp"
		$gpp $gpp_options $SRC/packages/bsp/jethub/jethub-j80/jethub_get_efuse_key_serial.cpp -o $destination/usr/bin/jethub_get_serial \
		|| exit_with_error "Unable to compile jethub_get_efuse_key_serial.cpp"
		$gpp $gpp_options $SRC/packages/bsp/jethub/jethub-j80/jethub_get_efuse_key_usid.cpp -o $destination/usr/bin/jethub_get_usid \
		|| exit_with_error "Unable to compile jethub_get_efuse_key_usid.cpp"
	elif [[ $BOARD == jethub-j100 ]]; then
		# Board identifiers
		$gpp $gpp_options $SRC/packages/bsp/jethub/jethub-j100/jethub_get_cmdline_key_mac.cpp -o $destination/usr/bin/jethub_get_mac \
		|| exit_with_error "Unable to compile jethub_get_cmdline_key_mac.cpp"
		$gpp $gpp_options $SRC/packages/bsp/jethub/jethub-j100/jethub_get_cmdline_key_serial.cpp -o $destination/usr/bin/jethub_get_serial \
		|| exit_with_error "Unable to compile jethub_get_cmdline_key_serial.cpp"
		$gpp $gpp_options $SRC/packages/bsp/jethub/jethub-j100/jethub_get_cmdline_key_usid.cpp -o $destination/usr/bin/jethub_get_usid \
		|| exit_with_error "Unable to compile jethub_get_cmdline_key_usid.cpp"

		# Wifi & Bluetooth
		mkdir -p --mode=755 $destination/lib/firmware/ || exit_with_error "Unable to mkdir firmware"
		mkdir --mode=775 $destination/lib/firmware/brcm/ || exit_with_error "Unable to mkdir brcm"
		cp -P $SRC/packages/bsp/jethub/jethub-j100/{BCM4345C0.hcd,'brcmfmac43455-sdio.jethome,j100.txt'} $destination/lib/firmware/brcm/ \
		|| "Unable to copy brcm firmware symlinks"
	else
		exit_with_error "Unexpected board \"$BOARD\""
	fi
}

# end custom functions -----------------------------------------

family_tweaks()
{
	# Log rotation
	comment_default_rsyslog_rules || exit_with_error "Unable to comment default rsyslog rules"

	# Hardware init
	chroot ${SDCARD} /bin/bash -c "systemctl --no-reload enable jethub-initer.service >/dev/null 2>&1"

	# pip3 packages
	chroot ${SDCARD} /bin/bash -c "pip3 install pyserial intelhex python-magic"	>> "${DEST}"/debug/install.log 2>&1

	# Hostapd
	chroot ${SDCARD} /bin/bash -c "systemctl --no-reload disable hostapd.service >/dev/null 2>&1"

	if [[ $BOARD == jethub-j80 ]]; then
		# Bluetooth
		chroot ${SDCARD} /bin/bash -c "systemctl --no-reload enable jethub-rtk-hciattach.service >/dev/null 2>&1"
	fi

	# TODO: check !!!
	# Booting from USB
	#mkimage -C none -A arm -T script -d "${SRC}"/config/bootscripts/boot-jethub.cmd ${SDCARD}/boot.scr >/dev/null 2>&1 \
	#|| exit_with_error "Unable to mkimage boot-jethub.cmd"

}

family_tweaks_bsp()
{
	# build jethub_get_cmdline_key jethub_get_cpuid
	buildjethomecmds

	# TODO: check
	# U-Boot environment
	#cp $SRC/packages/bsp/jethub/fw_env.config $destination/etc/ || exit_with_error "Unable to copy fw_env.config"

	# Filesystem
	mkdir -p $destination/var/lib/armbian || exit_with_error "Unable to mkdir /var/lib/armbian"
	touch $destination/var/lib/armbian/resize_second_stage || exit_with_error "Unable to touch resize_second_stage file"

	# Log rotation
	mkdir -p --mode=755 $destination/etc/rsyslog.d || exit_with_error "Unable to mkdir /etc/rsyslog.d"
	cp $SRC/packages/bsp/jethub/60-jethub-logs-rotate.conf $destination/etc/rsyslog.d/ \
	|| exit_with_error "Unable to copy 60-jethub-logs-rotate.conf"
	cp $SRC/packages/bsp/jethub/jethub-remove-log-file.sh $destination/usr/lib/armbian/ \
	|| exit_with_error "Unable to copy jethub-remove-log-file.sh"

	# Hardware init
	cp $SRC/packages/bsp/jethub/jethub-initer.service $destination/lib/systemd/system/ \
	|| exit_with_error "Unable to copy jethub-initer.service"
	cp $SRC/packages/bsp/jethub/${BOARD}/jethub-init $destination/usr/lib/armbian/ \
	|| exit_with_error "Unable to copy jethub-init"

	if [[ $BOARD == jethub-j80 ]]; then
		# Ethernet LED setup
		cp $SRC/packages/bsp/jethub/jethub-j80/05-jethub_set_eth_leds.rules $destination/etc/udev/rules.d/ 
	fi

	mkdir -p $destination/etc/udev/rules.d
	mkdir -p $destination/usr/local/bin
}
