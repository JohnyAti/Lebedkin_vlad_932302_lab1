#!/bin/sh
container_code=$HOSTNAME
counter=1
shared_dir=/shared_volume
mkdir -p $shared_dir

lock_file=$shared_dir/.lockfile
touch $lock_file

exec 9>"$lock_file"

while :; do
    created=""
    
    flock -x 9
    for name in @21 @22 @23 @24 @25; do
        path=$shared_dir/$name
        if [ ! -e "$path" ]; then
            echo "$container_code $counter" > "$path"         
            counter=$((counter+1))
            created="$name"
            break
        fi
    done
    flock -u 9
    
    if [ -n "$created" ]; then
        sleep 1
        if [ -f "$shared_dir/$created" ]; then
            if head -n 1 "$shared_dir/$created" 2>/dev/null | grep -q "^$container_code "; then
                rm "$shared_dir/$created"
            fi
        fi
        sleep 1
    else
        sleep 2
    fi
done