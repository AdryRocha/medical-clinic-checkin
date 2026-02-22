"""
Dados mockados para testar o bot sem a API
"""
import json
from datetime import datetime, timedelta
from typing import Dict, Any, List, Optional

from mock.mock_qrcode_service import prepare_qrcode_data

# Dados simulados de pacientes
PACIENTES_MOCK = {
    '12345678901': {
        'id': 1,
        'nome': 'João Silva',
        'cpf': '12345678901',
        'aceita_digital': True,
        'digital': None
    },
    '98765432100': {
        'id': 2,
        'nome': 'Maria Santos',
        'cpf': '98765432100',
        'aceita_digital': False,
        'digital': None
    }
}

# Categorias/Especialidades médicas
CATEGORIAS_MOCK = [
    {'id': 1, 'nome': 'Cardiologia'},
    {'id': 2, 'nome': 'Dermatologia'},
    {'id': 3, 'nome': 'Ortopedia'},
    {'id': 4, 'nome': 'Pediatria'},
    {'id': 5, 'nome': 'Ginecologia'},
    {'id': 6, 'nome': 'Psiquiatria'},
    {'id': 7, 'nome': 'Nutrição'},
    {'id': 8, 'nome': 'Fisioterapia'}
]

# Dados simulados de profissionais
PROFISSIONAIS_MOCK = [
    {
        'id': 1,
        'nome': 'Dr. Carlos Souza',
        'categoria_id': 1,
        'registro': 'CRM 12345-SP'
    },
    {
        'id': 2,
        'nome': 'Dra. Ana Lima',
        'categoria_id': 2,
        'registro': 'CRM 23456-SP'
    },
    {
        'id': 3,
        'nome': 'Dr. Pedro Costa',
        'categoria_id': 3,
        'registro': 'CRM 34567-SP'
    },
    {
        'id': 4,
        'nome': 'Dra. Julia Rocha',
        'categoria_id': 4,
        'registro': 'CRM 45678-SP'
    },
    {
        'id': 5,
        'nome': 'Dra. Beatriz Santos',
        'categoria_id': 5,
        'registro': 'CRM 56789-SP'
    },
    {
        'id': 6,
        'nome': 'Dr. Rafael Oliveira',
        'categoria_id': 6,
        'registro': 'CRM 67890-SP'
    },
    {
        'id': 7,
        'nome': 'Fernanda Costa',
        'categoria_id': 7,
        'registro': 'CRN 12345-3'
    },
    {
        'id': 8,
        'nome': 'Lucas Martins',
        'categoria_id': 8,
        'registro': 'CREFITO 12345-F'
    }
]

