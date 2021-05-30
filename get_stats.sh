#!/usr/bin/env bash
declare -a array
i=0
write_bytes=0
read_bytes=0
write_files=0
read_files=0
gone=0
while IFS= read -r line; do
    one=`echo $line | cut -d':' -f1`    #split the line in :
    two=`echo $line | cut -d':' -f2`
    if [ $one -eq 1 ]; then
        array[$i]=$two
        let i=i+1

    fi
   if [ $one -eq 2 ]; then
        let write_bytes=write_bytes+two
    fi

    if [ $one -eq 3 ]; then
        let read_bytes=read_bytes+two
   fi

    if [ $one -eq 4 ]; then
        let write_files=write_files+two
   fi

   if [ $one -eq 5 ]; then
        let read_files=read_files+two
   fi

   if [ $one -eq 6 ]; then
        let gone=gone+two
   fi
done
flag=0
for i in ${array[@]} ; do   #for every directory we created
    if [ $flag -eq 0 ]; then
        min=$i                  #initialize in the  first repetition
        max=$i
     fi
     flag=1
     if [ $i -lt $min ]; then
        min=$i
     fi
     if [ $i -gt $max ]; then
        max=$i
     fi
    done
echo "The list of id's is :"
echo ${array[*]}
echo "Min ID value:"$min
echo "Max ID value:"$max
echo "Bytes written:"$write_bytes
echo "Bytes read:"$read_bytes
echo "Files written:"$write_files
echo "Files read:"$read_files
echo "Clients disconnected:"$gone
