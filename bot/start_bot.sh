#!/bin/bash

echo "Iniciando Bot do Telegram..."
echo ""

if [ ! -f ".env" ]; then
    echo "Arquivo .env não encontrado. Criando a partir do .env.example..."
    cp .env.example .env
    echo "Arquivo .env criado!"
    echo "Por favor, edite o arquivo .env com suas configurações (TELEGRAM_TOKEN, API_BASE_URL, etc.)"
    exit 1
fi

if ! grep -q "^TELEGRAM_TOKEN=.\\+" .env; then
    echo "Erro: TELEGRAM_TOKEN não configurado no .env"
    echo "Por favor, edite o arquivo .env e adicione seu token do Telegram"
    exit 1
fi

echo "Configuração verificada"
echo ""

if [ ! -d "venv" ]; then
    echo "Criando ambiente virtual..."
    python3 -m venv venv
fi

echo "Ativando ambiente virtual..."
source venv/bin/activate

echo "Instalando dependências..."
pip install -q -r requirements.txt

echo ""
echo "Iniciando bot..."
echo ""
python src/telegram_bot.py
