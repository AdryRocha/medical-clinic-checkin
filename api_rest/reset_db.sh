#!/bin/bash

echo "Apagando e recriando o banco de dados..."

docker exec clinicas_postgres psql -U postgres -c "DROP DATABASE IF EXISTS clinicas_db;"

echo "Banco resetado! Execute ./start_api.sh para recriar as tabelas."
