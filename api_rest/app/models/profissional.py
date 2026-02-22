"""
Modelo SQLAlchemy para Profissional
"""
from sqlalchemy import Column, Integer, String, ForeignKey
from sqlalchemy.orm import relationship
from app.core.database import Base


class Profissional(Base):
    """Modelo de Profissional de Saúde no banco de dados"""
    __tablename__ = "profissionais"
    
    id = Column(Integer, primary_key=True, index=True)
    nome = Column(String(200), nullable=False)
    registro = Column(String(50), unique=True, nullable=False)
    categoria_id = Column(Integer, ForeignKey("categorias.id"), nullable=False)
    
    categoria = relationship("Categoria", back_populates="profissionais")
    consultas = relationship("Consulta", back_populates="profissional")
    horarios = relationship("HorarioProfissional", back_populates="profissional")
