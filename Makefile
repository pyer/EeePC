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
cron:
	@echo "Build cron"
	make -C cron/src
	@echo "Uninstall rotatelog and cron"
	sudo dpkg --purge logrotate
	sudo dpkg --purge anacron
	sudo dpkg --purge cron
	sudo dpkg --purge cron-daemon-common
	@echo "Install cron"
	sudo install -m 755 -s cron/src/cron /sbin/
	sudo cp cron/man/bitstring.3 /usr/share/man/man3/
	sudo gzip -f /usr/share/man/man3/bitstring.3 
	sudo cp cron/man/cron.8      /usr/share/man/man8/
	sudo gzip -f /usr/share/man/man8/cron.8
	sudo cp cron/man/crontab.5   /usr/share/man/man5/
	sudo gzip -f /usr/share/man/man5/crontab.5
	sudo rm -rf /etc/cron.*
	sudo rm -rf /var/cron
	@echo "Install rotatelog"
	sudo install -m 755 rotatelog/rotatelog /sbin/
	sudo cp rotatelog/man/rotatelog.8 /usr/share/man/man8/
	sudo gzip -f /usr/share/man/man8/rotatelog.8
	sudo cp -r rotatelog/etc/* /etc/
	sudo chmod +x /etc/rotatelog
	sudo chmod +x /etc/rotatelog.d/*

####################
init:
	@echo "Build init"
	make -C init/src all
	@echo "Install init"
	sudo rm -f /bin/sv /bin/runsv /bin/runsvdir
	sudo rm -f /sbin/run*
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

services:
	sudo cp -r init/etc/* /etc/
	sudo mkdir -p /etc/svdir/enabled
	sudo sv enable cron
	sudo sv enable dhcpd
	sudo sv enable ntpd
	sudo sv enable sshd
	sudo sv enable tty1
	sudo sv enable tty2
	sudo sv enable tty3
	sudo sv enable tty4

####################
network:
	sudo rm    /etc/resolv.conf
	sudo rm -r /etc/resolvconf
	sudo cp -r network/* /etc/

####################
system:
	sudo cp system/timezone /etc/timezone
	sudo rm -f /etc/localtime
	sudo ln -s /usr/share/zoneinfo/Europe/Paris /etc/localtime
	sudo apt install -y isc-dhcp-server
	sudo apt install -y ntp
	sudo apt install -y ssh
	sudo updatedb

####################
clean_system:
	sudo rm -rf /etc/sv
	sudo rm -rf /etc/rc*
	sudo rm -rf /etc/init.d
	sudo rm -rf /etc/runit*
	sudo rm -rf /etc/systemd

####################
