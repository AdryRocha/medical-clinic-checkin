"""
Modelo SQLAlchemy para Categoria
"""
from sqlalchemy import Column, Integer, String
from sqlalchemy.orm import relationship
from app.core.database import Base


class Categoria(Base):
    """Modelo de Categoria/Especialidade no banco de dados"""
    __tablename__ = "categorias"
    
    id = Column(Integer, primary_key=True, index=True)
    nome = Column(String(100), unique=True, nullable=False)
    descricao = Column(String(500))
    
    profissionais = relationship("Profissional", back_populates="categoria")
