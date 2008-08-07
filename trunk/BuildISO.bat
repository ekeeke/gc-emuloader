copy emupack.dol bootdisc
cd pcutils/
mkisofs -R -J -G .\gbi_USA.hdr -no-emul-boot -b emupack.dol -o ../emudisc.iso ../bootdisc
cd ../bootdisc
del emupack.dol
