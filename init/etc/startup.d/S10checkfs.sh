#! /bin/sh

# Include /usr/bin in path to find on_ac_power if /usr/ is on the root
# partition.
PATH=/usr/bin:/sbin:/bin
FSCK_LOGFILE=/var/log/fsck/checkfs
[ "$FSCKFIX" ] || FSCKFIX=no
. /lib/init/vars.sh

. /lib/lsb/init-functions
. /lib/init/mount-functions.sh

	# Trap SIGINT so that we can handle user interupt of fsck.
	trap "" INT

	# See if we're on AC Power.  If not, we're not gonna run our
	# check.  If on_ac_power (in /usr/) is unavailable, behave as
	# before and check all file systems needing it.

# Disabled AC power check until fsck can be told to only check the
# file system if it is corrupt when running on battery. (bug #526398)
#	if which on_ac_power >/dev/null 2>&1
#	then
#		on_ac_power >/dev/null 2>&1
#		if [ $? -eq 1 ]
#		then
#			[ "$VERBOSE" = no ] || log_success_msg "Running on battery power, so skipping file system check."
#			BAT=yes
#		fi
#	fi
	BAT=""

	#
	# Check the rest of the file systems.
	#
	if [ ! "$BAT" ] && [ "$FSCKTYPES" != "none" ]
	then
		if [ -f /forcefsck ] || grep -q -s -w -i "forcefsck" /proc/cmdline
		then
			force="-f"
		else
			force=""
		fi
		if [ "$FSCKFIX" = yes ]
		then
			fix="-y"
		else
			fix="-a"
		fi
		spinner="-C"
		case "$TERM" in
		  dumb|network|unknown|"")
			spinner=""
			;;
		esac
		[ "$(uname -m)" = s390x ] && spinner=""  # This should go away
		FSCKTYPES_OPT=""
		[ "$FSCKTYPES" ] && FSCKTYPES_OPT="-t $FSCKTYPES"
		handle_failed_fsck() {
			echo "ERROR: File system check failed. 
A log is being saved in ${FSCK_LOGFILE} if that location is writable. 
Please repair the file system manually."
			echo "A maintenance shell will now be started. 
CONTROL-D will terminate this shell and resume system boot."
			# Start a single user shell on the console
			if ! sulogin --force $CONSOLE
			then
				echo "ERROR: Attempt to start maintenance shell failed. 
Continuing with system boot in 5 seconds."
				sleep 5
			fi
		}

			echo "Checking file systems"
			fsck $spinner -T -M -A $fix $force $FSCKTYPES_OPT
			FSCKCODE=$?
			if [ "$FSCKCODE" -eq 32 ]
			then
				echo "code $FSCKCODE"
				echo "File system check was interrupted by user"
			elif [ "$FSCKCODE" -gt 1 ]
			then
				echo "code $FSCKCODE"
				handle_failed_fsck
			fi
	fi
	rm -f /fastboot /forcefsck 2>/dev/null
