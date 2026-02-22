"""
Módulo conversation_handlers - Camada de apresentação e fluxo de conversa.

Contém handlers para comandos do Telegram e gerenciamento do fluxo
de conversação com o usuário.
"""

from .commands import start, help_command, unknown_command
from .appointment import setup_appointment_handler
from . import helpers

__all__ = [
    'start',
    'help_command',
    'unknown_command',
    'setup_appointment_handler',
    'helpers'
]
