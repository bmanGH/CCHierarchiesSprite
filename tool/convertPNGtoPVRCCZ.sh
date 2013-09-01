#!/bin/sh

echo "==== sprites pack begin ===="
find . -name '*.sprites' -exec ./SpritesPackerMergeCL -padding 5 -maxw 1024 -maxh 1024 -i {} -o {}.png \;
echo "==== sprites pack end ===="

echo "==== alpha dilate begin ===="
find . -name '*.sprites.png' -exec ./AlphaUtilityCL -i {} -o {} \;
echo "==== alpha dilate end ===="

echo "==== convert to pvr begin ===="
find . -name '*.sprites.png' -exec ./PVRTexTool -fOGL8888 -pvrlegacy -yflip 0 -i {} \;
echo "==== convert to pvr end ===="

echo "==== compression pvr begin ===="
find . -name '*.pvr' -exec ./ccz {} \;
echo "==== compression pvr end ===="

echo "==== rename .sprites.pvr.ccz to .pvr.ccz begin ===="
find . -name '*.sprites.pvr.ccz' | while read filename
do
	old_file_name=${filename}
	new_file_name=`echo ${filename} | sed "s/.sprites.pvr.ccz/.pvr.ccz/g"`
	mv "${old_file_name}" "${new_file_name}"
done
echo "==== rename .sprites.pvr.ccz to .pvr.ccz end ===="

echo "==== rename image name in .sprites file begin ===="
find . -name '*.sprites' -exec sed -i .bak -e "s/.sprites.png/.pvr.ccz/" {} \;
echo "==== rename image name in .sprites file end ===="

echo "==== remove unused files begin ===="
find . -name '*.png' -exec rm {} \;
find . -name '*.pvr' -exec rm {} \;
find . -name '*.sprites.bak' -exec rm {} \;
echo "==== remove unused files end ===="
