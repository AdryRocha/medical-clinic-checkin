# 🏥 API REST - Sistema de Clínicas Médicas

API REST desenvolvida com FastAPI para gerenciamento de clínicas médicas, incluindo agendamento de consultas, gestão de pacientes e profissionais, e geração de QR Code para check-in.

## 🚀 Funcionalidades

- Gestão de pacientes (cadastro e busca por CPF)
- Upload e download de biometria (impressão digital .dat)
- Categorias e especialidades médicas
- Cadastro e listagem de profissionais de saúde
- Gerenciamento de horários de atendimento dos profissionais
- Agendamento de consultas com verificação de horários disponíveis
- Geração e validação de QR Code para check-in
- Autenticação JWT
- Documentação automática (Swagger UI e ReDoc)

## 📋 Endpoints Principais

### Autenticação
- `POST /auth/token` - Obter token JWT

### Pacientes
- `GET /pacientes/cpf/{cpf}` - Buscar paciente por CPF
- `POST /pacientes` - Criar novo paciente
- `GET /pacientes` - Listar todos os pacientes
- `GET /pacientes/{paciente_id}` - Obter paciente por ID
- `POST /pacientes/{paciente_id}/upload-fingerprint` - Upload de biometria (.dat)
- `GET /pacientes/{paciente_id}/fingerprint` - Download de biometria (.dat)

### Categorias
- `GET /categorias` - Listar categorias médicas
- `POST /categorias` - Criar nova categoria

### Profissionais
- `GET /profissionais` - Listar profissionais
- `POST /profissionais` - Criar novo profissional
- `GET /profissionais/{profissional_id}` - Obter profissional por ID

### Consultas
- `GET /horarios/disponiveis/{profissional_id}/{data}` - Verificar horários disponíveis
- `POST /consultas` - Agendar consulta
- `GET /consultas/{consulta_id}` - Obter consulta por ID
- `GET /consultas` - Listar consultas
- `PATCH /consultas/{consulta_id}/status` - Atualizar status da consulta

### Horários de Profissionais
- `POST /horarios-profissionais` - Criar horário para profissional
- `GET /horarios-profissionais/{profissional_id}` - Listar horários de um profissional
- `GET /horarios-profissionais/detalhe/{horario_id}` - Obter horário por ID
- `PUT /horarios-profissionais/{horario_id}` - Atualizar horário
- `DELETE /horarios-profissionais/{horario_id}` - Deletar horário

### QR Code
- `GET /qrcode/generate/{consulta_id}` - Gerar QR Code para consulta

### Sistema
- `GET /` - Informações da API
- `GET /health` - Health check

## 📋 Pré-requisitos

- Python 3.8+
- PostgreSQL (via Docker para desenvolvimento local)
- Dependências Python (listadas em `requirements.txt`)

## 📁 Estrutura do Projeto

```
api_rest/
├── app/
│   ├── __init__.py
│   ├── main.py                      # Aplicação principal FastAPI
│   ├── core/
│   │   ├── config.py                # Configurações e variáveis de ambiente
│   │   ├── database.py              # Configuração do banco de dados
│   │   └── security.py              # Autenticação JWT e segurança
│   ├── models/
│   │   ├── categoria.py             # Model de categorias médicas
│   │   ├── consulta.py              # Model de consultas
│   │   ├── horario_profissional.py  # Model de horários dos profissionais
│   │   ├── paciente.py              # Model de pacientes
│   │   └── profissional.py          # Model de profissionais de saúde
│   ├── routers/
│   │   ├── auth.py                  # Rotas de autenticação JWT
│   │   ├── categorias.py            # Rotas de categorias
│   │   ├── consultas.py             # Rotas de consultas e horários
│   │   ├── horarios_profissionais.py # Rotas de horários dos profissionais
│   │   ├── pacientes.py             # Rotas de pacientes e biometria
│   │   ├── profissionais.py         # Rotas de profissionais
│   │   └── qrcode.py                # Rotas de geração de QR Code
│   ├── schemas/
│   │   ├── auth.py                  # Schemas de autenticação
│   │   ├── categoria.py             # Schemas de categorias
│   │   ├── consulta.py              # Schemas de consultas
│   │   ├── horario_profissional.py  # Schemas de horários
│   │   ├── paciente.py              # Schemas de pacientes
│   │   ├── profissional.py          # Schemas de profissionais
│   │   └── qrcode.py                # Schemas de QR Code
│   └── utils/
│       └── qrcode_utils.py          # Utilitários para geração de QR Code
├── scripts/
│   └── seed_data.py                 # Script para popular banco com dados iniciais
├── docker-compose.yml               # Configuração do PostgreSQL
├── requirements.txt                 # Dependências Python
├── run.py                           # Script de execução da aplicação
├── start_api.sh                     # Script de inicialização completa
├── stop_api.sh                      # Script para parar API e PostgreSQL
├── reset_db.sh                      # Script para resetar banco de dados
└── .env                             # Variáveis de ambiente (criar manualmente)
```

