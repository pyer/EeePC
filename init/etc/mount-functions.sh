#
# Functions used by several mount* scripts in initscripts package
#
# Sourcer must source /lib/lsb/init-functions.sh

# Get size of physical RAM in kiB
ram_size ()
{
  [ -r /proc/meminfo ] && \
  grep MemTotal /proc/meminfo | \
  sed -e 's;.*[[:space:]]\([0-9][0-9]*\)[[:space:]]kB.*;\1;' || :
}

# Get size of swap space in kiB
swap_size ()
{
  [ -r /proc/meminfo ] && \
  grep SwapTotal /proc/meminfo | \
  sed -e 's;.*[[:space:]]\([0-9][0-9]*\)[[:space:]]kB.*;\1;' || :
}

#
# Get total VM size in kiB.  Prints nothing if no RAM and/or swap was
# detectable.
#
vm_size ()
{
    RAM=$(ram_size)
    SWAP=$(swap_size)

    RAM="${RAM:=0}"
    SWAP="${SWAP:=0}"

    echo $((RAM + SWAP))
    return 0;
}

run_size ()
{
  # 10% of memory
  RAM=$(ram_size)
  echo "$((RAM / 10))"
  return 0;
}

tmp_size ()
{
  # 20% of free memory
  RAM=$(ram_size)
  echo "$((RAM / 5))"
  return 0;
}


