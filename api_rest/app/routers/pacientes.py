"""
Rotas da API para operações com Pacientes
"""
from fastapi import APIRouter, Depends, HTTPException, status, UploadFile, File
from fastapi.responses import Response
from sqlalchemy.orm import Session
from typing import List
from app.core.database import get_db
from app.core.security import get_current_user
from app.models.paciente import Paciente
from app.schemas.paciente import PacienteCreate, PacienteResponse

router = APIRouter(prefix="/pacientes", tags=["Pacientes"])


@router.get("/cpf/{cpf}", response_model=PacienteResponse)
async def buscar_paciente_por_cpf(
    cpf: str, 
    db: Session = Depends(get_db),
    current_user: dict = Depends(get_current_user)
):
    """
    Busca um paciente pelo CPF
    """
    if not cpf.isdigit() or len(cpf) != 11:
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST,
            detail="CPF deve conter exatamente 11 dígitos numéricos"
        )
    
    paciente = db.query(Paciente).filter(Paciente.cpf == cpf).first()
    
    if not paciente:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=f"Paciente com CPF {cpf} não encontrado"
        )
    
    return paciente


@router.post("", response_model=PacienteResponse, status_code=status.HTTP_201_CREATED)
async def cadastrar_paciente(
    paciente_data: PacienteCreate, 
    db: Session = Depends(get_db),
    current_user: dict = Depends(get_current_user)
):
    """
    Cadastra um novo paciente
    """
    existing = db.query(Paciente).filter(Paciente.cpf == paciente_data.cpf).first()
    if existing:
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST,
            detail=f"Paciente com CPF {paciente_data.cpf} já cadastrado"
        )
    
    paciente = Paciente(
        nome=paciente_data.nome,
        cpf=paciente_data.cpf,
        aceita_digital=paciente_data.aceita_digital
    )
    
    db.add(paciente)
    db.commit()
    db.refresh(paciente)
    
    return paciente


@router.get("", response_model=List[PacienteResponse])
async def listar_pacientes(
    skip: int = 0, 
    limit: int = 100, 
    db: Session = Depends(get_db),
    current_user: dict = Depends(get_current_user)
):
    """
    Lista todos os pacientes (com paginação)
    """
    pacientes = db.query(Paciente).offset(skip).limit(limit).all()
    return pacientes


@router.get("/{paciente_id}", response_model=PacienteResponse)
async def buscar_paciente_por_id(
    paciente_id: int, 
    db: Session = Depends(get_db),
    current_user: dict = Depends(get_current_user)
):
    """
    Busca um paciente pelo ID
    """
    paciente = db.query(Paciente).filter(Paciente.id == paciente_id).first()
    
    if not paciente:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=f"Paciente com ID {paciente_id} não encontrado"
        )
    
    return paciente


@router.post("/{paciente_id}/upload-fingerprint", response_model=PacienteResponse)
async def upload_fingerprint(
    paciente_id: int,
    file: UploadFile = File(...),
    db: Session = Depends(get_db),
    current_user: dict = Depends(get_current_user)
):
    """
    Faz upload do arquivo .DAT de impressão digital para um paciente
    """
    paciente = db.query(Paciente).filter(Paciente.id == paciente_id).first()
    
    if not paciente:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=f"Paciente com ID {paciente_id} não encontrado"
        )
    
    if not file.filename.endswith('.dat'):
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST,
            detail="Arquivo deve ser um .dat"
        )
    
    content = await file.read()
    
    paciente.fingerprint_data = content
    db.commit()
    db.refresh(paciente)
    
    return paciente


@router.get("/{paciente_id}/fingerprint")
async def download_fingerprint(
    paciente_id: int,
    db: Session = Depends(get_db),
    current_user: dict = Depends(get_current_user)
):
    """
    Faz download do arquivo .DAT de impressão digital de um paciente
    """
    paciente = db.query(Paciente).filter(Paciente.id == paciente_id).first()
    
    if not paciente:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=f"Paciente com ID {paciente_id} não encontrado"
        )
    
    if not paciente.fingerprint_data:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail="Paciente não possui impressão digital cadastrada"
        )
    
    return Response(
        content=paciente.fingerprint_data,
        media_type="application/octet-stream",
        headers={
            "Content-Disposition": f"attachment; filename=fingerprint_{paciente.cpf}.dat"
        }
    )
