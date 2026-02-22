# 🤖 Bot Telegram - Sistema de Clínicas Médicas

Bot do Telegram para agendamento de consultas médicas, integrado à API REST do sistema. Permite que pacientes agendem consultas, visualizem agendamentos e gerem QR Codes para check-in de forma simples e intuitiva via chat.

## 🚀 Funcionalidades

- Agendamento completo de consultas via conversação guiada
- Busca e cadastro de pacientes por CPF
- Cadastro de dados biométricos (impressão digital)
- Seleção de especialidades médicas
- Escolha de profissionais de saúde
- Verificação de horários disponíveis por data
- Recuperação de consultas agendadas por CPF
- Geração de QR Code para check-in nas consultas
- Modo mock para testes sem API
- Autenticação automática com a API REST
- Sistema de conversação com estados (ConversationHandler)

## 🎯 Comandos Disponíveis

- `/iniciar` ou `/start` - Inicia o bot e exibe mensagem de boas-vindas
- `/agendar` - Inicia o processo de agendamento de consulta
- `/minhas_consultas` - Recupera consultas agendadas e gera QR Codes
- `/ajuda` ou `/help` - Lista todos os comandos disponíveis
- `/cancelar` - Cancela a operação atual e retorna ao menu inicial

**Mensagens não reconhecidas** exibem automaticamente a ajuda.

## 📋 Fluxo de Agendamento

O bot guia o usuário através de uma conversação estruturada:

1. **CPF**: Solicitação do CPF do paciente (validação automática)
2. **Nome**: Se novo paciente, solicitação do nome completo
3. **Biometria**: Pergunta se aceita coleta de impressão digital biométrica
4. **Especialidade**: Seleção da especialidade médica (botões inline)
5. **Profissional**: Escolha do profissional de saúde disponível
6. **Data**: Seleção da data desejada (datas com horários disponíveis)
7. **Horário**: Escolha do horário disponível na data selecionada
8. **Confirmação**: Revisão completa dos dados antes de confirmar
9. **QR Code**: Geração automática de QR Code para check-in

## 📋 Pré-requisitos

- Python 3.12+
- Token do Bot Telegram (obtido via [@BotFather](https://t.me/botfather))
- API REST rodando localmente

## 📁 Estrutura do Projeto

```
bot/
├── src/
│   ├── telegram_bot.py              # Ponto de entrada principal do bot
│   ├── api/
│   │   └── api_client.py            # Cliente HTTP para comunicação com API
│   ├── conversation_handlers/
│   │   ├── appointment.py           # Fluxo de agendamento de consultas
│   │   ├── commands.py              # Comandos básicos (/start, /help, etc)
│   │   └── helpers.py               # Funções auxiliares
│   ├── core/
│   │   ├── config.py                # Configurações centralizadas
│   │   └── constants.py             # Constantes e estados da conversa
│   ├── mock/
│   │   ├── mock_data.py             # Dados simulados para testes
│   │   └── mock_qrcode_service.py   # Serviço mock de QR Code
│   └── services/
│       └── formatting.py            # Formatação de mensagens
├── requirements.txt                 # Dependências Python
├── start_bot.sh                     # Script de inicialização
├── stop_bot.sh                      # Script para parar o bot
└── .env                             # Variáveis de ambiente (criar manualmente)
```

## 🚀 Instalação e Execução

**Use o script automático** que gerencia todo o processo:

1. **Navegar para o diretório:**
   ```bash
   cd bot
   ```

2. **Configurar variáveis de ambiente:**
   ```bash
   cp .env.example .env
   nano .env  # ou seu editor preferido
   ```

3. **Obter Token do Telegram:**
   - Acesse [@BotFather](https://t.me/botfather) no Telegram
   - Envie `/newbot` e siga as instruções
   - Copie o token fornecido para o arquivo `.env`

4. **Executar o script:**
   ```bash
   chmod +x start_bot.sh
   ./start_bot.sh
   ```

**O script `start_bot.sh` faz automaticamente:**
- ✅ Verifica se o arquivo `.env` existe
- ✅ Valida se o `TELEGRAM_TOKEN` está configurado
- ✅ Ativa o ambiente virtual Python (se existir)
- ✅ Inicia o bot Telegram

5. **Parar o bot:**
   ```bash
   ./stop_bot.sh
   ```

### Variáveis de Ambiente Necessárias

Edite o arquivo `.env` com as seguintes variáveis:

**Obrigatórias:**
- `TELEGRAM_TOKEN`: Token obtido via @BotFather
- `API_BASE_URL`: URL da API REST (ex: `http://localhost:8000`)
- `API_USERNAME`: Usuário para autenticação na API
- `API_PASSWORD`: Senha do usuário bot configurada na API

**Opcionais:**
- `LOG_LEVEL`: Nível de log (padrão: `INFO`)
- `API_TIMEOUT`: Timeout das requisições HTTP em segundos (padrão: `30`)
- `USE_MOCK_DATA`: `true` para modo mock, `false` para API real (padrão: `false`)

**Apenas para Modo Mock:**
- `QR_SECRET_KEY`: Chave secreta para geração de QR Code (necessária apenas se `USE_MOCK_DATA=true`)

## 🧪 Modo Mock (Testes sem API)

Para testar o bot sem necessidade da API rodando:

1. **Configurar modo mock no `.env`:**
   ```env
   USE_MOCK_DATA=true
   QR_SECRET_KEY=chave_secreta_minimo_16_chars
   ```

2. **Executar normalmente:**
   ```bash
   ./start_bot.sh
   ```

**O modo mock simula:**
- ✅ Dados de pacientes
- ✅ Categorias médicas
- ✅ Profissionais de saúde
- ✅ Horários disponíveis
- ✅ Agendamento de consultas
- ✅ Geração de QR Codes

**Útil para:**
- Testes de interface e fluxo de conversação
- Desenvolvimento sem dependência da API
- Demonstrações offline