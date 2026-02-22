"""
Handlers para comandos básicos do bot.

Comandos simples como /start, /help e mensagens não reconhecidas.
"""

import logging
from telegram import Update
from telegram.ext import ContextTypes

logger = logging.getLogger(__name__)


async def start(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    """
    Processa o comando /iniciar ou /start enviando mensagem de boas-vindas.
    
    Args:
        update: Objeto Update do Telegram contendo informações da mensagem
        context: Contexto da conversa com dados do bot
    """
    user = update.effective_user
    await update.message.reply_text(
        f"Olá {user.mention_html()}! 👋\n\n"
        "Bem-vindo ao sistema de agendamento de consultas médicas.\n\n"
        "Opções: /agendar | /minhas_consultas | /ajuda",
        parse_mode='HTML'
    )


async def help_command(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    """
    Processa o comando /ajuda exibindo lista de comandos disponíveis.
    
    Args:
        update: Objeto Update do Telegram contendo informações da mensagem
        context: Contexto da conversa com dados do bot
    """
    await update.message.reply_text(
        "🤖 Comandos disponíveis:\n\n"
        "/iniciar - Iniciar conversa com o bot\n"
        "/agendar - Agendar uma nova consulta\n"
        "/minhas_consultas - Recuperar consultas agendadas e gerar QR code\n"
        "/ajuda - Ver todos os comandos\n"
        "/cancelar - Cancelar agendamento em andamento"
    )


async def unknown_command(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    """
    Processa mensagens não reconhecidas e exibe ajuda.
    
    Args:
        update: Objeto Update do Telegram contendo informações da mensagem
        context: Contexto da conversa com dados do bot
    """
    await update.message.reply_text(
        "🤔 Não entendi sua mensagem.\n\n"
        "Para começar, use o comando /iniciar"
    )
