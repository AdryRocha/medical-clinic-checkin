#!/bin/bash

echo "Iniciando API REST de Clínicas Médicas..."
echo ""

if ! docker info > /dev/null 2>&1; then
    echo "Docker não está rodando. Por favor, inicie o Docker Desktop e tente novamente."
    exit 1
fi

echo "Iniciando PostgreSQL..."
docker compose up -d postgres

echo "Aguardando PostgreSQL inicializar..."
until docker compose exec -T postgres pg_isready -U postgres > /dev/null 2>&1; do
    echo "Aguardando PostgreSQL..."
    sleep 2
done

sleep 3

echo "PostgreSQL rodando!"
echo ""

echo "Verificando se o banco de dados existe..."
DB_EXISTS=$(docker compose exec -T postgres psql -U postgres -tAc "SELECT 1 FROM pg_database WHERE datname='clinicas_db'")
if [ "$DB_EXISTS" != "1" ]; then
    echo "Criando banco de dados clinicas_db..."
    docker compose exec -T postgres psql -U postgres -c "CREATE DATABASE clinicas_db;"
    echo "Banco de dados criado!"
else
    echo "Banco de dados já existe!"
fi
echo ""

if [ ! -f .env ]; then
    echo "Arquivo .env não encontrado. Criando a partir do .env.example..."
    cp .env.example .env
    echo "Arquivo .env criado!"
fi

if [ ! -d "venv" ]; then
    echo "Criando ambiente virtual..."
    python3 -m venv venv
fi

echo "Ativando ambiente virtual..."
source venv/bin/activate

echo "Instalando dependências..."
pip install -q -r requirements.txt

echo "Populando banco de dados..."
if PYTHONPATH=. python scripts/seed_data.py; then
    echo "Banco de dados populado com sucesso!"
else
    echo "Erro ao popular banco de dados"
fi

echo ""
echo "Iniciando servidor FastAPI..."
echo "Documentação: http://localhost:8000/docs"
echo "ReDoc: http://localhost:8000/redoc"
echo "PostgreSQL: localhost:5432"
echo ""

python run.py