# Read /etc/fstab, looking for:
# 1) The root filesystem, resolving LABEL=*|UUID=* entries to the
#  device node,
# 2) Swap that is on a md device or a file that may be on a md
#  device,
_read_fstab () {
  echo "fstabroot=/dev/root"
  echo "rootdev=none"
  echo "roottype=none"
  echo "rootopts=defaults"
  echo "rootmode=rw"
  echo "rootcheck=no"
  echo "swap_on_lv=no"
  echo "swap_on_file=no"

  file="/etc/fstab"
  if [ -f "$file" ]; then
      while read DEV MTPT FSTYPE OPTS DUMP PASS JUNK; do
        case "$DEV" in
          ""|\#*)
          continue
          ;;
          /dev/mapper/*)
          [ "$FSTYPE" = "swap" ] && echo swap_on_lv=yes
          ;;
          /dev/*)
          ;;
          LABEL=*|UUID=*)
          if [ "$MTPT" = "/" ] && [ -x /sbin/findfs ]
          then
            DEV="$(findfs "$DEV")"
          fi
          ;;
          /*)
          [ "$FSTYPE" = "swap" ] && echo swap_on_file=yes
          ;;
          *)
          ;;
        esac
        [ "$MTPT" != "/" ] && continue
        echo rootdev=\"$DEV\"
        echo fstabroot=\"$DEV\"
        echo rootopts=\"$OPTS\"
        echo roottype=\"$FSTYPE\"
        ( [ "$PASS" != 0 ] && [ "$PASS" != "" ]   ) && echo rootcheck=yes
        ( [ "$FSTYPE" = "nfs" ] || [ "$FSTYPE" = "nfs4" ] ) && echo rootcheck=no
        case "$OPTS" in
          ro|ro,*|*,ro|*,ro,*)
          echo rootmode=ro
          ;;
        esac
      done < "$file"
  fi
}

# Read /etc/fstab, looking for:
# 1) The root filesystem, resolving LABEL=*|UUID=* entries to the
#  device node,
# 2) Swap that is on a md device or a file that may be on a md
#  device,

read_fstab () {
  eval "$(_read_fstab)"
}

# Find a specific fstab entry
# $1=mountpoint
# $2=fstype (optional)
_read_fstab_entry () {
  # Not found by default.
  echo "MNT_FSNAME="
  echo "MNT_DIR="
  echo "MNT_TYPE="
  echo "MNT_OPTS="
  echo "MNT_FREQ="
  echo "MNT_PASS="

  file="/etc/fstab"
  if [ -f "$file" ]; then
      while read MNT_FSNAME MNT_DIR MNT_TYPE MNT_OPTS MNT_FREQ MNT_PASS MNT_JUNK; do
        case "$MNT_FSNAME" in
          ""|\#*)
          continue;
          ;;
        esac
        if [ "$MNT_DIR" = "$1" ]; then
          if [ -n "$2" ]; then
            [ "$MNT_TYPE" = "$2" ] || continue;
          fi
                                  echo "MNT_FSNAME=$MNT_FSNAME"
                                  echo "MNT_DIR=$MNT_DIR"
                                  echo "MNT_TYPE=$MNT_TYPE"
                                  echo "MNT_OPTS=$MNT_OPTS"
                                  echo "MNT_FREQ=$MNT_FREQ"
                                  echo "MNT_PASS=$MNT_PASS"
          break 2
        fi
        MNT_DIR=""
      done < "$file"
  fi
}

# Find a specific fstab entry
# $1=mountpoint
# $2=fstype (optional)
# returns 0 on success, 1 on failure (not found or no fstab)
read_fstab_entry () {
  eval "$(_read_fstab_entry "$1" "$2")"

  # Not found by default.
  found=1
  if [ "$1" = "$MNT_DIR" ]; then
    found=0
  fi

  return $found
}

# Mount kernel and device file systems.
# $1: mount mode (mount, remount)
# $2: file system type
# $3: alternative file system type (or empty string if none)
# $4: mount point
# $5: mount device name
# $6... : extra mount program options
domount () {
  MOUNTMODE="$1"
  PRIFSTYPE="$2"
  ALTFSTYPE="$3"
  MTPT="$4"
  DEVNAME="$5"
  CALLER_OPTS="$6"

  KERNEL="$(uname -s)"
  # Figure out filesystem type from primary and alternative type
  FSTYPE=
  # Filesystem-specific mount options
  FS_OPTS=
  # Mount options from fstab
  FSTAB_OPTS=

  if [ "$MOUNTMODE" = remount ] ; then
    case "$KERNEL" in
      *FreeBSD)
        case "$PRIFSTYPE" in
          proc|tmpfs|sysfs)
            # can't be remounted
            return 0
          ;;
        esac
      ;;
    esac
  fi

  if [ "$PRIFSTYPE" = proc ]; then
    case "$KERNEL" in
      Linux)     FSTYPE=proc ;;
      GNU)       FSTYPE=proc; FS_OPTS="-ocompatible" ;;
      *FreeBSD)  FSTYPE=linprocfs ;;
      *)         FSTYPE=procfs ;;
    esac
  elif [ "$PRIFSTYPE" = bind ]; then
    case "$KERNEL" in
      Linux)     FSTYPE="$DEVNAME"; FS_OPTS="-obind" ;;
      *FreeBSD)  FSTYPE=nullfs ;;
      GNU)       FSTYPE=firmlink ;;
      *)         FSTYPE=none ;;
    esac
  elif [ "$PRIFSTYPE" = tmpfs ]; then
    # always accept tmpfs, to mount /run before /proc
    case "$KERNEL" in
      *)  FSTYPE=$PRIFSTYPE ;;
    esac
  elif grep -E -qs "$PRIFSTYPE\$" /proc/filesystems; then
    FSTYPE=$PRIFSTYPE
  elif grep -E -qs "$ALTFSTYPE\$" /proc/filesystems; then
    FSTYPE=$ALTFSTYPE
  fi

  # Filesystem not supported by kernel
  if [ ! "$FSTYPE" ]; then
    if [ "$ALTFSTYPE" ]; then
      echo "Filesystem types '$PRIFSTYPE' and '$ALTFSTYPE' are not supported. Skipping mount."
    else
      echo "Filesystem type '$PRIFSTYPE' is not supported. Skipping mount."
    fi
    return
  fi

  # We give file system type as device name if not specified as
  # an argument
  if [ -z "$DEVNAME" ] ; then
      DEVNAME=$FSTYPE
  fi

  # Get the mount options from /etc/fstab
  if read_fstab_entry "$MTPT" "$FSTYPE"; then
    case "$MNT_OPTS" in
      noauto|*,noauto|noauto,*|*,noauto,*)
        return
        ;;
      ?*)
        FSTAB_OPTS="-o$MNT_OPTS"
        ;;
    esac
  fi

  if [ ! -d "$MTPT" ]
  then
    echo "Mount point '$MTPT' does not exist. Skipping mount."
    return
  fi

  if [ "$MOUNTMODE" = "mount_noupdate" ]; then
    MOUNTFLAGS="-n"
    MOUNTMODE=mount
  fi
  if [ "$MOUNTMODE" = "remount_noupdate" ]; then
    MOUNTFLAGS="-n"
    MOUNTMODE=remount
  fi

  case "$MOUNTMODE" in
    mount)
      if mountpoint -q "$MTPT"; then
          # Already mounted, probably moved from the
          # initramfs, so remount with the
          # user-specified mount options later on.
          :
      else
        # echo "mount $MOUNTFLAGS -t $FSTYPE $CALLER_OPTS $FSTAB_OPTS $FS_OPTS $DEVNAME $MTPT"
        mount $MOUNTFLAGS -t $FSTYPE $CALLER_OPTS $FSTAB_OPTS $FS_OPTS $DEVNAME $MTPT
      fi
      ;;
    remount)
      if mountpoint -q "$MTPT"; then
        # Remount with user-specified mount options
        # echo "mount $MOUNTFLAGS -oremount $CALLER_OPTS $FSTAB_OPTS $MTPT"
        mount $MOUNTFLAGS -oremount $CALLER_OPTS $FSTAB_OPTS $MTPT
      fi
      ;;
  esac
}

# Mount /run
mount_run ()
{
  MNTMODE="$1"

  # Needed to determine if root is being mounted read-only.
  read_fstab

  #
  # Get some writable area available before the root is checked
  # and remounted.  Note that /run may be handed over from the
  # initramfs.
  #

  # TODO: Add -onodev once checkroot no longer creates a device node.
  # If /run/shm is separately mounted, /run can be safely mounted noexec.
  if read_fstab_entry /run/shm tmpfs; then
    domount "$MNTMODE" tmpfs shmfs /run tmpfs "-onosuid,noexec"
  else
    domount "$MNTMODE" tmpfs shmfs /run tmpfs "-onosuid"
  fi

  # Make pidfile omit directory for sendsigs
  [ -d /run/sendsigs.omit.d ] || mkdir --mode=755 /run/sendsigs.omit.d/
  # Make sure we don't get cleaned
  touch /run/.tmpfs
}

# Mount /run/lock
mount_lock ()
{
  MNTMODE="$1"

  # Make lock directory
  [ -d /run/lock ] || mkdir --mode=755 /run/lock

  # Now check if there's an entry in /etc/fstab.  If there is,
  # it overrides the existing RAMLOCK setting.
  if read_fstab_entry /run/lock; then
      if [ "$MNT_TYPE" = "tmpfs" ] ; then
        domount "$MNTMODE" tmpfs shmfs /run/lock tmpfs "-onosuid,nodev,noexec"
      else
        chmod 1755 /run/lock
      fi
  fi

  # Make sure we don't get cleaned
  touch /run/lock/.tmpfs
}

# Mount /run/shm
mount_shm ()
{
  MNTMODE="$1"

  # Make shm directory
  [ -d /run/shm ] || mkdir --mode=755 /run/shm

  # Now check if there's an entry in /etc/fstab.  If there is,
  # it overrides the existing RAMSHM setting.
  if read_fstab_entry /run/shm; then
    if [ "$MNT_TYPE" = "tmpfs" ] ; then
      domount "$MNTMODE" tmpfs shmfs /run/shm tmpfs "-onosuid,nodev,noexec"
    else
      chmod 755 /run/shm
    fi
  fi

  # Make sure we don't get cleaned
  touch /run/shm/.tmpfs
}

# Mount /tmp
mount_tmp ()
{
  MNTMODE="$1"

  # Make /tmp directory
  rm -rf /tmp
  mkdir --mode=1777 /tmp

  RAMTMP="yes"
  RAM_SIZE="$(ram_size)"
  TMP_SIZE="$(tmp_size)"

  # Disable RAMTMP if there's 64MiB RAM or less.  May be
  # re-enabled by overflow or read only root, below.
  if [ -n "$RAM_SIZE" ] && [ "$RAM_SIZE" -le 65536 ]; then
    RAMTMP=no
  fi

  # Now check if there's an entry in /etc/fstab.  If there is,
  # it overrides all the above settings.
  if read_fstab_entry /tmp; then
    if [ "$MNT_TYPE" = "tmpfs" ] ; then
      RAMTMP="yes"
    else
      RAMTMP="no"
    fi
  fi

  # Mount /tmp as tmpfs if enabled.
  if [ yes = "$RAMTMP" ]; then
    domount "$MNTMODE" tmpfs shmfs /tmp tmpfs "-onodev,nosuid,mode=0777,size=$TMP_SIZE"
    # Make sure we don't get cleaned
    touch /tmp/.tmpfs
  fi
}

