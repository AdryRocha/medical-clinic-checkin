import logging
import aiohttp
import json
from typing import Dict, Any, Optional

from core.config import Config

logger = logging.getLogger(__name__)


class APIClient:
    """
    Cliente HTTP assíncrono para comunicação com a API REST.
    """

    def __init__(self, base_url: str = None, username: str = None, password: str = None, secret_key: Optional[str] = None, config: Optional[Config] = None):
        self.config = config or Config()
        self.base_url = (base_url or self.config.API_BASE_URL).rstrip('/')
        self.username = username or self.config.API_USERNAME
        self.password = password or self.config.API_PASSWORD
        self.secret_key = secret_key or self.config.QR_SECRET_KEY
        self.session: Optional[aiohttp.ClientSession] = None
        self.jwt_token: Optional[str] = None

    async def __aenter__(self):
        """Inicializa sessão HTTP ao entrar no contexto."""
        self.session = aiohttp.ClientSession()
        await self.authenticate()
        return self

    async def __aexit__(self, exc_type, exc_val, exc_tb):
        """Fecha sessão HTTP ao sair no contexto."""
        if self.session:
            await self.session.close()

    async def authenticate(self):
        """
        Autentica na API e obtém token JWT.
        """
        if not self.session:
            self.session = aiohttp.ClientSession()
        
        try:
            url = f"{self.base_url}/auth/token"
            data = {
                "username": self.username,
                "password": self.password
            }
            
            async with self.session.post(url, json=data) as response:
                if response.status == 200:
                    result = await response.json()
                    self.jwt_token = result.get('access_token')
                    logger.info("Autenticação bem-sucedida na API")
                    return True
                else:
                    error_text = await response.text()
                    logger.error(f"Falha na autenticação: {response.status} - {error_text}")
                    return False
        except Exception as e:
            logger.error(f"Erro ao autenticar na API: {e}")
            return False

    def _get_headers(self) -> Dict[str, str]:
        """
        Constrói headers HTTP para requisições.

        Returns:
            Dicionário com headers incluindo autenticação JWT se disponível
        """
        headers = {'Content-Type': 'application/json'}
        if self.jwt_token:
            headers['Authorization'] = f'Bearer {self.jwt_token}'
        return headers

    async def _make_request(self, method: str, endpoint: str, data: Optional[Dict] = None) -> Dict[str, Any]:
        """
        Realiza uma requisição HTTP, criando sessão temporária se necessário.

        Args:
            method: Método HTTP (GET, POST, etc.)
            endpoint: Endpoint da API (ex: '/pacientes')
            data: Dados a enviar no corpo da requisição (opcional)

        Returns:
            Dicionário com 'success' e 'data' ou 'error'
        """
        if not self.session:
            async with aiohttp.ClientSession() as session:
                self.session = session
                await self.authenticate()
                result = await self._make_request_with_session(session, method, endpoint, data)
                self.session = None
                return result
        
        if not self.jwt_token:
            await self.authenticate()

        return await self._make_request_with_session(self.session, method, endpoint, data)

    async def _make_request_with_session(self, session: aiohttp.ClientSession, method: str, endpoint: str, data: Optional[Dict] = None) -> Dict[str, Any]:
        """
        Executa requisição HTTP usando uma sessão específica.

        Args:
            session: Sessão aiohttp a usar
            method: Método HTTP
            endpoint: Endpoint da API
            data: Dados opcionais para envio

        Returns:
            Dicionário com resultado da requisição
        """
        url = f"{self.base_url}{endpoint}"
        headers = self._get_headers()

        try:
            if data:
                async with session.request(method, url, headers=headers, json=data) as response:
                    return await self._handle_response(response)
            else:
                async with session.request(method, url, headers=headers) as response:
                    return await self._handle_response(response)

        except aiohttp.ClientError as e:
            logger.error(f"Erro de conexão com API: {e}")
            return {'success': False, 'error': f'Erro de conexão: {str(e)}'}

    async def _handle_response(self, response: aiohttp.ClientResponse) -> Dict[str, Any]:
        """
        Processa resposta HTTP da API.

        Args:
            response: Resposta HTTP recebida

        Returns:
            Dicionário com status de sucesso e dados ou erro
        """
        try:
            if response.status in (200, 201):
                data = await response.json()
                return {'success': True, 'data': data}
            else:
                error_text = await response.text()
                logger.error(f"API retornou erro {response.status}: {error_text}")
                return {'success': False, 'error': f'Erro {response.status}: {error_text}'}

        except json.JSONDecodeError:
            return {'success': False, 'error': 'Resposta inválida da API'}

    async def buscar_paciente_por_cpf(self, cpf: str) -> Dict[str, Any]:
        """
        Busca paciente na API pelo CPF.

        Args:
            cpf: CPF do paciente (apenas números)

        Returns:
            Dicionário com dados do paciente ou erro
        """
        return await self._make_request('GET', f'/pacientes/cpf/{cpf}')

    async def cadastrar_paciente(self, nome: str, cpf: str, aceita_digital: bool = False) -> Dict[str, Any]:
        """
        Cadastra novo paciente na API.

        Args:
            nome: Nome completo do paciente
            cpf: CPF do paciente (apenas números)
            aceita_digital: Se aceita biometria digital

        Returns:
            Dicionário com dados do paciente cadastrado ou erro
        """
        data = {'nome': nome, 'cpf': cpf, 'aceita_digital': aceita_digital}
        return await self._make_request('POST', '/pacientes', data)

    async def buscar_categorias(self) -> Dict[str, Any]:
        """
        Busca lista de categorias/especialidades disponíveis.

        Returns:
            Dicionário com lista de categorias ou erro
        """
        return await self._make_request('GET', '/categorias')

    async def buscar_profissionais(self, categoria_id: Optional[int] = None) -> Dict[str, Any]:
        """
        Busca lista de profissionais disponíveis, opcionalmente filtrados por categoria.

        Args:
            categoria_id: ID da categoria para filtrar (opcional)

        Returns:
            Dicionário com lista de profissionais ou erro
        """
        endpoint = '/profissionais'
        if categoria_id:
            endpoint += f'?categoria_id={categoria_id}'
        return await self._make_request('GET', endpoint)

    async def buscar_horarios_disponiveis(self, profissional_id: int, data: str) -> Dict[str, Any]:
        """
        Busca horários disponíveis para um profissional em uma data específica.

        Args:
            profissional_id: ID do profissional
            data: Data no formato 'YYYY-MM-DD'

        Returns:
            Dicionário com horários disponíveis ou erro
        """
        endpoint = f'/horarios/disponiveis/{profissional_id}/{data}'
        logger.info(f"Buscando horários: profissional_id={profissional_id}, data={data}, endpoint={endpoint}")
        return await self._make_request('GET', endpoint)

    async def agendar_consulta(self, dados: Dict[str, Any]) -> Dict[str, Any]:
        """
        Agenda uma consulta e gera QR code para check-in.

        Args:
            dados: Dicionário com dados da consulta (paciente_id, profissional_id, data, horario, etc.)

        Returns:
            Dicionário com ID da consulta e QR code ou erro
        """
        try:
            consulta_data = {
                'paciente_id': dados.get('paciente_id'),
                'cpf': dados['cpf'],
                'nome': dados['nome'],
                'profissional_id': dados['profissional_id'],
                'data': dados['data'],
                'horario': dados['horario']
            }

            consulta_result = await self._make_request('POST', '/consultas', consulta_data)
            if not consulta_result['success']:
                return consulta_result

            consulta = consulta_result['data']
            qr_result = await self._make_request('GET', f'/qrcode/generate/{consulta["id"]}')

            if qr_result['success']:
                return {
                    'success': True,
                    'consulta_id': consulta['id'],
                    'qr_code': qr_result['data']['qr_code']
                }
            else:
                return qr_result

        except KeyError as e:
            return {'success': False, 'error': f'Dados incompletos: {str(e)}'}

    async def buscar_consulta(self, consulta_id: int) -> Dict[str, Any]:
        """
        Busca dados de uma consulta específica.

        Args:
            consulta_id: ID da consulta

        Returns:
            Dicionário com dados da consulta ou erro
        """
        return await self._make_request('GET', f'/consultas/{consulta_id}')
    
    async def buscar_consultas_por_cpf(self, cpf: str) -> Dict[str, Any]:
        """
        Busca todas as consultas de um paciente pelo CPF.
        
        Primeiro busca o paciente pelo CPF para obter o ID,
        depois busca as consultas usando o paciente_id.

        Args:
            cpf: CPF do paciente (apenas números)

        Returns:
            Dicionário com lista de consultas ou erro
        """
        # Primeiro busca o paciente pelo CPF
        paciente_result = await self.buscar_paciente_por_cpf(cpf)
        
        if not paciente_result['success']:
            return paciente_result
        
        paciente = paciente_result['data']
        paciente_id = paciente.get('id')
        
        if not paciente_id:
            return {'success': False, 'error': 'Paciente encontrado mas sem ID'}
        
        # Agora busca as consultas do paciente
        return await self._make_request('GET', f'/consultas?paciente_id={paciente_id}')

    async def gerar_qrcode(self, consulta_id: int) -> Dict[str, Any]:
        """
        Gera (ou regenera) QR code para uma consulta específica.

        Args:
            consulta_id: ID da consulta

        Returns:
            Dicionário com QR code em base64 ou erro
        """
        return await self._make_request('GET', f'/qrcode/generate/{consulta_id}')
