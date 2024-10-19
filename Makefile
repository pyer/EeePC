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
	sudo install -m 755 init/src/logon    /sbin
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
	sudo mkdir -p /etc/svdir/enabled
	sudo ln -s /etc/svdir/available/udevd /etc/svdir/enabled/ntpd
	sudo ln -s /etc/svdir/available/tty1  /etc/svdir/enabled/tty1
	sudo ln -s /etc/svdir/available/tty2  /etc/svdir/enabled/tty2
	sudo ln -s /etc/svdir/available/tty3  /etc/svdir/enabled/tty3
	sudo ln -s /etc/svdir/available/tty4  /etc/svdir/enabled/tty4
	sudo ln -s /etc/svdir/available/udevd /etc/svdir/enabled/udevd

####################
