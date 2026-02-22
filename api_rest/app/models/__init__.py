"""
Modelos SQLAlchemy (Tabelas do Banco de Dados)

Este pacote contém todos os modelos de dados que representam
as tabelas do banco de dados usando SQLAlchemy ORM.
"""

from .paciente import Paciente
from .categoria import Categoria
from .profissional import Profissional
from .consulta import Consulta
from .horario_profissional import HorarioProfissional

__all__ = [
    "Paciente",
    "Categoria", 
    "Profissional",
    "Consulta",
    "HorarioProfissional"
]
