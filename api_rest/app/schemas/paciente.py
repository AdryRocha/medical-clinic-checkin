"""
Schemas Pydantic para Paciente
"""
from pydantic import BaseModel, Field


class PacienteBase(BaseModel):
    """Schema base para Paciente"""
    nome: str = Field(..., min_length=3, max_length=200)
    cpf: str = Field(..., min_length=11, max_length=11, pattern=r'^\d{11}$')
    aceita_digital: bool = False


class PacienteCreate(PacienteBase):
    """Schema para criar Paciente"""
    pass


class PacienteResponse(PacienteBase):
    """Schema de resposta para Paciente"""
    id: int
    fingerprint_uploaded: bool = False
    
    class Config:
        from_attributes = True
