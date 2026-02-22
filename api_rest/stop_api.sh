#!/bin/bash

echo "Parando API REST de Clínicas Médicas..."
echo ""

echo "Encerrando processos da API..."
pkill -f "uvicorn.*main:app" && echo "Processos da API encerrados" || echo "Nenhum processo da API encontrado"

echo ""
echo "Parando PostgreSQL..."
docker compose down && echo "PostgreSQL parado" || echo "Erro ao parar PostgreSQL"

echo ""
echo "API REST parada com sucesso!"
