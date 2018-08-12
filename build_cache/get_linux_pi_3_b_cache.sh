#!/bin/sh

LINUX_CACHE=$(git ls-tree HEAD linux-4.14.22 | awk '{print "build_cache/"$3"_build_pi_b_plus_linux.tar.gz"}')
if [ -e $LINUX_CACHE ]; then
	echo "found $LINUX_CACHE , reuse it"
	tar -xvf $LINUX_CACHE
	rm -rf build/pi_3_b/linux
	mv build/pi_b_plus/linux build/pi_3_b/
	rm -rf build/pi_3_b/linux_mod
	mv build/pi_b_plus/linux_mod build/pi_3_b/
	rm -rf build/pi_b_plus
	exit 0;
else
	echo "not found $LINUX_CACHE"
	exit 1;
fi
