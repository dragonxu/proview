#!/bin/bash

pwre_help()
{
  cat <<EOF
pwre_configure.h

Arguments

--help           Display help.

--version        State fix version of loadfiles, eg

                 > pwre configure --version "29-MAY-2011 16:00:00"

--reset-version  Reset previous version
--lock-dbs       Lock dbs-files. Dbs-files will not be rebuilt.
--unlock-dbs     Unlock dbs-files
--parallel       Build with parallel execution.
--not-parallel   Don't build with parallel execution

EOF
}

pwre_config_init()
{
    d=`eval date +\"%F %X\"`
    cat > $cfile <<EOF
#!/bin/bash
#
# pwre configuration file
#
# Generated by pwre_configure $d
# Do not edit this file
#
EOF
}

pwre_config_check_include()
{
    let i=0
    let found=0
    let incfound=0
    IFS=:
    for file in $4
    do
        if test -e $file; then
            echo "...Checking   Yes   $1"
            echo "export PWRE_CONF_$2=1" >> $cfile

            if test $3 == "1"; then
                conf_cc_define=$conf_cc_define" -DPWRE_CONF_$2=1"
            fi

            incdir=${file%/*}
            if test $incdir == "/usr/include"; then
                incfound=1
            else
                while [ $i -lt $inc_cnt ]; do
                    if test ${inc_array[$i]} == $incdir; then
                        incfound=1
                    fi
                    i=$((i+1))
                done
            fi

            if test $incfound -eq 0; then
                inc_array[$inc_cnt]=$incdir
                inc_cnt=$((inc_cnt+1))
            fi

            found=1
            break
        fi
    done

    if test $found -eq 0; then
        echo "...Checking   No    $1"
    fi
}

pwre_config_check_tool()
{
    let i=0
    let found=0
    let incfound=0
    IFS=:
    for file in $3
    do
        if test -e $file; then
            echo "...Checking   Yes   $1"
            echo "export PWRE_CONF_$2=1" >> $cfile

            found=1
            break
        fi
    done

    if test $found -eq 0; then
        echo "...Checking   No    $1"
    fi
}

#
# Search for archive and insert in list
#
# Arguments
# 1: archive name
# 2: define name (PWRE_CONF_$2 will be defined)
# 3: special archives for gtk and motif (lib, gtk, motif)
# 4: variable to insert archive in (rt, wb, gtk, motif)
# 5: if 1, add -DPWRE_CONF_$2 to cc command.
# 6: search list for archive.
# Examples
#  pwre_config_check_lib libm LIBM lib lib 0 /usr/lib/libm.so:/usr/lib/libm.a
#  pwre_config_check_lib gtk  GTK  gtk gtk 0 /usr/lib/libgtk-x11-2.0.so
#
pwre_config_check_lib()
{
    let i=0
    let found=0
    let libfound=0
    IFS=:
    for file in $6
    do

        if test -e $file; then
            echo "...Checking   Yes   $1"
            echo "export PWRE_CONF_$2=1" >> $cfile

            if test $5 == "1"; then
                conf_cc_define=$conf_cc_define" -DPWRE_CONF_$2=1"
            fi

            if test $3 == "lib"; then
                libdir=${file%/*}
                if test $libdir == "/usr/lib"; then
                    libfound=1
                else
                    while [ $i -lt $lib_cnt ]; do
                        if test ${lib_array[$i]} == $libdir; then
                            libfound=1
                        fi
                        i=$((i+1))
                    done
                fi

                if test $libfound -eq 0; then
                    if test $4 == "powerlink"; then
                        lib_path=$libdir
                        elif test $4 == "powerlinkcn"; then
                        lib_path=$libdir
                    else
                        lib_array[$lib_cnt]=$libdir
                        lib_cnt=$((lib_cnt+1))
                    fi
                fi

                lib=${file##/*/lib}
                if test $4 == "gtk"; then
                    conf_libgtk=$conf_libgtk" -l${lib%.*}"
		elif test $4 == "motif"; then
                    conf_libmotif=$conf_libmotif" -l${lib%.*}"
		elif test $4 == "qt"; then
                    conf_libqt=$conf_libqt" -l${lib%.*}"
		elif test $4 == "wb"; then
                    conf_libwb=$conf_libwb" -l${lib%.*}"
		elif test $4 == "mq"; then
                    conf_libmq=$conf_libmq" -l${lib%.*}"
                    elif test $4 == "wmq"; then
                    conf_libwmq=$conf_libwmq" -l${lib%.*}"
		elif test $4 == "pnak"; then
                    conf_libpnak=$conf_libpnak" -l${lib%.*}"
		elif test $4 == "libusb"; then
                    conf_lib=$conf_lib" -lusb-1.0"
		elif test $4 == "libpiface"; then
                    conf_lib=$conf_lib" -lpiface-1.0"
		elif test $4 == "powerlink"; then
                    conf_libpowerlink=$conf_libpowerlink" -L$lib_path -l${lib%.*}"
		elif test $4 == "powerlinkcn"; then
                    conf_libpowerlinkcn=$conf_libpowerlinkcn" -L$lib_path -l${lib%.*}"
		elif test $4 == "libpcap"; then
                    conf_libpowerlink=$conf_libpowerlink" -l${lib%.*}"
                    conf_libpowerlinkcn=$conf_libpowerlinkcn" -l${lib%.*}"
                else
                    conf_lib=$conf_lib" -l${lib%%.*}"
                fi
	    elif test $3 == "gtk"; then
                conf_libgtk=$conf_libgtk" \\\`pkg-config --libs gtk+-2.0\\\`"
                conf_incdirgtk=$conf_incdirgtk" \\\`pkg-config --cflags gtk+-2.0\\\`"
	    elif test $3 == "qt"; then
                conf_libqt=$conf_libqt" \\\`pkg-config --libs QtCore QtGui QtNetwork phonon\\\`"
                conf_incdirqt=$conf_incdirqt" \\\`pkg-config --cflags QtCore QtGui QtNetwork phonon\\\`"
	    elif test $3 == "gst"; then
                conf_libgst=$conf_libgst" \\\`pkg-config --libs gstreamer-video-1.0 gstreamer-1.0\\\`"
                conf_incdirgst=$conf_incdirgst" \\\`pkg-config --cflags gstreamer-video-1.0 gstreamer-1.0\\\`"
	    elif test $3 == "motif"; then
                conf_libmotif=$conf_libmotif" -lImlib -lMrm -lXm -lXpm -lXt -lX11 -lXext -lXp -lSM -lICE"
            else
                echo "Unknown type"
            fi

            found=1
            break
        fi
    done

    if test $found -eq 0; then
        echo "...Checking   No    $1"
        echo "export PWRE_CONF_$2=0" >> $cfile
    fi
}

