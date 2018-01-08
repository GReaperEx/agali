.PHONY : ISO.iso
ISO.iso :
	@mkdir -p ISO/boot/kernel >/dev/null
	@mkdir -p ISO/boot/loader >/dev/null
	make -C bloaders/
	cp bloaders/cdrom.sys ISO/boot/loader/loader.sys
	fallocate -l 3K ISO/boot/kernel/os.sys
	mkisofs -R -b boot/loader/loader.sys -no-emul-boot -boot-load-size 4 -boot-info-table -o ISO.iso ISO/