# Horários disponíveis padrão por profissional (agenda semanal fixa)
# dia_semana: 0=segunda, 1=terça, 2=quarta, 3=quinta, 4=sexta, 5=sábado, 6=domingo (padrão Python weekday())
HORARIOS_DISPONIVEIS_MOCK = [
    # Dr. Carlos - Cardiologia (Segunda, Quarta e Sexta)
    {'id': 1, 'profissional_id': 1, 'dia_semana': 0, 'hora_inicio': '08:00', 'hora_fim': '12:00', 'duracao_minutos': 30},
    {'id': 2, 'profissional_id': 1, 'dia_semana': 0, 'hora_inicio': '14:00', 'hora_fim': '17:00', 'duracao_minutos': 30},
    {'id': 3, 'profissional_id': 1, 'dia_semana': 2, 'hora_inicio': '08:00', 'hora_fim': '12:00', 'duracao_minutos': 30},
    {'id': 4, 'profissional_id': 1, 'dia_semana': 4, 'hora_inicio': '08:00', 'hora_fim': '12:00', 'duracao_minutos': 30},
    
    # Dra. Ana - Dermatologia (Segunda a Sexta)
    {'id': 5, 'profissional_id': 2, 'dia_semana': 0, 'hora_inicio': '08:30', 'hora_fim': '11:30', 'duracao_minutos': 30},
    {'id': 6, 'profissional_id': 2, 'dia_semana': 1, 'hora_inicio': '08:30', 'hora_fim': '11:30', 'duracao_minutos': 30},
    {'id': 7, 'profissional_id': 2, 'dia_semana': 2, 'hora_inicio': '14:30', 'hora_fim': '16:30', 'duracao_minutos': 30},
    {'id': 8, 'profissional_id': 2, 'dia_semana': 3, 'hora_inicio': '08:30', 'hora_fim': '11:30', 'duracao_minutos': 30},
    {'id': 9, 'profissional_id': 2, 'dia_semana': 4, 'hora_inicio': '08:30', 'hora_fim': '11:30', 'duracao_minutos': 30},
    
    # Dr. Pedro - Ortopedia (Terça, Quinta e Sábado)
    {'id': 10, 'profissional_id': 3, 'dia_semana': 1, 'hora_inicio': '09:00', 'hora_fim': '12:00', 'duracao_minutos': 30},
    {'id': 11, 'profissional_id': 3, 'dia_semana': 1, 'hora_inicio': '14:00', 'hora_fim': '17:00', 'duracao_minutos': 30},
    {'id': 12, 'profissional_id': 3, 'dia_semana': 3, 'hora_inicio': '09:00', 'hora_fim': '12:00', 'duracao_minutos': 30},
    {'id': 13, 'profissional_id': 3, 'dia_semana': 5, 'hora_inicio': '08:00', 'hora_fim': '12:00', 'duracao_minutos': 30},
    
    # Dra. Julia - Pediatria (Segunda a Sexta - manhã)
    {'id': 14, 'profissional_id': 4, 'dia_semana': 0, 'hora_inicio': '08:00', 'hora_fim': '11:00', 'duracao_minutos': 20},
    {'id': 15, 'profissional_id': 4, 'dia_semana': 1, 'hora_inicio': '08:00', 'hora_fim': '11:00', 'duracao_minutos': 20},
    {'id': 16, 'profissional_id': 4, 'dia_semana': 2, 'hora_inicio': '08:00', 'hora_fim': '11:00', 'duracao_minutos': 20},
    {'id': 17, 'profissional_id': 4, 'dia_semana': 3, 'hora_inicio': '08:00', 'hora_fim': '11:00', 'duracao_minutos': 20},
    {'id': 18, 'profissional_id': 4, 'dia_semana': 4, 'hora_inicio': '08:00', 'hora_fim': '11:00', 'duracao_minutos': 20},
    
    # Dra. Beatriz - Ginecologia (Segunda, Quarta e Sexta)
    {'id': 19, 'profissional_id': 5, 'dia_semana': 0, 'hora_inicio': '08:00', 'hora_fim': '12:00', 'duracao_minutos': 40},
    {'id': 20, 'profissional_id': 5, 'dia_semana': 0, 'hora_inicio': '14:00', 'hora_fim': '16:00', 'duracao_minutos': 40},
    {'id': 21, 'profissional_id': 5, 'dia_semana': 2, 'hora_inicio': '08:00', 'hora_fim': '12:00', 'duracao_minutos': 40},
    {'id': 22, 'profissional_id': 5, 'dia_semana': 4, 'hora_inicio': '08:00', 'hora_fim': '12:00', 'duracao_minutos': 40},
    
    # Dr. Rafael - Psiquiatria (Segunda a Sexta - tarde)
    {'id': 23, 'profissional_id': 6, 'dia_semana': 0, 'hora_inicio': '14:00', 'hora_fim': '18:00', 'duracao_minutos': 60},
    {'id': 24, 'profissional_id': 6, 'dia_semana': 1, 'hora_inicio': '14:00', 'hora_fim': '18:00', 'duracao_minutos': 60},
    {'id': 25, 'profissional_id': 6, 'dia_semana': 2, 'hora_inicio': '14:00', 'hora_fim': '18:00', 'duracao_minutos': 60},
    {'id': 26, 'profissional_id': 6, 'dia_semana': 3, 'hora_inicio': '14:00', 'hora_fim': '18:00', 'duracao_minutos': 60},
    {'id': 27, 'profissional_id': 6, 'dia_semana': 4, 'hora_inicio': '14:00', 'hora_fim': '18:00', 'duracao_minutos': 60},
    
    # Fernanda - Nutricionista (Terça, Quinta e Sábado)
    {'id': 28, 'profissional_id': 7, 'dia_semana': 1, 'hora_inicio': '08:00', 'hora_fim': '11:00', 'duracao_minutos': 45},
    {'id': 29, 'profissional_id': 7, 'dia_semana': 1, 'hora_inicio': '13:00', 'hora_fim': '16:00', 'duracao_minutos': 45},
    {'id': 30, 'profissional_id': 7, 'dia_semana': 3, 'hora_inicio': '08:00', 'hora_fim': '11:00', 'duracao_minutos': 45},
    {'id': 31, 'profissional_id': 7, 'dia_semana': 5, 'hora_inicio': '08:00', 'hora_fim': '12:00', 'duracao_minutos': 45},
    
    # Lucas - Fisioterapeuta (Segunda a Sábado)
    {'id': 32, 'profissional_id': 8, 'dia_semana': 0, 'hora_inicio': '07:00', 'hora_fim': '11:00', 'duracao_minutos': 50},
    {'id': 33, 'profissional_id': 8, 'dia_semana': 0, 'hora_inicio': '13:00', 'hora_fim': '17:00', 'duracao_minutos': 50},
    {'id': 34, 'profissional_id': 8, 'dia_semana': 1, 'hora_inicio': '07:00', 'hora_fim': '11:00', 'duracao_minutos': 50},
    {'id': 35, 'profissional_id': 8, 'dia_semana': 2, 'hora_inicio': '13:00', 'hora_fim': '17:00', 'duracao_minutos': 50},
    {'id': 36, 'profissional_id': 8, 'dia_semana': 3, 'hora_inicio': '07:00', 'hora_fim': '11:00', 'duracao_minutos': 50},
    {'id': 37, 'profissional_id': 8, 'dia_semana': 4, 'hora_inicio': '07:00', 'hora_fim': '11:00', 'duracao_minutos': 50},
    {'id': 38, 'profissional_id': 8, 'dia_semana': 5, 'hora_inicio': '08:00', 'hora_fim': '12:00', 'duracao_minutos': 50},
]

