/* This is a generated file, don't edit */

#define NUM_APPLETS 226

const char applet_names[] ALIGN1 = ""
"[" "\0"
"[[" "\0"
"addgroup" "\0"
"adduser" "\0"
"ar" "\0"
"arping" "\0"
"ash" "\0"
"awk" "\0"
"basename" "\0"
"blkid" "\0"
"bunzip2" "\0"
"bzcat" "\0"
"cat" "\0"
"catv" "\0"
"chattr" "\0"
"chgrp" "\0"
"chmod" "\0"
"chown" "\0"
"chroot" "\0"
"chrt" "\0"
"chvt" "\0"
"cksum" "\0"
"clear" "\0"
"cmp" "\0"
"cp" "\0"
"cpio" "\0"
"crond" "\0"
"crontab" "\0"
"cut" "\0"
"date" "\0"
"dc" "\0"
"dd" "\0"
"deallocvt" "\0"
"delgroup" "\0"
"deluser" "\0"
"devmem" "\0"
"df" "\0"
"diff" "\0"
"dirname" "\0"
"dmesg" "\0"
"dnsd" "\0"
"dnsdomainname" "\0"
"dos2unix" "\0"
"du" "\0"
"dumpkmap" "\0"
"echo" "\0"
"egrep" "\0"
"eject" "\0"
"env" "\0"
"ether-wake" "\0"
"expr" "\0"
"false" "\0"
"fdflush" "\0"
"fdformat" "\0"
"fgrep" "\0"
"find" "\0"
"fold" "\0"
"free" "\0"
"freeramdisk" "\0"
"fsck" "\0"
"fuser" "\0"
"getopt" "\0"
"getty" "\0"
"grep" "\0"
"gunzip" "\0"
"gzip" "\0"
"halt" "\0"
"hdparm" "\0"
"head" "\0"
"hexdump" "\0"
"hostid" "\0"
"hostname" "\0"
"hwclock" "\0"
"id" "\0"
"ifconfig" "\0"
"ifdown" "\0"
"ifup" "\0"
"inetd" "\0"
"init" "\0"
"insmod" "\0"
"install" "\0"
"ip" "\0"
"ipaddr" "\0"
"ipcrm" "\0"
"ipcs" "\0"
"iplink" "\0"
"iproute" "\0"
"iprule" "\0"
"iptunnel" "\0"
"kill" "\0"
"killall" "\0"
"killall5" "\0"
"klogd" "\0"
"last" "\0"
"less" "\0"
"linux32" "\0"
"linux64" "\0"
"linuxrc" "\0"
"ln" "\0"
"loadfont" "\0"
"loadkmap" "\0"
"logger" "\0"
"login" "\0"
"logname" "\0"
"losetup" "\0"
"ls" "\0"
"lsattr" "\0"
"lsmod" "\0"
"lsof" "\0"
"lspci" "\0"
"lsusb" "\0"
"lzcat" "\0"
"lzma" "\0"
"makedevs" "\0"
"md5sum" "\0"
"mdev" "\0"
"mesg" "\0"
"microcom" "\0"
"mkdir" "\0"
"mkfifo" "\0"
"mknod" "\0"
"mkswap" "\0"
"mktemp" "\0"
"modprobe" "\0"
"more" "\0"
"mount" "\0"
"mountpoint" "\0"
"mt" "\0"
"mv" "\0"
"nameif" "\0"
"netstat" "\0"
"nice" "\0"
"nohup" "\0"
"nslookup" "\0"
"od" "\0"
"openvt" "\0"
"passwd" "\0"
"patch" "\0"
"pidof" "\0"
"ping" "\0"
"pipe_progress" "\0"
"pivot_root" "\0"
"poweroff" "\0"
"printenv" "\0"
"printf" "\0"
"ps" "\0"
"pwd" "\0"
"rdate" "\0"
"readlink" "\0"
"readprofile" "\0"
"realpath" "\0"
"reboot" "\0"
"renice" "\0"
"reset" "\0"
"resize" "\0"
"rm" "\0"
"rmdir" "\0"
"rmmod" "\0"
"route" "\0"
"run-parts" "\0"
"runlevel" "\0"
"sed" "\0"
"seq" "\0"
"setarch" "\0"
"setconsole" "\0"
"setkeycodes" "\0"
"setlogcons" "\0"
"setserial" "\0"
"setsid" "\0"
"sh" "\0"
"sha1sum" "\0"
"sha256sum" "\0"
"sha512sum" "\0"
"sleep" "\0"
"sort" "\0"
"start-stop-daemon" "\0"
"strings" "\0"
"stty" "\0"
"su" "\0"
"sulogin" "\0"
"swapoff" "\0"
"swapon" "\0"
"switch_root" "\0"
"sync" "\0"
"sysctl" "\0"
"syslogd" "\0"
"tail" "\0"
"tar" "\0"
"tee" "\0"
"telnet" "\0"
"test" "\0"
"tftp" "\0"
"time" "\0"
"top" "\0"
"touch" "\0"
"tr" "\0"
"traceroute" "\0"
"true" "\0"
"tty" "\0"
"udhcpc" "\0"
"umount" "\0"
"uname" "\0"
"uniq" "\0"
"unix2dos" "\0"
"unlzma" "\0"
"unxz" "\0"
"unzip" "\0"
"uptime" "\0"
"usleep" "\0"
"uudecode" "\0"
"uuencode" "\0"
"vconfig" "\0"
"vi" "\0"
"vlock" "\0"
"watch" "\0"
"watchdog" "\0"
"wc" "\0"
"wget" "\0"
"which" "\0"
"who" "\0"
"whoami" "\0"
"xargs" "\0"
"xz" "\0"
"xzcat" "\0"
"yes" "\0"
"zcat" "\0"
;

