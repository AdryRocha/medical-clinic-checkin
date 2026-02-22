import logging
import sys
from telegram import Update
from telegram.ext import Application, CommandHandler, MessageHandler, filters, ContextTypes
from dotenv import load_dotenv

from core.config import Config
from conversation_handlers.commands import start, help_command, unknown_command
from conversation_handlers.appointment import setup_appointment_handler, setup_recover_handler

load_dotenv()

logging.basicConfig(
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    level=logging.INFO
)
logger = logging.getLogger(__name__)


async def error_handler(update: object, context: ContextTypes.DEFAULT_TYPE) -> None:
    """
    Captura e registra erros do bot no sistema de logging.
    
    Args:
        update: Objeto que causou o erro
        context: Contexto contendo informações do erro
    """
    logger.error(f"Update {update} caused error {context.error}")


def main() -> None:
    """
    Função principal que inicializa e executa o bot do Telegram.
    
    Inicializa a configuração, valida as variáveis de ambiente,
    configura os handlers e inicia o polling do bot.
    """
    config = Config()
    
    try:
        config.validate()
        logger.info(f"Configuração carregada: {config}")
    except ValueError as e:
        logger.error(f"Erro na configuração: {e}")
        logger.error("Configure as variáveis de ambiente necessárias no arquivo .env")
        sys.exit(1)

    application = Application.builder().token(config.TELEGRAM_TOKEN).build()

    application.bot_data['config'] = config

    application.add_handler(CommandHandler("iniciar", start))
    application.add_handler(CommandHandler("start", start))
    application.add_handler(CommandHandler("ajuda", help_command))
    application.add_handler(CommandHandler("help", help_command))

    setup_appointment_handler(application, config)
    
    setup_recover_handler(application, config)

    application.add_handler(MessageHandler(filters.TEXT & ~filters.COMMAND, unknown_command))

    application.add_error_handler(error_handler)

    logger.info("Bot started. Press Ctrl+C to stop.")
    application.run_polling(allowed_updates=Update.ALL_TYPES)


if __name__ == '__main__':
    main()