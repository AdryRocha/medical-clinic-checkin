"""
Script para popular o banco de dados com dados de exemplo
"""
from sqlalchemy.orm import Session
from app.core.database import SessionLocal, init_db
from app.models.categoria import Categoria
from app.models.profissional import Profissional
from app.models.paciente import Paciente
from app.models.horario_profissional import HorarioProfissional


def popular_dados():
    """
    Popula o banco de dados com dados de exemplo
    """
    db: Session = SessionLocal()
    
    try:
        print("🔄 Populando banco de dados com dados de exemplo...")
        
        # Criar categorias/especialidades
        categorias = [
            Categoria(nome="Cardiologia", descricao="Especialidade médica que cuida do coração"),
            Categoria(nome="Dermatologia", descricao="Especialidade médica que cuida da pele"),
            Categoria(nome="Pediatria", descricao="Especialidade médica que cuida de crianças"),
            Categoria(nome="Ortopedia", descricao="Especialidade médica que cuida dos ossos e articulações"),
            Categoria(nome="Oftalmologia", descricao="Especialidade médica que cuida dos olhos"),
            Categoria(nome="Psicologia", descricao="Profissional que cuida da saúde mental e emocional"),
            Categoria(nome="Nutrição", descricao="Profissional que cuida da alimentação e nutrição"),
            Categoria(nome="Fisioterapia", descricao="Profissional que cuida da reabilitação física e motora"),
            Categoria(nome="Odontologia", descricao="Profissional que cuida da saúde bucal"),
        ]
        
        for categoria in categorias:
            existing = db.query(Categoria).filter(Categoria.nome == categoria.nome).first()
            if not existing:
                db.add(categoria)
        
        db.commit()
        print("✅ Categorias criadas!")
        
        # Buscar categorias para associar profissionais
        cat_cardio = db.query(Categoria).filter(Categoria.nome == "Cardiologia").first()
        cat_dermato = db.query(Categoria).filter(Categoria.nome == "Dermatologia").first()
        cat_pediatria = db.query(Categoria).filter(Categoria.nome == "Pediatria").first()
        cat_ortopedia = db.query(Categoria).filter(Categoria.nome == "Ortopedia").first()
        cat_psicologia = db.query(Categoria).filter(Categoria.nome == "Psicologia").first()
        cat_nutricao = db.query(Categoria).filter(Categoria.nome == "Nutrição").first()
        cat_fisioterapia = db.query(Categoria).filter(Categoria.nome == "Fisioterapia").first()
        cat_odontologia = db.query(Categoria).filter(Categoria.nome == "Odontologia").first()
        
        # Criar profissionais com diversos registros profissionais
        profissionais = [
            # Médicos (CRM)
            Profissional(nome="Dr. João Silva", registro="CRM12345-SP", categoria_id=cat_cardio.id),
            Profissional(nome="Dra. Maria Santos", registro="CRM23456-RJ", categoria_id=cat_dermato.id),
            Profissional(nome="Dr. Pedro Oliveira", registro="CRM34567-MG", categoria_id=cat_pediatria.id),
            Profissional(nome="Dra. Ana Costa", registro="CRM45678-SP", categoria_id=cat_ortopedia.id),
            Profissional(nome="Dr. Carlos Souza", registro="CRM56789-SP", categoria_id=cat_cardio.id),
            # Psicólogos (CRP)
            Profissional(nome="Psic. Juliana Ferreira", registro="CRP06-12345", categoria_id=cat_psicologia.id),
            Profissional(nome="Psic. Roberto Lima", registro="CRP06-23456", categoria_id=cat_psicologia.id),
            # Nutricionistas (CRN)
            Profissional(nome="Nutr. Beatriz Alves", registro="CRN3-45678", categoria_id=cat_nutricao.id),
            Profissional(nome="Nutr. Felipe Rocha", registro="CRN3-56789", categoria_id=cat_nutricao.id),
            # Fisioterapeutas (CREFITO)
            Profissional(nome="Ft. Amanda Ribeiro", registro="CREFITO3-123456", categoria_id=cat_fisioterapia.id),
            Profissional(nome="Ft. Lucas Martins", registro="CREFITO3-234567", categoria_id=cat_fisioterapia.id),
            # Dentistas (CRO)
            Profissional(nome="Dr. Gabriel Mendes", registro="CRO-SP-12345", categoria_id=cat_odontologia.id),
            Profissional(nome="Dra. Camila Torres", registro="CRO-SP-23456", categoria_id=cat_odontologia.id),
        ]
        
        for profissional in profissionais:
            existing = db.query(Profissional).filter(Profissional.registro == profissional.registro).first()
            if not existing:
                db.add(profissional)
        
        db.commit()
        print("✅ Profissionais criados!")
        
        # Criar alguns pacientes de exemplo
        pacientes = [
            Paciente(nome="José da Silva", cpf="12345678901", aceita_digital=True),
            Paciente(nome="Maria Oliveira", cpf="23456789012", aceita_digital=False),
            Paciente(nome="João Santos", cpf="34567890123", aceita_digital=True),
        ]
        
        for paciente in pacientes:
            existing = db.query(Paciente).filter(Paciente.cpf == paciente.cpf).first()
            if not existing:
                db.add(paciente)
        
        db.commit()
        print("✅ Pacientes de exemplo criados!")
        
        # Criar horários de atendimento dos profissionais
        
        # Dr. João Silva (Cardiologia) - Segunda, Quarta e Sexta - consultas de 45 min
        prof_joao = db.query(Profissional).filter(Profissional.registro == "CRM12345-SP").first()
        if prof_joao:
            horarios_joao = [
                HorarioProfissional(profissional_id=prof_joao.id, dia_semana=0, hora_inicio="08:00", hora_fim="12:00", duracao_minutos=45),
                HorarioProfissional(profissional_id=prof_joao.id, dia_semana=0, hora_inicio="14:00", hora_fim="17:00", duracao_minutos=45),
                HorarioProfissional(profissional_id=prof_joao.id, dia_semana=2, hora_inicio="08:00", hora_fim="12:00", duracao_minutos=45),
                HorarioProfissional(profissional_id=prof_joao.id, dia_semana=4, hora_inicio="08:00", hora_fim="12:00", duracao_minutos=45),
            ]
            for horario in horarios_joao:
                existing = db.query(HorarioProfissional).filter(
                    HorarioProfissional.profissional_id == horario.profissional_id,
                    HorarioProfissional.dia_semana == horario.dia_semana,
                    HorarioProfissional.hora_inicio == horario.hora_inicio
                ).first()
                if not existing:
                    db.add(horario)
        
        # Dra. Maria Santos (Dermatologia) - Segunda a Sexta - consultas de 20 min
        prof_maria = db.query(Profissional).filter(Profissional.registro == "CRM23456-RJ").first()
        if prof_maria:
            horarios_maria = [
                HorarioProfissional(profissional_id=prof_maria.id, dia_semana=0, hora_inicio="08:30", hora_fim="11:30", duracao_minutos=20),
                HorarioProfissional(profissional_id=prof_maria.id, dia_semana=1, hora_inicio="08:30", hora_fim="11:30", duracao_minutos=20),
                HorarioProfissional(profissional_id=prof_maria.id, dia_semana=2, hora_inicio="14:30", hora_fim="16:30", duracao_minutos=20),
                HorarioProfissional(profissional_id=prof_maria.id, dia_semana=3, hora_inicio="08:30", hora_fim="11:30", duracao_minutos=20),
                HorarioProfissional(profissional_id=prof_maria.id, dia_semana=4, hora_inicio="08:30", hora_fim="11:30", duracao_minutos=20),
            ]
            for horario in horarios_maria:
                existing = db.query(HorarioProfissional).filter(
                    HorarioProfissional.profissional_id == horario.profissional_id,
                    HorarioProfissional.dia_semana == horario.dia_semana,
                    HorarioProfissional.hora_inicio == horario.hora_inicio
                ).first()
                if not existing:
                    db.add(horario)
        
        # Dr. Pedro Oliveira (Pediatria) - Segunda a Sexta - consultas de 15 min (rápidas)
        prof_pedro = db.query(Profissional).filter(Profissional.registro == "CRM34567-MG").first()
        if prof_pedro:
            horarios_pedro = [
                HorarioProfissional(profissional_id=prof_pedro.id, dia_semana=0, hora_inicio="08:00", hora_fim="11:00", duracao_minutos=15),
                HorarioProfissional(profissional_id=prof_pedro.id, dia_semana=1, hora_inicio="08:00", hora_fim="11:00", duracao_minutos=15),
                HorarioProfissional(profissional_id=prof_pedro.id, dia_semana=2, hora_inicio="08:00", hora_fim="11:00", duracao_minutos=15),
                HorarioProfissional(profissional_id=prof_pedro.id, dia_semana=3, hora_inicio="08:00", hora_fim="11:00", duracao_minutos=15),
                HorarioProfissional(profissional_id=prof_pedro.id, dia_semana=4, hora_inicio="08:00", hora_fim="11:00", duracao_minutos=15),
            ]
            for horario in horarios_pedro:
                existing = db.query(HorarioProfissional).filter(
                    HorarioProfissional.profissional_id == horario.profissional_id,
                    HorarioProfissional.dia_semana == horario.dia_semana,
                    HorarioProfissional.hora_inicio == horario.hora_inicio
                ).first()
                if not existing:
                    db.add(horario)
        
        # Dra. Ana Costa (Ortopedia) - Terça, Quinta e Sábado - consultas de 30 min
        prof_ana = db.query(Profissional).filter(Profissional.registro == "CRM45678-SP").first()
        if prof_ana:
            horarios_ana = [
                HorarioProfissional(profissional_id=prof_ana.id, dia_semana=1, hora_inicio="09:00", hora_fim="12:00", duracao_minutos=30),
                HorarioProfissional(profissional_id=prof_ana.id, dia_semana=1, hora_inicio="14:00", hora_fim="17:00", duracao_minutos=30),
                HorarioProfissional(profissional_id=prof_ana.id, dia_semana=3, hora_inicio="09:00", hora_fim="12:00", duracao_minutos=30),
                HorarioProfissional(profissional_id=prof_ana.id, dia_semana=5, hora_inicio="08:00", hora_fim="12:00", duracao_minutos=30),
            ]
            for horario in horarios_ana:
                existing = db.query(HorarioProfissional).filter(
                    HorarioProfissional.profissional_id == horario.profissional_id,
                    HorarioProfissional.dia_semana == horario.dia_semana,
                    HorarioProfissional.hora_inicio == horario.hora_inicio
                ).first()
                if not existing:
                    db.add(horario)
        
        # Dr. Carlos Souza (Cardiologia) - Segunda, Terça e Quinta - consultas de 60 min (mais longas)
        prof_carlos = db.query(Profissional).filter(Profissional.registro == "CRM56789-SP").first()
        if prof_carlos:
            horarios_carlos = [
                HorarioProfissional(profissional_id=prof_carlos.id, dia_semana=0, hora_inicio="14:00", hora_fim="18:00", duracao_minutos=60),
                HorarioProfissional(profissional_id=prof_carlos.id, dia_semana=1, hora_inicio="14:00", hora_fim="18:00", duracao_minutos=60),
                HorarioProfissional(profissional_id=prof_carlos.id, dia_semana=3, hora_inicio="14:00", hora_fim="18:00", duracao_minutos=60),
            ]
            for horario in horarios_carlos:
                existing = db.query(HorarioProfissional).filter(
                    HorarioProfissional.profissional_id == horario.profissional_id,
                    HorarioProfissional.dia_semana == horario.dia_semana,
                    HorarioProfissional.hora_inicio == horario.hora_inicio
                ).first()
                if not existing:
                    db.add(horario)
        
        # Psic. Juliana Ferreira (Psicologia) - Segunda a Sexta - consultas de 45 min
        prof_juliana = db.query(Profissional).filter(Profissional.registro == "CRP06-12345").first()
        if prof_juliana:
            horarios_juliana = [
                HorarioProfissional(profissional_id=prof_juliana.id, dia_semana=0, hora_inicio="09:00", hora_fim="12:00", duracao_minutos=45),
                HorarioProfissional(profissional_id=prof_juliana.id, dia_semana=0, hora_inicio="14:00", hora_fim="18:00", duracao_minutos=45),
                HorarioProfissional(profissional_id=prof_juliana.id, dia_semana=1, hora_inicio="09:00", hora_fim="12:00", duracao_minutos=45),
                HorarioProfissional(profissional_id=prof_juliana.id, dia_semana=2, hora_inicio="14:00", hora_fim="18:00", duracao_minutos=45),
                HorarioProfissional(profissional_id=prof_juliana.id, dia_semana=3, hora_inicio="09:00", hora_fim="12:00", duracao_minutos=45),
                HorarioProfissional(profissional_id=prof_juliana.id, dia_semana=4, hora_inicio="09:00", hora_fim="12:00", duracao_minutos=45),
            ]
            for horario in horarios_juliana:
                existing = db.query(HorarioProfissional).filter(
                    HorarioProfissional.profissional_id == horario.profissional_id,
                    HorarioProfissional.dia_semana == horario.dia_semana,
                    HorarioProfissional.hora_inicio == horario.hora_inicio
                ).first()
                if not existing:
                    db.add(horario)
        
        # Psic. Roberto Lima (Psicologia) - Terça, Quinta e Sábado - consultas de 60 min
        prof_roberto = db.query(Profissional).filter(Profissional.registro == "CRP06-23456").first()
        if prof_roberto:
            horarios_roberto = [
                HorarioProfissional(profissional_id=prof_roberto.id, dia_semana=1, hora_inicio="13:00", hora_fim="19:00", duracao_minutos=60),
                HorarioProfissional(profissional_id=prof_roberto.id, dia_semana=3, hora_inicio="13:00", hora_fim="19:00", duracao_minutos=60),
                HorarioProfissional(profissional_id=prof_roberto.id, dia_semana=5, hora_inicio="09:00", hora_fim="13:00", duracao_minutos=60),
            ]
            for horario in horarios_roberto:
                existing = db.query(HorarioProfissional).filter(
                    HorarioProfissional.profissional_id == horario.profissional_id,
                    HorarioProfissional.dia_semana == horario.dia_semana,
                    HorarioProfissional.hora_inicio == horario.hora_inicio
                ).first()
                if not existing:
                    db.add(horario)
        
        # Nutr. Beatriz Alves (Nutrição) - Segunda, Quarta e Sexta - consultas de 30 min
        prof_beatriz = db.query(Profissional).filter(Profissional.registro == "CRN3-45678").first()
        if prof_beatriz:
            horarios_beatriz = [
                HorarioProfissional(profissional_id=prof_beatriz.id, dia_semana=0, hora_inicio="08:00", hora_fim="12:00", duracao_minutos=30),
                HorarioProfissional(profissional_id=prof_beatriz.id, dia_semana=0, hora_inicio="13:00", hora_fim="17:00", duracao_minutos=30),
                HorarioProfissional(profissional_id=prof_beatriz.id, dia_semana=2, hora_inicio="08:00", hora_fim="12:00", duracao_minutos=30),
                HorarioProfissional(profissional_id=prof_beatriz.id, dia_semana=4, hora_inicio="08:00", hora_fim="12:00", duracao_minutos=30),
            ]
            for horario in horarios_beatriz:
                existing = db.query(HorarioProfissional).filter(
                    HorarioProfissional.profissional_id == horario.profissional_id,
                    HorarioProfissional.dia_semana == horario.dia_semana,
                    HorarioProfissional.hora_inicio == horario.hora_inicio
                ).first()
                if not existing:
                    db.add(horario)
        
        # Nutr. Felipe Rocha (Nutrição) - Terça e Quinta - consultas de 45 min
        prof_felipe = db.query(Profissional).filter(Profissional.registro == "CRN3-56789").first()
        if prof_felipe:
            horarios_felipe = [
                HorarioProfissional(profissional_id=prof_felipe.id, dia_semana=1, hora_inicio="14:00", hora_fim="18:00", duracao_minutos=45),
                HorarioProfissional(profissional_id=prof_felipe.id, dia_semana=3, hora_inicio="14:00", hora_fim="18:00", duracao_minutos=45),
            ]
            for horario in horarios_felipe:
                existing = db.query(HorarioProfissional).filter(
                    HorarioProfissional.profissional_id == horario.profissional_id,
                    HorarioProfissional.dia_semana == horario.dia_semana,
                    HorarioProfissional.hora_inicio == horario.hora_inicio
                ).first()
                if not existing:
                    db.add(horario)
        
        # Ft. Amanda Ribeiro (Fisioterapia) - Segunda a Sexta - consultas de 30 min
        prof_amanda = db.query(Profissional).filter(Profissional.registro == "CREFITO3-123456").first()
        if prof_amanda:
            horarios_amanda = [
                HorarioProfissional(profissional_id=prof_amanda.id, dia_semana=0, hora_inicio="07:00", hora_fim="12:00", duracao_minutos=30),
                HorarioProfissional(profissional_id=prof_amanda.id, dia_semana=1, hora_inicio="07:00", hora_fim="12:00", duracao_minutos=30),
                HorarioProfissional(profissional_id=prof_amanda.id, dia_semana=2, hora_inicio="07:00", hora_fim="12:00", duracao_minutos=30),
                HorarioProfissional(profissional_id=prof_amanda.id, dia_semana=3, hora_inicio="07:00", hora_fim="12:00", duracao_minutos=30),
                HorarioProfissional(profissional_id=prof_amanda.id, dia_semana=4, hora_inicio="07:00", hora_fim="12:00", duracao_minutos=30),
            ]
            for horario in horarios_amanda:
                existing = db.query(HorarioProfissional).filter(
                    HorarioProfissional.profissional_id == horario.profissional_id,
                    HorarioProfissional.dia_semana == horario.dia_semana,
                    HorarioProfissional.hora_inicio == horario.hora_inicio
                ).first()
                if not existing:
                    db.add(horario)
        
        # Ft. Lucas Martins (Fisioterapia) - Segunda a Sexta (tarde) - consultas de 45 min
        prof_lucas = db.query(Profissional).filter(Profissional.registro == "CREFITO3-234567").first()
        if prof_lucas:
            horarios_lucas = [
                HorarioProfissional(profissional_id=prof_lucas.id, dia_semana=0, hora_inicio="13:00", hora_fim="18:00", duracao_minutos=45),
                HorarioProfissional(profissional_id=prof_lucas.id, dia_semana=1, hora_inicio="13:00", hora_fim="18:00", duracao_minutos=45),
                HorarioProfissional(profissional_id=prof_lucas.id, dia_semana=2, hora_inicio="13:00", hora_fim="18:00", duracao_minutos=45),
                HorarioProfissional(profissional_id=prof_lucas.id, dia_semana=3, hora_inicio="13:00", hora_fim="18:00", duracao_minutos=45),
                HorarioProfissional(profissional_id=prof_lucas.id, dia_semana=4, hora_inicio="13:00", hora_fim="18:00", duracao_minutos=45),
            ]
            for horario in horarios_lucas:
                existing = db.query(HorarioProfissional).filter(
                    HorarioProfissional.profissional_id == horario.profissional_id,
                    HorarioProfissional.dia_semana == horario.dia_semana,
                    HorarioProfissional.hora_inicio == horario.hora_inicio
                ).first()
                if not existing:
                    db.add(horario)
        
        # Dr. Gabriel Mendes (Odontologia) - Segunda, Quarta e Sexta - consultas de 20 min
        prof_gabriel = db.query(Profissional).filter(Profissional.registro == "CRO-SP-12345").first()
        if prof_gabriel:
            horarios_gabriel = [
                HorarioProfissional(profissional_id=prof_gabriel.id, dia_semana=0, hora_inicio="08:00", hora_fim="12:00", duracao_minutos=20),
                HorarioProfissional(profissional_id=prof_gabriel.id, dia_semana=0, hora_inicio="14:00", hora_fim="18:00", duracao_minutos=20),
                HorarioProfissional(profissional_id=prof_gabriel.id, dia_semana=2, hora_inicio="08:00", hora_fim="12:00", duracao_minutos=20),
                HorarioProfissional(profissional_id=prof_gabriel.id, dia_semana=4, hora_inicio="08:00", hora_fim="12:00", duracao_minutos=20),
            ]
            for horario in horarios_gabriel:
                existing = db.query(HorarioProfissional).filter(
                    HorarioProfissional.profissional_id == horario.profissional_id,
                    HorarioProfissional.dia_semana == horario.dia_semana,
                    HorarioProfissional.hora_inicio == horario.hora_inicio
                ).first()
                if not existing:
                    db.add(horario)
        
        # Dra. Camila Torres (Odontologia) - Terça, Quinta e Sábado - consultas de 30 min
        prof_camila = db.query(Profissional).filter(Profissional.registro == "CRO-SP-23456").first()
        if prof_camila:
            horarios_camila = [
                HorarioProfissional(profissional_id=prof_camila.id, dia_semana=1, hora_inicio="09:00", hora_fim="13:00", duracao_minutos=30),
                HorarioProfissional(profissional_id=prof_camila.id, dia_semana=1, hora_inicio="14:00", hora_fim="18:00", duracao_minutos=30),
                HorarioProfissional(profissional_id=prof_camila.id, dia_semana=3, hora_inicio="09:00", hora_fim="13:00", duracao_minutos=30),
                HorarioProfissional(profissional_id=prof_camila.id, dia_semana=5, hora_inicio="08:00", hora_fim="12:00", duracao_minutos=30),
            ]
            for horario in horarios_camila:
                existing = db.query(HorarioProfissional).filter(
                    HorarioProfissional.profissional_id == horario.profissional_id,
                    HorarioProfissional.dia_semana == horario.dia_semana,
                    HorarioProfissional.hora_inicio == horario.hora_inicio
                ).first()
                if not existing:
                    db.add(horario)
        
        db.commit()
        print("✅ Horários de atendimento configurados!")
        
        print("✨ Banco de dados populado com sucesso!")
        
    except Exception as e:
        print(f"❌ Erro ao popular banco de dados: {e}")
        db.rollback()
    finally:
        db.close()


if __name__ == "__main__":
    # Inicializar banco de dados
    init_db()
    
    # Popular com dados de exemplo
    popular_dados()