#ifndef SKIP_applet_main
int (*const applet_main[])(int argc, char **argv) = {
test_main,
test_main,
addgroup_main,
adduser_main,
ar_main,
arping_main,
ash_main,
awk_main,
basename_main,
blkid_main,
bunzip2_main,
bunzip2_main,
cat_main,
catv_main,
chattr_main,
chgrp_main,
chmod_main,
chown_main,
chroot_main,
chrt_main,
chvt_main,
cksum_main,
clear_main,
cmp_main,
cp_main,
cpio_main,
crond_main,
crontab_main,
cut_main,
date_main,
dc_main,
dd_main,
deallocvt_main,
deluser_main,
deluser_main,
devmem_main,
df_main,
diff_main,
dirname_main,
dmesg_main,
dnsd_main,
hostname_main,
dos2unix_main,
du_main,
dumpkmap_main,
echo_main,
grep_main,
eject_main,
env_main,
ether_wake_main,
expr_main,
false_main,
freeramdisk_main,
fdformat_main,
grep_main,
find_main,
fold_main,
free_main,
freeramdisk_main,
fsck_main,
fuser_main,
getopt_main,
getty_main,
grep_main,
gunzip_main,
gzip_main,
halt_main,
hdparm_main,
head_main,
hexdump_main,
hostid_main,
hostname_main,
hwclock_main,
id_main,
ifconfig_main,
ifupdown_main,
ifupdown_main,
inetd_main,
init_main,
insmod_main,
install_main,
ip_main,
ipaddr_main,
ipcrm_main,
ipcs_main,
iplink_main,
iproute_main,
iprule_main,
iptunnel_main,
kill_main,
kill_main,
kill_main,
klogd_main,
last_main,
less_main,
setarch_main,
setarch_main,
init_main,
ln_main,
loadfont_main,
loadkmap_main,
logger_main,
login_main,
logname_main,
losetup_main,
ls_main,
lsattr_main,
lsmod_main,
lsof_main,
lspci_main,
lsusb_main,
unlzma_main,
unlzma_main,
makedevs_main,
md5_sha1_sum_main,
mdev_main,
mesg_main,
microcom_main,
mkdir_main,
mkfifo_main,
mknod_main,
mkswap_main,
mktemp_main,
modprobe_main,
more_main,
mount_main,
mountpoint_main,
mt_main,
mv_main,
nameif_main,
netstat_main,
nice_main,
nohup_main,
nslookup_main,
od_main,
openvt_main,
passwd_main,
patch_main,
pidof_main,
ping_main,
pipe_progress_main,
pivot_root_main,
halt_main,
printenv_main,
printf_main,
ps_main,
pwd_main,
rdate_main,
readlink_main,
readprofile_main,
realpath_main,
halt_main,
renice_main,
reset_main,
resize_main,
rm_main,
rmdir_main,
rmmod_main,
route_main,
run_parts_main,
runlevel_main,
sed_main,
seq_main,
setarch_main,
setconsole_main,
setkeycodes_main,
setlogcons_main,
setserial_main,
setsid_main,
ash_main,
md5_sha1_sum_main,
md5_sha1_sum_main,
md5_sha1_sum_main,
sleep_main,
sort_main,
start_stop_daemon_main,
strings_main,
stty_main,
su_main,
sulogin_main,
swap_on_off_main,
swap_on_off_main,
switch_root_main,
sync_main,
sysctl_main,
syslogd_main,
tail_main,
tar_main,
tee_main,
telnet_main,
test_main,
tftp_main,
time_main,
top_main,
touch_main,
tr_main,
traceroute_main,
true_main,
tty_main,
udhcpc_main,
umount_main,
uname_main,
uniq_main,
dos2unix_main,
unlzma_main,
unxz_main,
unzip_main,
uptime_main,
usleep_main,
uudecode_main,
uuencode_main,
vconfig_main,
vi_main,
vlock_main,
watch_main,
watchdog_main,
wc_main,
wget_main,
which_main,
who_main,
whoami_main,
xargs_main,
unxz_main,
unxz_main,
yes_main,
gunzip_main,
};
#endif

