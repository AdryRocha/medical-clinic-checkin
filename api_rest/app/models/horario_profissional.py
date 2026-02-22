"""
Modelo SQLAlchemy para Horário de Profissional
"""
from sqlalchemy import Column, Integer, String, ForeignKey
from sqlalchemy.orm import relationship
from app.core.database import Base


class HorarioProfissional(Base):
    """
    Modelo de Horário de Atendimento por Profissional no banco de dados.
    Define os períodos de atendimento semanais de cada profissional.
    """
    __tablename__ = "horarios_profissionais"
    
    id = Column(Integer, primary_key=True, index=True)
    profissional_id = Column(Integer, ForeignKey("profissionais.id"), nullable=False)
    dia_semana = Column(Integer, nullable=False)
    hora_inicio = Column(String(5), nullable=False)
    hora_fim = Column(String(5), nullable=False)
    duracao_minutos = Column(Integer, nullable=False, default=15)
    
    profissional = relationship("Profissional", back_populates="horarios")
