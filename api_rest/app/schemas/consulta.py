"""
Schemas Pydantic para Consulta
"""
from pydantic import BaseModel, Field
from typing import Optional
from app.schemas.paciente import PacienteResponse
from app.schemas.profissional import ProfissionalResponse


class ConsultaBase(BaseModel):
    """Schema base para Consulta"""
    paciente_id: Optional[int] = None
    cpf: str = Field(..., pattern=r'^\d{11}$')
    nome: str = Field(..., min_length=3, max_length=200)
    profissional_id: int
    data: str = Field(..., pattern=r'^\d{4}-\d{2}-\d{2}$')
    horario: str = Field(..., pattern=r'^\d{2}:\d{2}$')


class ConsultaCreate(ConsultaBase):
    """Schema para criar Consulta"""
    pass


class ConsultaResponse(BaseModel):
    """Schema de resposta para Consulta"""
    id: int
    paciente_id: int
    profissional_id: int
    data: str
    horario: str
    status: str
    paciente: PacienteResponse
    profissional: ProfissionalResponse
    
    class Config:
        from_attributes = True


class HorarioDisponivel(BaseModel):
    """Schema para horário disponível"""
    horario: str
    disponivel: bool
