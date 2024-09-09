# 💾 CMR Cache - Servidor de Cache Alternativo ao Redis

**CMR Cache** é um servidor de cache open-source projetado para oferecer uma solução rápida e leve para armazenamento temporário de dados em memória. Ele foi criado para ser uma alternativa ao Redis, com foco em simplicidade e flexibilidade, ideal para pequenos projetos ou comunidades que precisam de um cache eficiente, mas sem a complexidade do Redis.

## 🚀 Funcionalidades

- 🔥 **Alto desempenho**: Respostas rápidas para operações de leitura e escrita.
- 📦 **Armazenamento em memória**: Dados mantidos em memória volátil para acesso ultrarrápido.
- 🧩 **Comandos simples**: Suporte aos principais comandos de cache, como `SET`, `GET`, `DEL`.
- 🔒 **Autenticação básica**: Controle de acesso via autenticação com usuário e senha.
- 📜 **Persistência opcional**: Opção de persistir dados no disco para cenários de cache que exigem durabilidade.
- 🌐 **Compatibilidade**: Suporte para integração com diversas linguagens.

## 🛠️ Instalação

### Pré-requisitos

- [Clang++](https://clang.llvm.org/get_started.html)
- [Boost C++ Libraries](https://www.boost.org/)

### Clonando o repositório

```bash
git clone https://github.com/camargo2019/cmr-cache.git
cd cmr-cache
```

### Instalando dependências

No Ubuntu, você pode instalar as dependências necessárias com:

```bash
sudo apt-get install libboost-all-dev
```

### Compilando

Para compilar o projeto no Ubuntu, use o seguinte comando:

```bash
clang++ -o cmr_cache main.cpp -I./vendor/yaml -I/usr/local/include/boost -L/usr/local/lib -lboost_system -lpthread -Wmissing-declarations
```

## 🎮 Uso

### Executando o servidor

Para iniciar o servidor CMR Cache, basta rodar o seguinte comando:

```bash
./cmr_cache
```

Por padrão, o servidor escutará na porta `6347` na sua máquina local. Se desejar, você pode alterar a porta e outras configurações em `config/host.yaml`.

### Exemplo de uso com Python

Aqui está um exemplo simples de como interagir com o servidor CMR Cache utilizando Python:

```python
import socket

# Conectando ao servidor CMR Cache
HOST = 'localhost'
PORT = 6347

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
    sock.connect((HOST, PORT))
    
    # Autenticando
    sock.sendall(b'AUTH admin admin\r\n')
    response = sock.recv(1024)
    print('Response:', response.decode())

    # Definindo um valor no cache
    sock.sendall(b'SET mykey myvalue 3600\r\n')
    response = sock.recv(1024)
    print('Response:', response.decode())

    # Obtendo um valor do cache
    sock.sendall(b'GET mykey\r\n')
    response = sock.recv(1024)
    print('Response:', response.decode())
```

## 📚 Comandos Suportados

| Comando                | Descrição                                                 |
|------------------------|-----------------------------------------------------------|
| `SET key value`         | Define um valor para a chave                             |
| `SET key value expire`  | Define um valor com tempo de expiração                   |
| `GET key`               | Retorna o valor associado à chave                        |
| `DEL key`               | Remove uma chave                                         |
| `AUTH user password`    | Autentica o usuário com a senha                          |
| `USE database`          | Especifica qual banco de dados será utilizado            |
| `KEYS`                  | Lista todas as chaves registradas                        |

## 🌟 Contribuindo

Contribuições são bem-vindas! Sinta-se à vontade para abrir issues ou enviar pull requests. Agradecemos qualquer tipo de contribuição para tornar o CMR Cache melhor para todos.

### Como contribuir

1. Faça um fork do repositório.
2. Crie uma branch para sua feature (`git checkout -b feat/minha-feature`).
3. Commit suas mudanças (`git commit -m 'feat: Adiciona nova feature'`).
4. Envie seu código para o repositório (`git push origin feat/minha-feature`).
5. Abra um Pull Request.

## 📝 Licença

Este projeto está licenciado sob a [MIT License](LICENSE).
