"""
Módulo core - Infraestrutura e configurações fundamentais do bot.

Este módulo contém a camada de infraestrutura da aplicação,
incluindo configurações, constantes e setup básico.
"""

from .config import Config
from .constants import (
    # Estados da conversa
    CPF, NAME, ACCEPTS_DIGITAL, CATEGORY, PROFESSIONAL, DAY, TIME_SLOT, CONFIRM,
    # Constantes
    DIAS_SEMANA, DATE_FORMAT, DATETIME_FORMAT
)

__all__ = [
    'Config',
    'CPF', 'NAME', 'ACCEPTS_DIGITAL', 'CATEGORY', 
    'PROFESSIONAL', 'DAY', 'TIME_SLOT', 'CONFIRM',
    'DIAS_SEMANA', 'DATE_FORMAT', 'DATETIME_FORMAT'
]
