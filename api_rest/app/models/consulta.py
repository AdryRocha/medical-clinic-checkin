"""
Modelo SQLAlchemy para Consulta
"""
from sqlalchemy import Column, Integer, String, DateTime, ForeignKey
from sqlalchemy.orm import relationship
from datetime import datetime
from app.core.database import Base


class Consulta(Base):
    """Modelo de Consulta no banco de dados"""
    __tablename__ = "consultas"
    
    id = Column(Integer, primary_key=True, index=True)
    paciente_id = Column(Integer, ForeignKey("pacientes.id"), nullable=False)
    profissional_id = Column(Integer, ForeignKey("profissionais.id"), nullable=False)
    data = Column(String(10), nullable=False)
    horario = Column(String(5), nullable=False)
    status = Column(String(20), default="agendada")
    
    paciente = relationship("Paciente", back_populates="consultas")
    profissional = relationship("Profissional", back_populates="consultas")
