"""
Schemas Pydantic (Validação e Serialização)

Este pacote contém todos os schemas Pydantic para validação
de entrada e serialização de saída de dados da API.
"""

from .paciente import PacienteCreate, PacienteResponse
from .categoria import CategoriaCreate, CategoriaResponse
from .profissional import ProfissionalCreate, ProfissionalResponse
from .consulta import ConsultaCreate, ConsultaResponse, HorarioDisponivel
from .horario_profissional import HorarioProfissionalCreate, HorarioProfissionalUpdate, HorarioProfissionalResponse
from .qrcode import QRCodeResponse

__all__ = [
    "PacienteCreate",
    "PacienteResponse",
    "CategoriaCreate",
    "CategoriaResponse",
    "ProfissionalCreate",
    "ProfissionalResponse",
    "ConsultaCreate",
    "ConsultaResponse",
    "HorarioDisponivel",
    "HorarioProfissionalCreate",
    "HorarioProfissionalUpdate",
    "HorarioProfissionalResponse",
    "QRCodeResponse",
]
