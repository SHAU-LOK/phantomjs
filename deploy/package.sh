#!/usr/bin/env bash

#
# usage: just run this script (after having run build.sh)
#        and deploy the created tarball to your target machine.
#
# It creates a chromessjs-$version folder and copies the binary,
# example, license etc. together with all shared library dependencies
# to that folder. Furthermore brandelf is used to make the lib
# and binary compatible with older unix/linux machines that don't
# know the new Linux ELF ABI.
#

cd $(dirname $0)

if [[ ! -f ../bin/chromessjs ]]; then
    echo "chromessjs was not built yet, please run build.sh first"
    exit 1
fi

if [[ "$1" = "--bundle-libs" ]]; then
    bundle_libs=1
else
    bundle_libs=0
fi

version=$(../bin/chromessjs --version | sed 's/ /-/' | sed 's/[()]//g')
src=..

echo "packaging chromessjs $version"

if [[ $OSTYPE = darwin* ]]; then
    dest="chromessjs-$version-macosx"
else
    dest="chromessjs-$version-linux-$(uname -m)"
fi

rm -Rf $dest{.tar.bz2,} &> /dev/null
mkdir -p $dest/bin

echo

echo -n "copying files..."
cp $src/bin/chromessjs $dest/bin
cp -r $src/{ChangeLog,examples,LICENSE.BSD,third-party.txt,README.md} $dest/
echo "done"
echo

chromessjs=$dest/bin/chromessjs

if [[ "$bundle_libs" = "1" ]]; then
    mkdir -p $dest/lib

    if [[ ! -f brandelf ]]; then
        echo
        echo "brandelf executable not found in current dir"
        echo -n "compiling it now..."
        g++ brandelf.c -o brandelf || exit 1
        echo "done"
    fi

    libs=$(ldd $chromessjs | egrep -o "/[^ ]+ ")

    echo -n "copying shared libs..."
    libld=
    for l in $libs; do
        ll=$(basename $l)
        cp $l $dest/lib/$ll

        if [[ "$bundle_libs" = "1" ]]; then
            # ensure OS ABI compatibility
            ./brandelf -t SVR4 $dest/lib/$ll
            if [[ "$l" == *"ld-linux"* ]]; then
                libld=$ll
            fi
        fi
    done
    echo "done"
    echo

    echo -n "writing run script..."
    mv $chromessjs $chromessjs.bin
    chromessjs=$chromessjs.bin
    run=$dest/bin/chromessjs
    echo '#!/bin/sh' >> $run
    echo 'path=$(dirname $(dirname $(readlink -f $0)))' >> $run
    echo 'export LD_LIBRARY_PATH=$path/lib' >> $run
    echo 'exec $path/lib/'$libld' $chromessjs $@' >> $run
    chmod +x $run
    echo "done"
    echo
fi

echo -n "stripping binary and libs..."
if [[ $OSTYPE = darwin* ]]; then
    strip -x $chromessjs
else
    strip -s $chromessjs
    [[ -d $dest/lib ]] && strip -s $dest/lib/*
fi
echo "done"
echo

echo -n "compressing binary..."
if type upx >/dev/null 2>&1; then
    upx -qqq -9 $chromessjs
    echo "done"
else
    echo "upx not found"
fi
echo

echo -n "creating archive..."
if [[ $OSTYPE = darwin* ]]; then
    zip -r $dest.zip $dest
else
    tar -cjf $dest{.tar.bz2,}
fi
echo "done"
echo
