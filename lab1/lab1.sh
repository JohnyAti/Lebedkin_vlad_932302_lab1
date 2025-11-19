#!/bin/sh


source_file="$1"
temp_dir=$(mktemp -d)
exitup_done=false

exitup() 
{
    if [ "$exitup_done" = "false" ]; then
        exitup_done=true
        if [ -n "$temp_dir" ] && [ -d "$temp_dir" ]; then
            echo "удаление временного каталога $temp_dir"
            rm -rf "$temp_dir"
        fi
        if [ -n "$1" ]; then
            echo "Скрипт прерван  $1"
        fi
    fi
}

trap 'exitup "INT"' INT    
trap 'exitup "TERM"' TERM  
trap 'exitup "HUP"' HUP    
trap 'exitup "EXIT"' EXIT            



if [ ! -d "$temp_dir" ]; then
    echo "Не удалось создать временный каталог"
    exit 1
fi

if [ ! -f "$source_file" ]; then
    echo "Файл $source_file не существует"
    exit 2
else 
    echo "Файл $source_file существует"
fi

output_name=$(grep '^[[:space:]]*//[[:space:]]*Output:[[:space:]]*[[:alnum:]_]*' "$source_file" | head -1 | sed 's/^.*Output:[[:space:]]*\([[:alnum:]_]*\).*$/\1/')

if [ -z "$output_name" ]; then
    output_name=$(grep '/\*[[:space:]]*Output:[[:space:]]*[[:alnum:]_]*' "$source_file" | head -1 | sed 's/^.*Output:[[:space:]]*\([[:alnum:]_]*\).*$/\1/')
fi


if [ -z "$output_name" ]; then
    
    output_name="output"
    echo "Имя выходного файла не указано : $output_name"
else
    echo "Найдено имя выходного файла : $output_name"
fi


origin_dir=$(pwd)
cd "$temp_dir"

if g++ "$origin_dir/$source_file" -o "$output_name"; then
    echo "Сборка  завершена"

    cp "$temp_dir/$output_name" "$origin_dir"
else
    echo "Ошибка компиляции"
    exit 4
fi

exit 0