#!/bin/sh
# Первый аргумент вызова - название docker volume
# Второй аргумент - количество одновременно запущенных контейнеров

vol_name=$1
container_num=$2

docker volume create $vol_name 2>/dev/null || true

exit_handler()
{
    echo "Остановка контейнеров"
    return_code=$?
    trap - EXIT INT HUP QUIT TERM
    i=1
    while [ $i -le $container_num ]; do
        docker stop "container_$i" 2>/dev/null || true
        i=$((i+1))
    done
    exit $return_code
}

trap exit_handler INT HUP QUIT TERM EXIT

docker build -t lab2_unix . || exit 1

i=1
while [ $i -le $container_num ]; do
    docker run -d --rm \
        --name "container_$i" \
        --volume $vol_name:/shared_volume \
        lab2_unix || exit 2
    i=$((i+1))
done

echo "Запущено $container_num контейнеров. Нажмите Ctrl+C для остановки."

while true; do
    sleep 3600
done