let pwr_conf_qt=0
let ebuild=0
cfile="$pwre_broot/pwre_${pwre_hw:3}_${pwre_os:3}.cnf"
dos=`eval echo ${pwre_os} | tr '[:lower:]' '[:upper:]'`
dhw=`eval echo ${pwre_hw} | tr '[:lower:]' '[:upper:]'`
conf_cc_define="-D$dos=1 -D$dhw=1 -DOS=${pwre_os:3} -DHW=${pwre_hw:3} -D_${dos:3}"
conf_lib=""
conf_libwb=""
conf_libmq=""
conf_libwmq=""
conf_libpnak=""
conf_libgtk=""
conf_libqt=""
conf_libmotif=""
conf_libgst=""
conf_libdir=""
conf_incdirgtk=""
conf_incdirqt=""
conf_incdirgst=""
let inc_cnt=0
let lib_cnt=0
let i=0

hwpl=i386
if [ ${pwre_hw:3} == "arm" ]; then
    hwpl=arm
fi
if [ ${pwre_hw:3} == "x86_64" ]; then
    hwpl=x86_64
fi

# Bash
if [ "$SHELL" != "/bin/bash" ] && [ "$SHELL" != "/usr/local/bin/bash" ]; then
    echo "Config error: Default shell has to be bash"
    exit
fi

echo "...Checking         /bin/bash"
if test ! -e /bin/bash; then
    if test -e /usr/local/bin/bash; then
        echo "Config error: /bin/bash not found"
        echo "  Create link /bin/bash to /usr/local/bin/bash (ln -s /usr/local/bin/bash /bin/bash)"
    else
        echo "Config error: /bin/bash not found"
    fi
    exit
fi

if test "$pwre_broot" == ""; then
    echo "Config error: pwre not initialized"
    exit
