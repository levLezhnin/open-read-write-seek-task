#!/bin/bash
set -e

echo "0) Очищаю файлы..."
rm -rf build myprogram A B C D *.gz

echo "1) Собираю программу..."
rm -rf build myprogram
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cp build/myprogram ./

echo "2) Создаю тестовый файл 'A'..."
bash test_file.sh

echo "3) Копирую A -> B (разреженный)"
./myprogram A B

echo "4) Сжимаю A и B"
gzip -c A > A.gz
gzip -c B > B.gz

echo "5) Распаковываю B.gz в stdout -> C (через stdin)"
gzip -cd B.gz | ./myprogram C

echo "6) Восстанавливаю A из архива для следующего теста"
gzip -cd A.gz > A

echo "7) Копирую A -> D с блоком 100 байт"
./myprogram -b 100 A D

echo "--- Статистика файлов ---"
STAT_OUT=""
for f in A A.gz B B.gz C D; do
    if [ -f "$f" ]; then
        SIZE=$(stat -c %s "$f")
        BLOCKS=$(stat -c %b "$f")
        REAL=$((BLOCKS * 512))
        STAT_OUT="${STAT_OUT}$f: логический_размер=${SIZE}_байт, выделено_блоков=${BLOCKS} (реальный_размер=${REAL}_байт)"$'\n'
    fi
done
echo "$STAT_OUT"
