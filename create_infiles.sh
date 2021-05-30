#!/usr/bin/env bash
if [ "$#" -ne 4 ]
	then echo "Give 4 arguments"
	exit 1
fi
if [ "$2" -lt 0 ]
	then echo Please give positive numbers
	exit 1
fi
if [ "$3" -lt 0 ]
	then echo Please give positive numbers
	exit 1
fi
if [ "$4" -lt 0 ]
	then echo Please give positive numbers
	exit 1
fi
if [ ! -d "$1" ]; then      #check if directory exists
  mkdir -p "$1";
fi

num_of_dirs=$3
levels=$4
i=0
declare -a array
while [  $num_of_dirs -gt 0 ]; do
    number=$(( ( RANDOM % 8 )  + 1 ))   #generate a random number between 1 and 8
    random=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w $number | head -n 1)       #generate a random alphanumerical name
    path="$1/$random/"          #create path name
    array[$i]=$path
    let i=i+1

    mkdir -p $path;             #create directory

    previous=$path
    let num_of_dirs=num_of_dirs-1
    if [ $num_of_dirs -eq 0 ]
        then
        break
      fi

    for (( j = 1; j < levels; ++j ))        #create the next directories at the path of the previous directory
    do
      random=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w $number | head -n 1)       #generate a random alphanumerical name
      path="$previous$random/"
      array[$i]=$path        #create path name
      let i=i+1

      mkdir -p $path;        #create directory
      previous=$path
      let num_of_dirs=num_of_dirs-1
      if [ $num_of_dirs -eq 0 ]
        then
        break
      fi
    done
done

echo "[OK] creation of directories."
echo "The directories are:"
echo ${array[*]}
directory="$1"
num_of_files=$2

while [  $num_of_files -gt 0 ]
 do
        #so the first file is stored to dir_name/
        number=$(( ( RANDOM % 8 )  + 1 ))   #generate a random number between 1 and 8
        random=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w $number | head -n 1)       #generate a random alphanumerical name
        n=$(( ( RANDOM % 131072 )  + 1 ))                                                   #128 KB == 131072 Bytes
        data=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w $n | head -n 1)              #input of the file
        path=""$1/"$random"
        echo >> $path           #create file
        echo $data > $path       #copy the input to file
        let "num_of_files=num_of_files-1"
        if [ $num_of_files -eq 0 ]
        then
            break
         fi
    for i in ${array[@]} ; do   #for every directory we created
        number=$(( ( RANDOM % 8 )  + 1 ))   #generate a random number between 1 and 8
        random=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w $number | head -n 1)       #generate a random alphanumerical name
        n=$(( ( RANDOM % 131072 )  + 1 ))                                                   #128 KB == 131072 Bytes
        data=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w $n | head -n 1)              #input of the file
        path=""$i"$random"
        echo >> $path           #create file
        echo $data > $path      #copy the input to file
        let "num_of_files=num_of_files-1"
        if [ $num_of_files -eq 0 ]
        then
            break
         fi
    done

done
echo "[OK] creation of files."