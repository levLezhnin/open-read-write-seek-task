#!/bin/bash

FILE="A"
SIZE=$((4 * 1024 * 1024 + 1))

truncate -s $SIZE $FILE

printf '1' | dd of=$FILE bs=1 seek=0 count=1 conv=notrunc 2>/dev/null

printf '1' | dd of=$FILE bs=1 seek=10000 count=1 conv=notrunc 2>/dev/null

printf '1' | dd of=$FILE bs=1 seek=$((SIZE - 1)) count=1 conv=notrunc 2>/dev/null

echo "Файл $FILE успешно создан (размер: $(stat -c %s $FILE) байт)"
