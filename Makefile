#
.PHONY: clean cron init

####################
help:
	@echo "Usage:"
	@grep -E "^[a-z].*" Makefile | sed -e "s/^\(.*\):.*/  make \1/g"
#	@echo "$(grep -E '^[a-z].*' Makefile | sed -e 's/:.*//g' -e 's/\(.*\)/make \1/g')"

####################
clean:
	make -C cron/src clean
	make -C init/src clean
	find ./ -name "*~" -delete

####################
#build:
#	@echo "Build $(lastword $(MAKECMDGOALS))"

uninstall_packages:
	sudo dpkg --purge logrotate
	sudo dpkg --purge anacron
	sudo dpkg --purge cron
	sudo dpkg --purge cron-daemon-common

####################
cron:
	make -C cron/src

install_cron:
	sudo install -m 755 -s cron/src/cron /sbin/
	sudo cp cron/man/bitstring.3 /usr/share/man/man3/
	sudo cp cron/man/cron.8      /usr/share/man/man8/
	sudo cp cron/man/crontab.5   /usr/share/man/man5/
	sudo rm -rf /etc/cron.*
	sudo rm -rf /var/cron

####################
init:
	make -C init/src all

install_init:
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

install_services:
	sudo cp -r init/etc/* /etc/
	sudo mkdir -p /etc/svdir/enabled
	sudo sv enable dhcpd
	sudo sv enable ntpd
	sudo sv enable sshd
	sudo sv enable tty1
	sudo sv enable tty2
	sudo sv enable tty3
	sudo sv enable tty4

install_etc:
	sudo cp -r etc/* /etc/

####################
