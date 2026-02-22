"""
Schemas Pydantic para Profissional
"""
from pydantic import BaseModel, Field
from app.schemas.categoria import CategoriaResponse


class ProfissionalBase(BaseModel):
    """Schema base para Profissional"""
    nome: str = Field(..., min_length=3, max_length=200)
    registro: str = Field(..., min_length=3, max_length=50)
    categoria_id: int


class ProfissionalCreate(ProfissionalBase):
    """Schema para criar Profissional"""
    pass


class ProfissionalResponse(ProfissionalBase):
    """Schema de resposta para Profissional"""
    id: int
    categoria: CategoriaResponse
    
    class Config:
        from_attributes = True
