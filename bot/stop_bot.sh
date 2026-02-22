#!/bin/bash

echo "Parando Bot do Telegram..."
echo ""

if pkill -f "telegram_bot.py"; then
    echo "Bot encerrado com sucesso"
else
    echo "Nenhum processo do bot encontrado"
fi

echo ""
echo "Bot parado!"