fi

if test ! -w $pwre_broot; then
    echo "Config error: pwre build root doesn't exist or is not writable"
    exit
fi

if test ! -e $pwre_croot; then
    echo "Config error: pwre source root doesn't exist"
    exit
fi

# Options
declare -i buildversion_set=0
declare -i lockdbs_set=0
declare -i lockdbs=0
declare -i parallel_set=0
declare -i parallel=0
if [ "$1" = "--help" ]; then
    pwre_help
    exit
elif [ "$1" = "--version" ] && [ "$2" != "" ] && [ "$3" != "" ]; then
    buildversion=$2" "$3
    buildversion_set=1
elif [ "$1" = "--reset-version" ]; then
    buildversion=""
    buildversion_set=1
elif [ "$1" = "--lock-dbs" ]; then
    lockdbs=1
    lockdbs_set=1
elif [ "$1" = "--unlock-dbs" ]; then
    lockdbs=0
    lockdbs_set=1
elif [ "$1" = "--parallel" ]; then
    parallel=1
    parallel_set=1
elif [ "$1" = "--not-parallel" ]; then
    parallel=0
    parallel_set=1
elif [ "$1" = "--ebuild" ]; then
    ebuild=1
elif [ "$1" != "" ]; then
    echo "Unknown option \"$1\""
    exit
fi

