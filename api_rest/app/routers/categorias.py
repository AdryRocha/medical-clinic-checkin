"""
Rotas da API para operações com Categorias
"""
from fastapi import APIRouter, Depends, HTTPException, status
from sqlalchemy.orm import Session
from typing import List
from app.core.database import get_db
from app.core.security import get_current_user
from app.models.categoria import Categoria
from app.schemas.categoria import CategoriaCreate, CategoriaResponse

router = APIRouter(prefix="/categorias", tags=["Categorias"])


@router.get("/", response_model=List[CategoriaResponse])
async def listar_categorias(
    db: Session = Depends(get_db),
    current_user: dict = Depends(get_current_user)
):
    """
    Lista todas as categorias/especialidades disponíveis
    """
    categorias = db.query(Categoria).all()
    return categorias


@router.post("/", response_model=CategoriaResponse, status_code=status.HTTP_201_CREATED)
async def criar_categoria(
    categoria_data: CategoriaCreate, 
    db: Session = Depends(get_db),
    current_user: dict = Depends(get_current_user)
):
    """
    Cria uma nova categoria/especialidade
    """
    existing = db.query(Categoria).filter(Categoria.nome == categoria_data.nome).first()
    if existing:
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST,
            detail=f"Categoria '{categoria_data.nome}' já existe"
        )
    
    categoria = Categoria(
        nome=categoria_data.nome,
        descricao=categoria_data.descricao
    )
    
    db.add(categoria)
    db.commit()
    db.refresh(categoria)
    
    return categoria