#! /bin/bash	
# split one file.L1 to every cores

#from gpu-sim (app.result) file to L1 address file(app.L1)  , new 1 file
grep "@l1_cache" app.result| tee app.L1

#from app.L1 to core_[0..14].addr (only addr ,without the type,length information), new 15 files
for core in {0..14}
do
	echo "curent : $core"
	awk '$2=="'"${core}"'" {print $3}' app.L1 | tee "core_${core}.ADDRonly"
done

# using dis.out to get the RD of every core_?.addr. new 15 files 
for core in {0..14}
do
	# the ?.RD file contain "addr, set , rd" message
	./dis5 "core_${core}.ADDRonly" "core_${core}.RD" hit_rate
	# get the pure RD for fingure   . new 1 file 	
	cut -f 4 "core_${core}.RD" | tee "core_${core}.RDonly"
	if [ -e "dir_core_${core}" ]
	then
		echo "dir_core_${core} is exist. "
		rm -r "dir_core_${core}"
		mkdir "dir_core_${core}"
	else
		mkdir "dir_core_${core}"
	fi
	# new 32 files for every set 
	for set in {0..31}
	do
		awk ' $3 == "'"${set}"'" {print $4}' "core_${core}.RD" | tee  "./dir_core_${core}/set_${set}.RDonly"
	done 
done

source ./getCount.sh
