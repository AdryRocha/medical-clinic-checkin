"""
Módulo services - Camada de lógica de negócio reutilizável.

Contém serviços que implementam lógica de negócio independente
da interface (Telegram), permitindo reutilização em outros contextos.
"""

from .formatting import (
    clean_cpf,
    format_cpf,
    format_date,
    parse_date,
    get_category_name
)

__all__ = [
    # Formatting
    'clean_cpf',
    'format_cpf',
    'format_date',
    'parse_date',
    'get_category_name'
]
