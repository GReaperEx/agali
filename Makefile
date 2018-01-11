.PHONY : ISO.iso kernel/kernel.sys bloaders/cdrom.sys clean

ISO.iso : kernel/kernel.sys bloaders/cdrom.sys
	@mkdir -p ISO/boot/kernel >/dev/null
	@mkdir -p ISO/boot/loader >/dev/null
	cp bloaders/cdrom.sys ISO/boot/loader/loader.sys
	cp kernel/kernel.sys ISO/boot/kernel/
	mkisofs -R -b boot/loader/loader.sys -no-emul-boot -boot-load-size 4 -boot-info-table -o ISO.iso ISO/

kernel/kernel.sys :
	make -C kernel/

bloaders/cdrom.sys :
	make -C bloaders/

clean :
	make -C bloaders/ clean
	make -C kernel/ clean
	rm -rf ISO ISO.iso