const uint16_t applet_nameofs[] ALIGN2 = {
0x0000,
0x0002,
0x0005,
0x000e,
0x0016,
0x0019,
0x0020,
0x0024,
0x0028,
0x0031,
0x0037,
0x003f,
0x0045,
0x0049,
0x004e,
0x0055,
0x005b,
0x0061,
0x0067,
0x006e,
0x0073,
0x0078,
0x007e,
0x0084,
0x0088,
0x008b,
0x0090,
0x8096,
0x009e,
0x00a2,
0x00a7,
0x00aa,
0x00ad,
0x00b7,
0x00c0,
0x00c8,
0x00cf,
0x00d2,
0x00d7,
0x00df,
0x00e5,
0x00ea,
0x00f8,
0x0101,
0x0104,
0x010d,
0x0112,
0x0118,
0x011e,
0x0122,
0x012d,
0x0132,
0x0138,
0x0140,
0x0149,
0x014f,
0x0154,
0x0159,
0x015e,
0x016a,
0x016f,
0x0175,
0x017c,
0x0182,
0x0187,
0x018e,
0x0193,
0x0198,
0x019f,
0x01a4,
0x01ac,
0x01b3,
0x01bc,
0x01c4,
0x01c7,
0x01d0,
0x01d7,
0x01dc,
0x01e2,
0x01e7,
0x01ee,
0x01f6,
0x01f9,
0x0200,
0x0206,
0x020b,
0x0212,
0x021a,
0x0221,
0x022a,
0x022f,
0x0237,
0x0240,
0x0246,
0x024b,
0x0250,
0x0258,
0x0260,
0x0268,
0x026b,
0x0274,
0x027d,
0x8284,
0x028a,
0x0292,
0x029a,
0x029d,
0x02a4,
0x02aa,
0x02af,
0x02b5,
0x02bb,
0x02c1,
0x02c6,
0x02cf,
0x02d6,
0x02db,
0x02e0,
0x02e9,
0x02ef,
0x02f6,
0x02fc,
0x0303,
0x030a,
0x0313,
0x4318,
0x031e,
0x0329,
0x032c,
0x032f,
0x0336,
0x033e,
0x0343,
0x0349,
0x0352,
0x0355,
0x835c,
0x0363,
0x0369,
0x436f,
0x0374,
0x0382,
0x038d,
0x0396,
0x039f,
0x03a6,
0x03a9,
0x03ad,
0x03b3,
0x03bc,
0x03c8,
0x03d1,
0x03d8,
0x03df,
0x03e5,
0x03ec,
0x03ef,
0x03f5,
0x03fb,
0x0401,
0x040b,
0x0414,
0x0418,
0x041c,
0x0424,
0x042f,
0x043b,
0x0446,
0x0450,
0x0457,
0x045a,
0x0462,
0x046c,
0x0476,
0x047c,
0x0481,
0x0493,
0x049b,
0x84a0,
0x04a3,
0x04ab,
0x04b3,
0x04ba,
0x04c6,
0x04cb,
0x04d2,
0x04da,
0x04df,
0x04e3,
0x04e7,
0x04ee,
0x04f3,
0x04f8,
0x04fd,
0x0501,
0x0507,
0x450a,
0x0515,
0x051a,
0x051e,
0x0525,
0x052c,
0x0532,
0x0537,
0x0540,
0x0547,
0x054c,
0x0552,
0x0559,
0x0560,
0x0569,
0x0572,
0x057a,
0x857d,
0x0583,
0x0589,
0x0592,
0x0595,
0x059a,
0x05a0,
0x05a4,
0x05ab,
0x05b1,
0x05b4,
0x05ba,
0x05be,
};

const uint8_t applet_install_loc[] ALIGN1 = {
0x33,
0x11,
0x33,
0x31,
0x23,
0x33,
0x11,
0x11,
0x11,
0x34,
0x33,
0x33,
0x11,
0x34,
0x13,
0x13,
0x13,
0x21,
0x31,
0x13,
0x14,
0x33,
0x11,
0x31,
0x33,
0x13,
0x31,
0x31,
0x33,
0x22,
0x13,
0x12,
0x11,
0x22,
0x33,
0x13,
0x32,
0x22,
0x42,
0x22,
0x13,
0x31,
0x13,
0x11,
0x11,
0x33,
0x32,
0x13,
0x01,
0x41,
0x32,
0x31,
0x12,
0x21,
0x33,
0x33,
0x23,
0x23,
0x33,
0x31,
0x21,
0x21,
0x11,
0x11,
0x21,
0x11,
0x33,
0x33,
0x33,
0x11,
0x21,
0x12,
0x13,
0x41,
0x43,
0x23,
0x33,
0x13,
0x21,
0x12,
0x12,
0x13,
0x32,
0x14,
0x13,
0x33,
0x13,
0x23,
0x13,
0x21,
0x22,
0x12,
0x22,
0x13,
0x33,
0x33,
0x33,
0x31,
0x13,
0x23,
0x11,
0x33,
0x33,
0x33,
0x31,
0x23,
0x31,
0x21,
0x33,
0x33,
0x33,
0x33,
0x13,
};

#define MAX_APPLET_NAME_LEN 17
