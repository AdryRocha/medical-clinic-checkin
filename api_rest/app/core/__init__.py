"""
Configurações e utilitários core da aplicação

Este pacote contém configurações centralizadas, setup do banco
de dados e utilitários de segurança/autenticação.
"""

from .config import settings
from .database import get_db, init_db
from .security import criar_token_acesso, verificar_token, get_current_user

__all__ = [
    "settings",
    "get_db",
    "init_db",
    "criar_token_acesso",
    "verificar_token",
    "get_current_user",
]
