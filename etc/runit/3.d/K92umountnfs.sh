#! /bin/sh

PATH=/usr/sbin:/usr/bin:/sbin:/bin
KERNEL="$(uname -s)"
RELEASE="$(uname -r)"
. /lib/init/vars.sh

. /lib/lsb/init-functions

case "${KERNEL}:${RELEASE}" in
  Linux:[01].*|Linux:2.[01].*)
	FLAGS=""
	;;
  Linux:2.[23].*|Linux:2.4.?|Linux:2.4.?-*|Linux:2.4.10|Linux:2.4.10-*)
	FLAGS="-f"
	;;
  *)
	FLAGS="-f -l"
	;;
esac

# Write a reboot record to /var/log/wtmp before unmounting
halt -w

# Remove bootclean flag files (precaution against symlink attacks)
rm -f /tmp/.clean /run/.clean /run/lock/.clean

#
# Make list of points to unmount in reverse order of their creation
#

DIRS=""
while read -r DEV MTPT FSTYPE OPTS REST
do
	case "$MTPT" in
		 /|/proc|/dev|/dev/pts|/dev/shm|/proc/*|/sys|/run|/run/*)
		continue
		;;
	esac
	case "$FSTYPE" in
		 nfs|nfs4|smbfs|ncp|ncpfs|cifs|coda|ceph)
		DIRS="$MTPT $DIRS"
		;;
		 proc|procfs|linprocfs|devpts|usbfs|usbdevfs|sysfs)
		DIRS="$MTPT $DIRS"
		;;
	esac
	case "$OPTS" in
		 _netdev|*,_netdev|_netdev,*|*,_netdev,*)
		case "$FSTYPE" in
			 ocfs2|gfs)
				;;
			 *)
			DIRS="$MTPT $DIRS"
			;;
		esac
		;;
	esac
done < /etc/mtab

if [ "$DIRS" ]
then
	[ "$VERBOSE" = no ] || log_action_begin_msg "Unmounting remote and non-toplevel virtual filesystems"
	fstab-decode umount $FLAGS $DIRS
	ES=$?
	[ "$VERBOSE" = no ] || log_action_end_msg $ES
fi

# emit unmounted-remote-filesystems hook point so any upstart jobs
# that support remote filesystems can be stopped
if [ -x /sbin/initctl ]; then
	initctl --quiet emit unmounted-remote-filesystems 2>/dev/null || true
fi
