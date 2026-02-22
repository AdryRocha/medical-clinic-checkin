"""
Schemas Pydantic para Horário de Profissional
"""
from pydantic import BaseModel, Field, validator
from typing import Optional


class HorarioProfissionalBase(BaseModel):
    """Schema base para HorarioProfissional"""
    profissional_id: int = Field(..., description="ID do profissional")
    dia_semana: int = Field(..., ge=0, le=6, description="Dia da semana (0=segunda, 6=domingo)")
    hora_inicio: str = Field(..., pattern=r"^([01]\d|2[0-3]):([0-5]\d)$", description="Horário de início (HH:MM)")
    hora_fim: str = Field(..., pattern=r"^([01]\d|2[0-3]):([0-5]\d)$", description="Horário de término (HH:MM)")
    duracao_minutos: int = Field(15, ge=10, le=120, description="Duração de cada consulta em minutos (padrão: 15 min, opções: 15, 20, 30, 45, 60)")
    
    @validator('hora_fim')
    def validar_hora_fim(cls, v, values):
        """Valida que hora_fim é maior que hora_inicio"""
        if 'hora_inicio' in values:
            if v <= values['hora_inicio']:
                raise ValueError('hora_fim deve ser maior que hora_inicio')
        return v


class HorarioProfissionalCreate(HorarioProfissionalBase):
    """Schema para criar novo horário de profissional"""
    pass


class HorarioProfissionalUpdate(BaseModel):
    """Schema para atualizar horário de profissional"""
    dia_semana: Optional[int] = Field(None, ge=0, le=6)
    hora_inicio: Optional[str] = Field(None, pattern=r"^([01]\d|2[0-3]):([0-5]\d)$")
    hora_fim: Optional[str] = Field(None, pattern=r"^([01]\d|2[0-3]):([0-5]\d)$")
    duracao_minutos: Optional[int] = Field(None, ge=10, le=120)


class HorarioProfissionalResponse(HorarioProfissionalBase):
    """Schema de resposta para horário de profissional"""
    id: int
    
    class Config:
        from_attributes = True