if [ $buildversion_set -eq 0 ]; then
    # Catch current version
    if [ -e $cfile ]; then
        ver=`eval cat $cfile | grep "\bexport PWRE_CONF_BUILDVERSION"`
        ver=${ver#*=\"}
        ver=${ver%\"}
        buildversion=$ver
    fi
fi
if [ $lockdbs_set -eq 0 ]; then
    # Catch current version
    if [ -e $cfile ]; then
        ver=`eval cat $cfile | grep "\bexport PWRE_CONF_LOCKDBS"`
        ver=${ver#*=}
        lockdbs=$ver
    fi
fi
if [ $parallel_set -eq 0 ]; then
    # Catch current value
    if [ -e $cfile ]; then
        ver=`eval cat $cfile | grep "\bexport PWRE_CONF_PARALLEL"`
        ver=${ver#*=}
        parallel=$ver
    fi
fi

pwre_config_init

if [ "$buildversion" != "" ]; then
    echo "export PWRE_CONF_BUILDVERSION=\"$buildversion\"" >> $cfile
else
    echo "export PWRE_CONF_BUILDVERSION=\"0\"" >> $cfile
fi
echo "export PWRE_CONF_LOCKDBS=$lockdbs" >> $cfile
echo "export PWRE_CONF_PARALLEL=$parallel" >> $cfile

if [ $pwre_hw == "hw_arm" ] && [ $ebuild -eq 1 ]; then
    echo "Arm ebuild"

#    if [ $pwre_conf_qt -eq 1 ]; then
        pwre_config_check_lib qt        QT      qt qt 0 "/usr/lib/libQtGui.so:/usr/lib/$hwpl-linux-$gnu/libQtGui.so"
        pwre_config_check_include qt    QT   1 "/usr/include/qt4/QtGui"
        pwre_config_check_include qt    QT   1 "/usr/include/qt4/QtCore/QtCore"
        pwre_config_check_include qt    QT   1 "/usr/include/qt4/QtGui/QtGui"
        pwre_config_check_include qt    QT   1 "/usr/include/qt4/QtNetwork/QtNetwork"
#    fi
#    if [ $pwre_conf_gtk -eq 1 ]; then
        pwre_config_check_lib gtk       GTK      gtk gtk 0 "/usr/lib/libgtk-x11-2.0.so:/usr/lib/$hwpl-linux-$gnu/libgtk-x11-2.0.so"
        pwre_config_check_include gtk   GTK   1 "/usr/local/include/gtk-2.0/gtk.h:/usr/local/include/gtk-2.0/gtk/gtk.h:/usr/include/gtk-2.0/gtk/gtk.h"
#    fi

    pwre_config_check_include jni   JNI   1 $jdk/include/jni.h
    pwre_config_check_include jni   JNI   0 $jdk/include/linux/jni_md.h

    let i=0
    while [ $i -lt $inc_cnt ]; do
        conf_incdir=$conf_incdir" -I${inc_array[$i]}"
        i=$((i+1))
    done

    echo "export pwre_conf_cc_define=\"$conf_cc_define\"" >> $cfile
    echo "export pwre_conf_libpwrco=\"-lpwr_co\"" >> $cfile
    echo "export pwre_conf_libpwrrt=\"-lpwr_rt -lpwr_statussrv -lpwr_co -lpwr_msg_dummy\"" >> $cfile
    echo "export pwre_conf_libpwrdtt=\"-lpwr_dtt\"" >> $cfile
    echo "export pwre_conf_libpwrotherio=\"-lpwr_usbio_dummy -lpwr_usb_dummy -lpwr_cifx_dummy\"" >> $cfile
    echo "export pwre_conf_libpwrprofibus=\"-lpwr_pnak_dummy\"" >> $cfile
    echo "export pwre_conf_libpwrxtt=\"-lpwr_xtt -lpwr_ge -lpwr_cow -lpwr_flow -lpwr_glow\"" >> $cfile
    echo "export pwre_conf_libpwrxttgtk=\" -lpwr_xtt_gtk -lpwr_ge_gtk -lpwr_cow_gtk -lpwr_flow_gtk -lpwr_glow_gtk\"" >> $cfile
    echo "export pwre_conf_libpwrxttqt=\" -lpwr_xtt_qt -lpwr_ge_qt -lpwr_cow_qt -lpwr_flow_qt -lpwr_glow_qt\"" >> $cfile
    echo "export pwre_conf_libpwrxttmotif=\" -lpwr_xtt_motif -lpwr_ge_motif -lpwr_cow_motif -lpwr_flow_motif -lpwr_glow_motif\"" >> $cfile
    echo "export pwre_conf_libpwrwb=\"-lpwr_wb\"" >> $cfile
    echo "export pwre_conf_libpwrwbgtk=\"-lpwr_wb_gtk\"" >> $cfile
    echo "export pwre_conf_libpwrwbqt=\"-lpwr_wb_qt\"" >> $cfile
    echo "export pwre_conf_libpwrwbmotif=\"-lpwr_wb_motif\"" >> $cfile
    echo "export pwre_conf_libpwropc=\"-lpwr_opc\"" >> $cfile
    echo "export pwre_conf_libpwrremote=\"-lpwr_remote\"" >> $cfile
    echo "export pwre_conf_libpwrnmps=\"-lpwr_nmps\"" >> $cfile
    echo "export pwre_conf_libpwrtlog=\"-lpwr_tlog\"" >> $cfile
    echo "export pwre_conf_libpwrsev=\"-lpwr_sev\"" >> $cfile
    echo "export pwre_conf_lib=\"-lpthread -lm -lrt -lcrypt\"" >> $cfile
    echo "export pwre_conf_libwb=\"$conf_libwb\"" >> $cfile
    echo "export pwre_conf_libmq=\"$conf_libmq\"" >> $cfile
    echo "export pwre_conf_libwmq=\"$conf_libwmq\"" >> $cfile
    echo "export pwre_conf_libpnak=\"$conf_libpnak\"" >> $cfile
    echo "export pwre_conf_libgtk=\"$conf_libgtk\"" >> $cfile
    echo "export pwre_conf_libqt=\"$conf_libqt\"" >> $cfile
    echo "export pwre_conf_libmotif=\"$conf_libmotif\"" >> $cfile
    echo "export pwre_conf_libgst=\"$conf_libgtk\"" >> $cfile
    echo "export pwre_conf_libdir=\"$conf_libdir\"" >> $cfile
    echo "export pwre_conf_incdir=\"$conf_incdir\"" >> $cfile
    echo "export pwre_conf_incdirgtk=\"$conf_incdirgtk\"" >> $cfile
    echo "export pwre_conf_incdirqt=\"$conf_incdirqt\"" >> $cfile
    echo "export pwre_conf_dtt_platform=\"arm_linux\"" >> $cfile
else

    if [ $pwre_hw == "hw_arm" ]; then
        gnu=gnueabihf
    else
        gnu=gnu
    fi

    #Gtk
    echo "Mandatory :"

    pwre_config_check_lib librpcsvc LIBRPCSVC lib lib 0 "/usr/lib/librpcsvc.so:/usr/lib/librpcsvc.a:/usr/lib/$hwpl-linux-$gnu/librpcsvc.a"
    pwre_config_check_lib libasound LIBASOUND lib lib 0 "/usr/lib/libasound.so:/usr/lib/libasound.a:/usr/lib/$hwpl-linux-$gnu/libasound.so"
    pwre_config_check_lib libpthread LIBPTHREAD lib lib 0 "/usr/lib/libpthread.so:/usr/lib/libpthread.a:/usr/lib/$hwpl-linux-$gnu/libpthread.so"
    pwre_config_check_lib libm      LIBM     lib lib 0 "/usr/lib/libm.so:/usr/lib/libm.a:/usr/lib/$hwpl-linux-$gnu/libm.so"
    pwre_config_check_lib libdb     LIBDB    lib lib 1 "/usr/lib/libdb.so:/usr/lib/$hwpl-linux-$gnu/libdb.so"
    pwre_config_check_lib libdb_cxx LIBDB_CXX lib wb 1 "/usr/lib/libdb_cxx.so:/usr/lib/$hwpl-linux-$gnu/libdb_cxx.so"
    pwre_config_check_lib libz      LIBZ     lib lib 0 "/usr/lib/libz.so:/usr/lib/libz.a:/usr/lib/$hwpl-linux-$gnu/libz.so"
    pwre_config_check_lib libcrypt  LIBCRYPT lib lib 0 "/usr/lib/libcrypt.so:/usr/lib/libcrypt.a:/usr/lib/$hwpl-linux-$gnu/libcrypt.so"
    pwre_config_check_lib librt     LIBRT    lib lib 0 "/usr/lib/librt.so:/usr/lib/librt.a:/usr/lib/$hwpl-linux-$gnu/librt.so"
    pwre_config_check_lib libX11    LIBX11   lib lib 0 "/usr/lib/libX11.so:/usr/lib/$hwpl-linux-$gnu/libX11.so"
    pwre_config_check_include alsa  ALSA  1 "/usr/include/alsa/asoundlib.h"

    echo ""
    echo "Gui Qt/Gtk:"
    if [ ! -z $pwre_conf_qt ]; then
        pwre_config_check_lib qt        QT      qt qt 0 "/usr/lib/libQtGui.so:/usr/lib/$hwpl-linux-$gnu/libQtGui.so"
        pwre_config_check_include qt    QT   1 "/usr/include/qt4/QtGui"
        pwre_config_check_include qt    QT   1 "/usr/include/qt4/QtCore/QtCore"
        pwre_config_check_include qt    QT   1 "/usr/include/qt4/QtGui/QtGui"
        pwre_config_check_include qt    QT   1 "/usr/include/qt4/QtNetwork/QtNetwork"
    else
        pwre_config_check_lib gtk       GTK      gtk gtk 0 "/usr/lib/libgtk-x11-2.0.so:/usr/lib/$hwpl-linux-$gnu/libgtk-x11-2.0.so"
        pwre_config_check_include gtk   GTK   1 "/usr/local/include/gtk-2.0/gtk.h:/usr/local/include/gtk-2.0/gtk/gtk.h:/usr/include/gtk-2.0/gtk/gtk.h"
    fi

    echo ""
    echo "Optional :"
    pwre_config_check_include jni   JNI   1 "$jdk/include/jni.h"
    pwre_config_check_include jni   JNI   0 "$jdk/include/linux/jni_md.h"
    pwre_config_check_lib motif     MRM      motif motif 0 "/usr/lib/libMrm.so"
    pwre_config_check_lib mysql     MYSQL    lib lib 1 "/usr/lib/libmysqlclient.so:/usr/lib/mysql/libmysqlclient.so:/usr/lib/$hwpl-linux-$gnu/libmysqlclient.so"
    pwre_config_check_include sqlite3 SQLITE3   1 "/usr/include/sqlite3.h"
    pwre_config_check_lib libsqlite3   LIBSQLITE3  lib lib 0 "/usr/lib/libsqlite3.so:/usr/lib/$hwpl-linux-$gnu/libsqlite3.so"
    pwre_config_check_include hdf5  HDF5   1 "/usr/lib/openmpi/include/mpi.h:/usr/lib/$hwpl-linux-$gnu/openmpi/include/mpi.h"
    pwre_config_check_include hdf5  HDF5   1 "/usr/include/hdf5/openmpi/hdf5.h:/usr/lib/$hwpl-linux-$gnu/hdf5/openmpi/include/hdf5.h"
    pwre_config_check_lib libhdf5   LIBHDF5  lib lib 0 "/usr/lib/libhdf5.so:/usr/lib/$hwpl-linux-$gnu/libhdf5_openmpi.so:/usr/lib/$hwpl-linux-$gnu/libmpi.so.20"
    pwre_config_check_lib libhdf5   LIBHDF5  lib lib 0 "/usr/lib/$hwpl-linux-$gnu/libmpi.so.20"
    pwre_config_check_lib rabbitmq  RABBITMQ lib rabbitmq  1 "/usr/lib/$hwpl-linux-$gnu/librabbitmq.so"
    pwre_config_check_lib mq        MQ       lib mq  1 "/usr/lib/libdmq.so:/usr/local/dmq/lib/libdmq.so:/usr/local/lib/libdmq.so"
    pwre_config_check_lib wmq       WMQ      lib wmq 1 "/usr/lib/libmqic.so"
    pwre_config_check_lib libprofinet PNAK     lib pnak 1 "/usr/lib/libprofinet.a"
    pwre_config_check_lib libusb    LIBUSB   lib libusb 1 "/usr/lib/libusb-1.0.so:/usr/lib/$hwpl-linux-$gnu/libusb-1.0.so"
    pwre_config_check_lib powerlink POWERLINK lib powerlink 1 "$epl/build/Examples/X86/Generic/powerlink_user_lib/libpowerlink.a"
    pwre_config_check_lib powerlinkcn POWERLINKCN lib powerlinkcn 1 "$epl/buildcn/Examples/X86/Generic/powerlink_user_lib/libpowerlink.a"
    pwre_config_check_lib libpcap   LIBPCAP  lib libpcap 1 "/usr/lib/libpcap.so:/usr/lib/$hwpl-linux-$gnu/libpcap.so"
    pwre_config_check_lib librsvg   LIBRSVG  lib librsvg 1 "/usr/lib/librsvg-2.so:/usr/lib/$hwpl-linux-$gnu/librsvg-2.so"
    pwre_config_check_include gst   GST   1 "/usr/include/gstreamer-1.0/gst/gst.h:/opt/gstreamer-sdk/include/gstreamer-1.0/gst/gst.h"
    pwre_config_check_lib gst    	  GST      gst gst 0 "/usr/lib/$hwpl-linux-$gnu/libgstreamer-1.0.so:/opt/gstreamer-sdk/lib/libgstreamer-1.0.so:/usr/lib/libgstreamer-1.0.so:/usr/lib/$hwpl-linux-$gnu/libgstreamer-1.0.so:/opt/gstreamer-sdk/lib/libgstreamer-0.10.so:/usr/lib/libgstreamer-0.10.so"
    if [ $pwre_hw == "hw_arm" ]; then
        pwre_config_check_lib libpiface LIBPIFACE lib libpiface 1 "/usr/local/lib/libpiface-1.0.a"
        pwre_config_check_include piface  PIFACE  1 "/usr/local/include/libpiface-1.0/pfio.h"
    fi

    pwre_config_check_include mq    MQ    0 "/usr/local/dmq/include/p_entry.h:/usr/local/include/p_entry.h"
    pwre_config_check_include wmq   WMQ   1 "/opt/mqm/inc/cmqc.h"
    pwre_config_check_include cifx  CIFX  1 "/usr/local/include/cifx/cifxlinux.h"
    pwre_config_check_include nodave NODAVE 1 "/usr/include/nodave.h"
    pwre_config_check_include powerlink EPL 1 "$epl/Include/Epl.h"
    pwre_config_check_include powerlinkuser EPLU 0 "$epl/Examples/X86/Generic/powerlink_user_lib/EplCfg.h"
    pwre_config_check_include rsvg  RSVG  1 "/usr/include/librsvg-2/librsvg/rsvg.h:/usr/include/librsvg-2.0/librsvg/rsvg.h"
    pwre_config_check_tool android ANDROID "/usr/local/android-sdk-linux/tools/android"


    export pwre_conf_alsa=1


    let i=0
    while [ $i -lt $inc_cnt ]; do
        conf_incdir=$conf_incdir" -I${inc_array[$i]}"
        i=$((i+1))
    done

    let i=0
    while [ $i -lt $lib_cnt ]; do
        conf_libdir=$conf_libdir" -L${lib_array[$i]}"
        i=$((i+1))
    done

    echo "export pwre_conf_cc_define=\"$conf_cc_define\"" >> $cfile
    echo "export pwre_conf_libpwrco=\"-lpwr_co\"" >> $cfile
    echo "export pwre_conf_libpwrrt=\"-lpwr_rt -lpwr_statussrv -lpwr_co -lpwr_msg_dummy\"" >> $cfile
    echo "export pwre_conf_libpwrdtt=\"-lpwr_dtt\"" >> $cfile
    echo "export pwre_conf_libpwrotherio=\"-lpwr_usbio_dummy -lpwr_usb_dummy -lpwr_cifx_dummy -lpwr_nodave_dummy -lpwr_epl_dummy\"" >> $cfile
    echo "export pwre_conf_libpwrprofibus=\"-lpwr_pnak_dummy\"" >> $cfile
    echo "export pwre_conf_libpwrpowerlink=\"$conf_libpowerlink\"" >> $cfile
    echo "export pwre_conf_libpwrpowerlinkcn=\"$conf_libpowerlinkcn\"" >> $cfile
    echo "export pwre_conf_libpwrxtt=\"-lpwr_xtt -lpwr_ge -lpwr_cow -lpwr_flow -lpwr_glow\"" >> $cfile
    echo "export pwre_conf_libpwrxttgtk=\" -lpwr_xtt_gtk -lpwr_ge_gtk -lpwr_cow_gtk -lpwr_flow_gtk -lpwr_glow_gtk\"" >> $cfile
    echo "export pwre_conf_libpwrxttqt=\" -lpwr_xtt_qt -lpwr_ge_qt -lpwr_cow_qt -lpwr_flow_qt -lpwr_glow_qt\"" >> $cfile
    echo "export pwre_conf_libpwrxttmotif=\" -lpwr_xtt_motif -lpwr_ge_motif -lpwr_cow_motif -lpwr_flow_motif -lpwr_glow_motif\"" >> $cfile
    echo "export pwre_conf_libpwrwb=\"-lpwr_wb\"" >> $cfile
    echo "export pwre_conf_libpwrwbgtk=\"-lpwr_wb_gtk\"" >> $cfile
    echo "export pwre_conf_libpwrwbqt=\"-lpwr_wb_qt\"" >> $cfile
    echo "export pwre_conf_libpwrwbmotif=\"-lpwr_wb_motif\"" >> $cfile
    echo "export pwre_conf_libpwropc=\"-lpwr_opc\"" >> $cfile
    echo "export pwre_conf_libpwrremote=\"-lpwr_remote\"" >> $cfile
    echo "export pwre_conf_libpwrnmps=\"-lpwr_nmps\"" >> $cfile
    echo "export pwre_conf_libpwrtlog=\"-lpwr_tlog\"" >> $cfile
    echo "export pwre_conf_libpwrsev=\"-lpwr_sev\"" >> $cfile
    echo "export pwre_conf_lib=\"$conf_lib\"" >> $cfile
    echo "export pwre_conf_libwb=\"$conf_libwb\"" >> $cfile
    echo "export pwre_conf_libmq=\"$conf_libmq\"" >> $cfile
    echo "export pwre_conf_libwmq=\"$conf_libwmq\"" >> $cfile
    echo "export pwre_conf_libpnak=\"$conf_libpnak\"" >> $cfile
    echo "export pwre_conf_libgtk=\"$conf_libgtk\"" >> $cfile
    echo "export pwre_conf_libqt=\"$conf_libqt\"" >> $cfile
    echo "export pwre_conf_libgst=\"$conf_libgst\"" >> $cfile
    echo "export pwre_conf_libmotif=\"$conf_libmotif\"" >> $cfile
    echo "export pwre_conf_libdir=\"$conf_libdir\"" >> $cfile
    echo "export pwre_conf_incdir=\"$conf_incdir\"" >> $cfile
    echo "export pwre_conf_incdirgtk=\"$conf_incdirgtk\"" >> $cfile
    echo "export pwre_conf_incdirqt=\"$conf_incdirqt\"" >> $cfile
    echo "export pwre_conf_incdirgst=\"$conf_incdirgst\"" >> $cfile

fi