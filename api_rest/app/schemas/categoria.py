"""
Schemas Pydantic para Categoria
"""
from pydantic import BaseModel, Field
from typing import Optional


class CategoriaBase(BaseModel):
    """Schema base para Categoria"""
    nome: str = Field(..., min_length=3, max_length=100)
    descricao: Optional[str] = None


class CategoriaCreate(CategoriaBase):
    """Schema para criar Categoria"""
    pass


class CategoriaResponse(CategoriaBase):
    """Schema de resposta para Categoria"""
    id: int
    
    class Config:
        from_attributes = True