# Contador para IDs de consultas e pacientes
_next_consulta_id = 1
_next_paciente_id = len(PACIENTES_MOCK) + 1

# Armazenar consultas agendadas
CONSULTAS_AGENDADAS = []


def gerar_slots_horarios(hora_inicio: str, hora_fim: str, duracao_minutos: int) -> List[str]:
    """
    Gera lista de horários baseado no intervalo e duração.
    Ex: ('08:00', '10:00', 30) -> ['08:00', '08:30', '09:00', '09:30']
    """
    from datetime import datetime, timedelta
    
    formato = '%H:%M'
    inicio = datetime.strptime(hora_inicio, formato)
    fim = datetime.strptime(hora_fim, formato)
    
    slots = []
    horario_atual = inicio
    
    while horario_atual < fim:
        slots.append(horario_atual.strftime(formato))
        horario_atual += timedelta(minutes=duracao_minutos)
    
    return slots


class MockAPIClient:
    """Cliente simulado da API para testes"""
    
    def __init__(self, secret_key: str = "mock_secret_key_12345"):
        """
        Inicializa o cliente mock com chave secreta para QR codes.
        
        Args:
            secret_key: Chave secreta para geração de hash de validação
        """
        self.secret_key = secret_key
    
    async def buscar_paciente_por_cpf(self, cpf: str) -> Dict[str, Any]:
        """Busca paciente por CPF nos dados mockados"""
        if cpf in PACIENTES_MOCK:
            return {
                'success': True,
                'data': PACIENTES_MOCK[cpf]
            }
        return {
            'success': True,
            'data': None
        }
    
    async def cadastrar_paciente(self, nome: str, cpf: str, aceita_digital: bool = False) -> Dict[str, Any]:
        """Cadastra novo paciente nos dados mockados"""
        global _next_paciente_id
        
        if cpf in PACIENTES_MOCK:
            return {
                'success': False,
                'error': 'CPF já cadastrado'
            }
        
        novo_paciente = {
            'id': _next_paciente_id,
            'nome': nome,
            'cpf': cpf,
            'aceita_digital': aceita_digital,
            'digital': None
        }
        PACIENTES_MOCK[cpf] = novo_paciente
        _next_paciente_id += 1
        
        return {
            'success': True,
            'data': novo_paciente
        }
    
    async def buscar_categorias(self) -> Dict[str, Any]:
        """Retorna lista de categorias/especialidades disponíveis"""
        return {
            'success': True,
            'data': CATEGORIAS_MOCK
        }
    
    async def buscar_profissionais(self, categoria_id: Optional[int] = None) -> Dict[str, Any]:
        """Retorna lista de profissionais mockados, opcionalmente filtrados por categoria_id"""
        profissionais = PROFISSIONAIS_MOCK.copy()
        
        if categoria_id:
            profissionais = [
                p for p in PROFISSIONAIS_MOCK 
                if p['categoria_id'] == categoria_id
            ]
        
        return {
            'success': True,
            'data': profissionais
        }
    
    async def buscar_horarios_disponiveis(self, profissional_id: int, data: str) -> Dict[str, Any]:
        """
        Retorna horários disponíveis para um profissional em uma data específica.
        
        Args:
            profissional_id: ID do profissional
            data: Data no formato 'YYYY-MM-DD'
        
        Returns:
            Lista de horários disponíveis (não ocupados por consultas)
        """
        try:
            # Converter data string para datetime e pegar dia da semana (padrão Python)
            data_obj = datetime.strptime(data, '%Y-%m-%d')
            dia_semana = data_obj.weekday()  # 0=segunda, 1=terça, ..., 6=domingo
            
            # Buscar horários padrão do profissional para esse dia da semana
            horarios_padrao = [
                h for h in HORARIOS_DISPONIVEIS_MOCK
                if h['profissional_id'] == profissional_id and h['dia_semana'] == dia_semana
            ]
            
            if not horarios_padrao:
                return {
                    'success': True,
                    'data': []
                }
            
            # Gerar todos os slots possíveis
            todos_slots = []
            for periodo in horarios_padrao:
                slots = gerar_slots_horarios(
                    periodo['hora_inicio'],
                    periodo['hora_fim'],
                    periodo['duracao_minutos']
                )
                todos_slots.extend(slots)
            
            # Buscar horários já ocupados nessa data
            horarios_ocupados = set()
            for consulta in CONSULTAS_AGENDADAS:
                if (consulta['profissional_id'] == profissional_id and 
                    consulta['data'] == data and
                    consulta['status'] in ['agendada']):
                    horarios_ocupados.add(consulta['horario'])
            
            # Filtrar horários disponíveis
            horarios_disponiveis = [
                {'hora': h} for h in todos_slots 
                if h not in horarios_ocupados
            ]
            
            return {
                'success': True,
                'data': horarios_disponiveis
            }
        
        except Exception as e:
            return {
                'success': False,
                'error': f'Erro ao buscar horários: {str(e)}'
            }
    
    async def agendar_consulta(self, dados: Dict[str, Any]) -> Dict[str, Any]:
        """Simula o agendamento de uma consulta"""
        global _next_consulta_id
        
        try:
            # Buscar dados do paciente
            paciente_id = dados.get('paciente_id')
            paciente = None
            if paciente_id:
                for cpf, p in PACIENTES_MOCK.items():
                    if p['id'] == paciente_id:
                        paciente = p
                        break
            
            # Buscar dados do profissional
            profissional = next(
                (p for p in PROFISSIONAIS_MOCK if p['id'] == dados['profissional_id']), 
                None
            )
            
            # Criar consulta
            consulta = {
                'id': _next_consulta_id,
                'paciente_id': paciente_id,
                'profissional_id': dados['profissional_id'],
                'data': dados['data'],
                'horario': dados['horario'],
                'status': 'agendada',
                'created_at': datetime.now().isoformat()
            }
            
            CONSULTAS_AGENDADAS.append(consulta)
            _next_consulta_id += 1
            
            if paciente:
                qr_code_data = prepare_qrcode_data(consulta, paciente, self.secret_key)
            else:
                qr_code_data = {
                    'cmd': 'checkin',
                    'appt_id': consulta['id'],
                    'cpf': '',
                    'name': '',
                    'hash': ''
                }
            
            return {
                'success': True,
                'consulta_id': consulta['id'],
                'qr_code': qr_code_data,
                'data': consulta
            }
        
        except KeyError as e:
            return {
                'success': False,
                'error': f'Dados incompletos: {str(e)}'
            }
    
    async def buscar_consulta(self, consulta_id: int) -> Dict[str, Any]:
        """Busca uma consulta agendada"""
        for consulta in CONSULTAS_AGENDADAS:
            if consulta['id'] == consulta_id:
                return {
                    'success': True,
                    'data': consulta
                }
        
        return {
            'success': False,
            'error': 'Consulta não encontrada'
        }

    async def buscar_consultas_por_cpf(self, cpf: str) -> Dict[str, Any]:
        """Busca todas as consultas de um paciente pelo CPF"""
        # Buscar paciente
        paciente = PACIENTES_MOCK.get(cpf)
        if not paciente:
            return {
                'success': True,
                'data': []
            }
        
        # Buscar consultas do paciente
        consultas_paciente = []
        for consulta in CONSULTAS_AGENDADAS:
            if consulta['paciente_id'] == paciente['id']:
                # Enriquecer com dados do profissional
                profissional = next(
                    (p for p in PROFISSIONAIS_MOCK if p['id'] == consulta['profissional_id']),
                    None
                )
                categoria = None
                if profissional:
                    categoria = next(
                        (c for c in CATEGORIAS_MOCK if c['id'] == profissional['categoria_id']),
                        None
                    )
                
                consulta_completa = consulta.copy()
                if profissional:
                    consulta_completa['profissional'] = profissional.copy()
                    if categoria:
                        consulta_completa['profissional']['categoria'] = categoria
                
                consultas_paciente.append(consulta_completa)
        
        return {
            'success': True,
            'data': consultas_paciente
        }
    
    async def gerar_qrcode(self, consulta_id: int) -> Dict[str, Any]:
        """Gera (ou regenera) QR code para uma consulta específica"""
        # Buscar consulta
        consulta = None
        for c in CONSULTAS_AGENDADAS:
            if c['id'] == consulta_id:
                consulta = c
                break
        
        if not consulta:
            return {
                'success': False,
                'error': 'Consulta não encontrada'
            }
        
        # Buscar dados do paciente
        paciente = None
        for cpf, p in PACIENTES_MOCK.items():
            if p['id'] == consulta['paciente_id']:
                paciente = p
                break
        
        if not paciente:
            return {
                'success': False,
                'error': 'Paciente não encontrado'
            }
        
        # Gerar QR code
        qr_code_data = prepare_qrcode_data(consulta, paciente, self.secret_key)
        
        return {
            'success': True,
            'data': {
                'qr_code': qr_code_data
            }
        }
