#!/usr/bin/env sh

if [ -n "$FULL" ]; then
    OPK_NAME=sdlretro-full.opk
else
    OPK_NAME=sdlretro.opk
fi

echo ${OPK_NAME}

cp ../build/src/sdlretro .
mipsel-linux-strip sdlretro

if [ -n "$FULL" ] && [ -d cores ]; then
    mipsel-linux-strip cores/*.so
    CORES=cores
fi

# create opk
rm -f ${OPK_NAME}
mksquashfs default.gcw0.desktop sdlretro ../data/sdlretro_icon.png ${CORES} ${OPK_NAME} -all-root -no-xattrs -noappend -no-exports
