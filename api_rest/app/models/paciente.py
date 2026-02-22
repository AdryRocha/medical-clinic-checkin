"""
Modelo SQLAlchemy para Paciente
"""
from sqlalchemy import Column, Integer, String, Boolean, DateTime, LargeBinary
from sqlalchemy.orm import relationship
from datetime import datetime
from app.core.database import Base


class Paciente(Base):
    """Modelo de Paciente no banco de dados"""
    __tablename__ = "pacientes"
    
    id = Column(Integer, primary_key=True, index=True)
    nome = Column(String(200), nullable=False)
    cpf = Column(String(11), unique=True, index=True, nullable=False)
    aceita_digital = Column(Boolean, default=False)
    fingerprint_data = Column(LargeBinary, nullable=True)
    
    consultas = relationship("Consulta", back_populates="paciente")
    
    @property
    def fingerprint_uploaded(self) -> bool:
        return self.fingerprint_data is not None
