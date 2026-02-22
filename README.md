# 🏥 Sistema de Clínicas Médicas

Sistema completo para gerenciamento de clínicas médicas, composto por uma **API REST** desenvolvida com FastAPI e um **Bot do Telegram** para agendamento de consultas. O sistema permite gestão de pacientes, profissionais de saúde, horários de atendimento, agendamento de consultas e geração de QR Codes para check-in.

## 🚀 Visão Geral

O projeto é dividido em dois componentes principais:

### 1. **API REST** (`api_rest/`)
Backend completo desenvolvido com FastAPI que fornece:
- Gerenciamento de pacientes (cadastro, busca por CPF, biometria)
- Categorias e especialidades médicas
- Cadastro de profissionais de saúde
- Gestão de horários de atendimento
- Sistema de agendamento de consultas
- Geração de QR Code para check-in
- Autenticação JWT
- Documentação interativa (Swagger UI)

### 2. **Bot Telegram** (`bot/`)
Interface conversacional para pacientes via Telegram:
- Agendamento de consultas guiado
- Cadastro de novos pacientes
- Coleta de dados biométricos
- Seleção de especialidades e profissionais
- Visualização de consultas agendadas
- Geração de QR Code para check-in

## 📁 Estrutura do Projeto

```
Clinicas_Medicas_WebAPI/
├── api_rest/                    # API REST (FastAPI)
│   ├── app/                     # Código da aplicação
│   │   ├── core/                # Configurações, database, security
│   │   ├── models/              # Modelos SQLAlchemy
│   │   ├── routers/             # Endpoints da API
│   │   ├── schemas/             # Schemas Pydantic
│   │   └── utils/               # Utilitários
│   ├── scripts/                 # Scripts auxiliares
│   ├── docker-compose.yml       # PostgreSQL
│   ├── start_api.sh            # Script de inicialização
│   ├── stop_api.sh             # Script para parar
│   └── README.md               # Documentação detalhada da API
│
├── bot/                         # Bot Telegram
│   ├── src/                     # Código do bot
│   │   ├── api/                 # Cliente HTTP para API
│   │   ├── conversation_handlers/ # Fluxos de conversa
│   │   ├── core/                # Configurações
│   │   ├── mock/                # Dados para testes
│   │   └── services/            # Serviços auxiliares
│   ├── start_bot.sh            # Script de inicialização
│   ├── stop_bot.sh             # Script para parar
│   └── README.md               # Documentação detalhada do bot
│
└── README.md                    # Este arquivo
```

## 📋 Pré-requisitos

- **Python 3.12+**
- **Docker** e **Docker Compose** (para PostgreSQL)
- **Token do Bot Telegram** (obtido via [@BotFather](https://t.me/botfather))

## 🚀 Início Rápido

### 1️⃣ Configurar e Iniciar a API REST

```bash
# Navegar para o diretório da API
cd api_rest

# Configurar variáveis de ambiente
cp .env.example .env
# Edite o .env com suas chaves secretas

# Iniciar API (PostgreSQL + FastAPI)
chmod +x start_api.sh
./start_api.sh
```

A API estará disponível em:
- **API**: http://localhost:8000

📖 **Documentação completa**: [`api_rest/README.md`](api_rest/README.md)

### 2️⃣ Configurar e Iniciar o Bot Telegram

```bash
# Navegar para o diretório do bot
cd bot

# Configurar variáveis de ambiente
cp .env.example .env
# Edite o .env com seu token do Telegram e credenciais da API

# Iniciar bot
chmod +x start_bot.sh
./start_bot.sh
```

📖 **Documentação completa**: [`bot/README.md`](bot/README.md)

### 3️⃣ Parar os Serviços

```bash
# Parar API
cd api_rest
./stop_api.sh

# Parar Bot
cd bot
./stop_bot.sh
```

## 🗄️ Banco de Dados Local

O PostgreSQL é gerenciado automaticamente via Docker:

```bash
# Acessar banco com ferramentas externas (DBeaver, pgAdmin)
# Credenciais:
Host: localhost
Porta: 5432
Database: clinicas_db
Usuário: postgres
Senha: postgres
```

## 🎯 Fluxo de Uso

1. **Iniciar a API REST** - Backend está pronto para receber requisições
2. **Iniciar o Bot Telegram** - Interface para pacientes via Telegram
3. **Paciente acessa o bot** - Conversa com @seu_bot no Telegram
4. **Agendamento guiado** - Bot guia o paciente passo a passo
5. **Confirmação** - Consulta agendada e QR Code gerado
6. **Check-in** - Paciente usa QR Code na clínica

## 🛠️ Tecnologias Utilizadas

### Backend (API REST)
- **FastAPI** - Framework web moderno e rápido
- **SQLAlchemy** - ORM para Python
- **PostgreSQL** - Banco de dados relacional
- **Pydantic** - Validação de dados
- **JWT** - Autenticação e autorização
- **Docker** - Containerização do PostgreSQL
- **QR Code** - Geração de códigos para check-in

### Bot
- **python-telegram-bot** - Framework para bots do Telegram
- **httpx** - Cliente HTTP assíncrono
- **Conversation Handler** - Gerenciamento de estados de conversa

## 📚 Documentação Detalhada

Cada componente possui sua própria documentação completa:

- **[API REST](api_rest/README.md)** - Endpoints, autenticação, banco de dados, estrutura
- **[Bot Telegram](bot/README.md)** - Comandos, fluxo de conversação, configuração