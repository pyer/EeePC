#
.PHONY: clean cron init services network config system clean_system

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
	sudo cp cron/etc/crontab /etc/crontab
	sudo task enable cron
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
	sudo rm -f /bin/task /bin/runtask
	sudo rm -f /sbin/run*
	sudo rm -f /sbin/init
	sudo install -m 755 init/src/init     /sbin
	sudo install -m 755 init/src/logon    /sbin
	sudo install -m 755 init/src/runtask  /sbin
	sudo install -m 755 init/src/utmpset  /sbin
	sudo install -m 755 init/task           /sbin
	sudo rm -f /sbin/halt /sbin/poweroff  /sbin/reboot
	sudo ln -s /sbin/init /sbin/halt
	sudo ln -s /sbin/init /sbin/poweroff
	sudo ln -s /sbin/init /sbin/reboot
	@echo "Install scripts"
	sudo rm -rf /etc/startup.d
	sudo rm -rf /etc/shutdown.d
	sudo cp -r init/etc/* /etc/
	@echo "Install tasks"
	sudo mkdir -p /etc/tasks/enabled
	sudo task enable dhcpd
	sudo task enable ntpd
	sudo task enable sshd
	sudo task enable tty1
	sudo task enable tty2
	sudo task enable tty3
	sudo task enable tty4

####################
config:
	mkdir -p /home/pierre/.ssh
	chmod 700 /home/pierre/.ssh
	cp config/authorized_keys /home/pierre/.ssh/
	chmod 600 /home/pierre/.ssh/authorized_keys
	cp config/bashrc.pierre /home/pierre/.bashrc
	sudo cp config/bashrc.root /root/.bashrc
	sudo cp config/timezone /etc/timezone
	sudo rm -f /etc/localtime
	sudo ln -s /usr/share/zoneinfo/Europe/Paris /etc/localtime
	sudo cp config/motd /etc/

####################
system:
	# sudo mkdir -p /usr/share/fonts/psf
	# sudo cp /usr/share/consolefonts/Uni*psf.gz /usr/share/fonts/psf/
	sudo apt install -y iptables
	sudo apt install -y isc-dhcp-server
	sudo touch /var/lib/dhcp/dhcpd.leases
	sudo apt install -y ntp
	#sudo apt install -y ssh
	sudo updatedb

####################
remove_packages:
	sudo mkdir -p /usr/share/fonts/psf
	sudo cp /usr/share/consolefonts/*psf.gz /usr/share/fonts/psf/
	sudo apt purge -y ufw
	sudo apt purge -y x11-common
	sudo apt purge -y libx11-6
	sudo apt purge -y libx11-data
	sudo apt purge -y libx11-6
	sudo apt purge -y xkb-data
	sudo apt purge -y systemd
	sudo apt purge -y libsystemd-shared
	sudo apt purge -y tasksel
	#sudo apt purge -y init-system-helpers
	#sudo apt purge -y runit-init-antix
	#sudo apt purge -y sysvinit-utils-antix
	sudo apt autoremove -y

####################
clean_system:
	sudo rm -rf /etc/sv
	sudo rm -rf /etc/rc*
	sudo rm -rf /etc/init.d
	sudo rm -rf /etc/runit*
	sudo rm -f  /etc/service
	sudo rm -rf /etc/systemd
	sudo rm -f  /etc/slimski.local.conf
	sudo rm -rf /var/log/fsck
	sudo rm -rf /var/log/private
	sudo rm -rf /var/log/runit
	sudo rm -rf /var/log/samba
	sudo rm -rf /etc/ufw
	sudo rm -rf /etc/X11

####################
