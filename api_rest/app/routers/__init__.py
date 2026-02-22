"""
Rotas da API

Este pacote contém todos os routers (endpoints) da API REST.
Cada arquivo representa um conjunto de rotas relacionadas.
"""

from . import auth, pacientes, categorias, profissionais, consultas, horarios_profissionais, qrcode

__all__ = [
    "auth",
    "pacientes",
    "categorias",
    "profissionais",
    "consultas",
    "horarios_profissionais",
    "qrcode",
]