## 🚀 Deploy

### Desenvolvimento Local (Recomendado)

**Use o script automático** que gerencia todo o processo:

1. **Navegar para o diretório:**
   ```bash
   cd api_rest
   ```

2. **Configurar variáveis de ambiente:**
   ```bash
   cp .env.example .env
   # Edite .env com suas chaves secretas (JWT_SECRET_KEY, QR_SECRET_KEY)
   ```

3. **Executar o script:**
   ```bash
   chmod +x start_api.sh
   ./start_api.sh
   ```

**O script `start_api.sh` faz automaticamente:**
- ✅ Verifica se o Docker está rodando
- ✅ Inicia o PostgreSQL via Docker Compose
- ✅ Aguarda o banco estar pronto
- ✅ Cria o banco de dados `clinicas_db`
- ✅ Cria e ativa ambiente virtual Python
- ✅ Instala todas as dependências
- ✅ Popula o banco com dados iniciais (seed)
- ✅ Inicia o servidor FastAPI

4. **Parar a API:**
   ```bash
   ./stop_api.sh
   ```

### Variáveis de Ambiente Necessárias

Edite o arquivo `.env` com as seguintes variáveis:

- `DATABASE_URL`: URL de conexão com PostgreSQL (ex: `postgresql://postgres:postgres@localhost:5432/clinicas_db`)
- `JWT_SECRET_KEY`: Chave secreta para JWT (mínimo 32 caracteres)
- `QR_SECRET_KEY`: Chave secreta para QR Code (mínimo 16 caracteres)
- `ADMIN_USERNAME`, `ADMIN_PASSWORD`: Credenciais do administrador
- `BOT_USERNAME`, `BOT_PASSWORD`: Credenciais para o bot Telegram
- `DEVICE_USERNAME`, `DEVICE_PASSWORD`: Credenciais para dispositivos
- `ENVIRONMENT`: Definir como `development`


## 📖 Documentação da API

Após iniciar, acesse:
- **Swagger UI**: http://localhost:8000/docs
- **ReDoc**: http://localhost:8000/redoc
- **Health Check**: http://localhost:8000/health

## 🔐 Autenticação no Swagger

A API utiliza autenticação JWT. Para testar as rotas protegidas no Swagger UI:

1. **Obter token JWT:**
   - Vá para `POST /auth/token` no Swagger
   - Use uma das credenciais disponíveis:
     - Admin: `admin` / senha definida em `ADMIN_PASSWORD`
     - Bot: `bot_user` / senha definida em `BOT_PASSWORD`
     - Device: `device` / senha definida em `DEVICE_PASSWORD`
   - Clique em "Try it out" e "Execute"
   - Copie o token retornado (sem as aspas)

2. **Configurar autenticação no Swagger:**
   - Clique no botão "Authorize" (cadeado) no topo da página
   - Cole o token no campo "Value" com o prefixo `Bearer ` (ex: `Bearer eyJ0eXAi...`)
   - Clique em "Authorize"

3. **Testar rotas protegidas:**
   - Agora você pode testar todas as rotas autenticadas
   - O token será incluído automaticamente nos headers das requisições

## 🗄️ Banco de Dados

### Gerenciamento Automático

O script `start_api.sh` **gerencia automaticamente** o banco de dados PostgreSQL:
- Inicia o container PostgreSQL via Docker Compose
- Aguarda o PostgreSQL estar pronto
- Cria o banco `clinicas_db` (se não existir)
- Popula com dados iniciais (categorias, usuários admin, etc.)

**Não é necessário** executar comandos Docker manualmente quando usar o script!

### Conexão Manual para Inspeção

Se deseja inspecionar o banco de dados com **DBeaver**, **pgAdmin** ou outras ferramentas:

1. **Apenas iniciar o PostgreSQL** (sem a API):
   ```bash
   docker compose up -d postgres
   ```

2. **Credenciais de conexão:**
   - Host: `localhost`
   - Porta: `5432`
   - Database: `clinicas_db`
   - Usuário: `postgres`
   - Senha: `postgres`

3. **Parar o PostgreSQL:**
   ```bash
   docker compose down
   ```

### Scripts Utilitários

- **`start_api.sh`**: Inicia PostgreSQL + API 
- **`stop_api.sh`**: Para a API e o PostgreSQL
- **`reset_db.sh`**: Reseta o banco de dados


