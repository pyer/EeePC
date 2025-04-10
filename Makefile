#
####################
default:
	@echo "Usage:"
	@echo "  make clean"
	@echo "  make cleaner"
	@echo "  make compile"
	@echo "  make install-etc"
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
install-etc:
	sudo cp -r etc/* /etc/

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
	sudo sv enable dhcpd
	sudo sv enable ntpd
	sudo sv enable sshd
	sudo sv enable tty1
	sudo sv enable tty2
	sudo sv enable tty3
	sudo sv enable tty4

####################
