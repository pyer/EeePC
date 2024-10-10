#
####################
default:
	@echo "Usage:"
	@echo "  make clean"
	@echo "  make cleaner"
	@echo "  make compile"
	@echo "  make install-init"
	@echo "  make install-svdir"

####################
clean:
	make -C init/src clean

cleaner: clean
	make -C init/src cleaner

####################
compile:
	make -C init/src all

####################
install-init:
	sudo rm -f /sbin/init
	sudo install -m 755 init/src/init     /sbin
	sudo install -m 755 init/src/runsvdir /sbin
	sudo install -m 755 init/src/runsv    /sbin
	sudo install -m 755 init/src/utmpset  /sbin
	sudo install -m 755 init/sv           /sbin
	sudo rm -f /sbin/halt /sbin/poweroff  /sbin/reboot
	sudo ln -s /sbin/init /sbin/halt
	sudo ln -s /sbin/init /sbin/poweroff
	sudo ln -s /sbin/init /sbin/reboot

install-svdir:
	sudo cp -r init/etc/* /etc/
	sudo mkdir -p /etc/svdir
	sudo ln -s /etc/svdir.d/udevd /etc/svdir/udevd
	sudo ln -s /etc/svdir.d/tty1  /etc/svdir/tty1
	sudo ln -s /etc/svdir.d/tty2  /etc/svdir/tty2
	sudo ln -s /etc/svdir.d/tty3  /etc/svdir/tty3
	sudo ln -s /etc/svdir.d/tty4  /etc/svdir/tty4

####################
