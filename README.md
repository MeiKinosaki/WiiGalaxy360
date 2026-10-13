# WiiGalaxy360

Emulador experimental de Nintendo Wii para Xbox 360 RGH.

## Estado real — v2.17b: subfc e carry do XER

A v2.17b implementa `subfc` (XO=8), calculando RB − RA e atualizando CA do XER segundo a semântica carry-not-borrow: CA é definido quando RB ≥ RA e limpo quando ocorre empréstimo. Mantém `addc`, CR0 e as instruções anteriores. Testes adicionados cobrem subtração com e sem empréstimo, incluindo o resultado de 32 bits com wraparound. O diagnóstico inicial também executa `subfc`. A validação da v2.17b passou: testes de host, diagnóstico inicial, compilação/ligação Xenon e empacotamento/validação estrutural de `default.xex` concluídos com sucesso. Logs: https://github.com/MeiKinosaki/WiiGalaxy360/actions/runs/38298337507.

## Como executar

1. Abra a branch `v2.17b-xer-subfc`.
2. Vá para **Actions** → **Cloud Xenon toolchain**.
3. Execute **Run workflow** ou acompanhe o workflow disparado pela branch.
4. Abra https://github.com/MeiKinosaki/WiiGalaxy360/actions/runs/38298337507 e baixe o artefato `WiiGalaxy360-v2.17b-xer-subfc`.
5. O pacote contém `default.xex`, ELF, objetos e inventário da toolchain.

## O que o workflow valida

- Testes de host.
- Compilação dos arquivos atuais do núcleo com `xenon-g++`.
- Testes de host para `addi`, `stw`, `lwz`, reset, diagnósticos de falha, instruções X-form, CR0, `addc`, `subfc` e XER/CA.
- Rotina de startup que verifica `addi`, `add.` (Rc=1), `addc`, `subfc`, `stw`, `lwz`, CR0 e XER/CA.
- Empacotamento Xenon de um XEX de diagnóstico; a execução da v2.16b passou.
- Linkagem do smoke test e conversão/validação de ELF32 PowerPC.
- Execução do empacotador XEX externo em modo `--no-sign`.
- Cabeçalho `XEX2` e tamanho mínimo do arquivo resultante.

## Limites importantes

- `default.xex` nesta versão é um **XEX de diagnóstico sem assinatura e sem criptografia**, com uma rotina mínima que exercita parte do núcleo Broadway; não é um executável funcional do emulador Wii.
- A validação automatizada verifica os testes de host, a compilação e o contêiner/cabeçalho XEX2, mas não prova que o arquivo inicia em hardware Xbox 360 RGH.
- O modo `--no-sign` não produz a imagem assinada/criptografada exigida por um console retail/devkit padrão. O comportamento em um RGH depende do ambiente de execução e não foi testado aqui.
- O empacotador é obtido de um repositório externo e fixado em commit; consulte a licença e os avisos do projeto RXDK360 antes de reutilizá-lo ou redistribuí-lo.
- O núcleo continua experimental. A rotina de diagnóstico ainda precisa ser confirmada no ambiente Xenon e no console RGH; não há compatibilidade de jogos Wii demonstrada, nem implementação completa de GPU/Flipper, áudio, IOS e inicialização de jogos.

Referências:
- Toolchain libXenon: https://github.com/Free60Project/libxenon
- Empacotador RXDK360: https://github.com/Team-Resurgent/RXDK360

## v2.18b — fundação da biblioteca e interface

Esta etapa prepara a estrutura de dados para a biblioteca inspirada no layout Aurora, com paleta azul/branca, pastas de capas, ROMs, favoritos, save data, save states e músicas de abertura/menu. Configuração inicial: `config/ui_settings.json`; tema: `themes/aurora-blue-white.json`; favoritos começam vazios em `favorites/favorites.json`.

**Estado honesto:** esta etapa cria a estrutura e os arquivos de configuração, não a interface gráfica funcional nem o áudio. A detecção/carregamento de jogos, filtragem visual, reprodução de música e salvamento/restauração real ainda precisam ser integrados ao frontend e ao núcleo. A música deve ser fornecida pelo usuário ou ter licença apropriada. A tela de abertura inspirada no Wii e o tema sonoro selecionado ainda não estão implementados.

## v2.19b — protótipo interativo da biblioteca

Adicionado `ui/library.html`, um frontend demonstrativo em HTML/CSS/JavaScript com tema azul/branco, grade de capas ilustrativas, navegação lateral, pesquisa, ordenação, filtro por categoria, favoritos e painel de preferências gráficas. Favoritos, itens recentes e preferências usam armazenamento local do navegador quando disponível.

**Limites desta etapa:** os itens da grade são fictícios; não há descoberta de ROMs, carregamento de jogos, integração com o núcleo, renderização gráfica do emulador, áudio real nem save/load state. Os controles de qualidade são apenas preferências demonstrativas até serem ligados ao renderizador. O botão de áudio não reproduz música.

Para visualizar o protótipo, abra `ui/library.html` em um navegador moderno. Isto não significa que a interface já esteja integrada ao `default.xex` ou seja executável como app Xbox 360.

## v2.20b — importação de catálogo e capas no protótipo

A biblioteca web agora permite importar um catálogo JSON por arquivo e selecionar imagens de capa locais. Os metadados importados são guardados no armazenamento local do navegador quando há espaço; capas escolhidas são associadas nesta sessão pelo nome/ID do arquivo. Exemplo de formato: `config/catalog.example.json`.

Formato aceito: uma lista de jogos ou um objeto com `games: []`; cada item pode ter `id`, `name`/ `title`, `category`/ `platform`, `path`/ `file` e `cover`. Para associar uma capa, use o ID ou nome do jogo como nome-base do arquivo de imagem.

**Limites:** isso é importação de metadados no navegador, não varredura automática de pastas do Xbox, verificação do conteúdo dos discos, nem carregamento de ROM. A importação de imagens é temporária na sessão e não as grava na pasta `covers/`. A integração com o frontend nativo e o `default.xex` permanece pendente.

## v2.21b — persistência de capas e exportação do catálogo

O protótipo da biblioteca passa a converter capas selecionadas em Data URLs e guardá-las junto aos metadados no armazenamento local do navegador, quando a quota permite. Também adiciona exportação do catálogo em JSON (sem embutir as imagens no arquivo exportado) e atualiza os itens recentes ao abrir detalhes.

**Limitações:** armazenamento local pode ter quota restrita e não é armazenamento da pasta `covers/` do Xbox. O export JSON não inclui bytes das capas; mantenha os arquivos originais à parte. Navegação de controle físico e integração com o `default.xex` continuam pendentes. ROMs continuam sendo apenas registros de catálogo, não jogos carregáveis.


## v2.22b — navegação de teclado na biblioteca

A grade do protótipo agora anuncia a lista de jogos para tecnologias assistivas, usa foco visual mais evidente e oferece navegação por teclado entre os botões de detalhes dos cartões: setas para percorrer os itens, Home/End para ir ao primeiro/último item e Enter para ativar o botão focado. A captura das setas é suspensa quando o foco está em campos de entrada, pesquisa ou seleção.

**Limites:** isso é acessibilidade de teclado no protótipo HTML do navegador; não implementa mapeamento nativo do controle Xbox 360, navegação dentro de um XEX nem integração com o núcleo. Os jogos continuam não carregáveis, e nenhuma validação em hardware foi feita.


## v2.23b — navegação espacial refinada

A navegação por setas agora considera a posição visual dos cartões na grade responsiva, em vez de simplesmente avançar ou voltar na lista. Isso torna o movimento horizontal e vertical mais natural em diferentes larguras de tela. A navegação também identifica o cartão atual quando o foco está no botão de favorito, e Home/End levam ao primeiro/último cartão.

**Limites:** continua sendo navegação por teclado no protótipo HTML. Não foi implementado nem testado mapeamento do controle Xbox 360, integração ao default.xex, carregamento de ROM ou execução no console.


## v2.24b — persistência de pesquisa e filtros

A biblioteca do protótipo agora salva no armazenamento local do navegador a pesquisa digitada, o modo de ordenação, a categoria selecionada e a aba de biblioteca ativa (todos, favoritos ou recentes). Ao reabrir o protótipo no mesmo navegador, esses controles são restaurados quando os dados continuam disponíveis. O botão **Limpar filtros** redefine pesquisa, ordenação, categoria e aba para os valores padrão.

**Limites:** os dados ficam no navegador e podem ser apagados ou limitados pela quota; não são preferências gravadas no Xbox 360. Isso não implementa leitura de diretórios, carregamento de jogos, integração com `default.xex` ou suporte nativo ao controle. Não houve teste em hardware nesta etapa.


## v2.25b — validação de catálogo importado

O importador agora rejeita registros que não sejam objetos, tenham nome ausente, ID vazio ou ID duplicado (incluindo IDs já presentes nos itens demonstrativos). A mensagem de resultado informa quantos registros foram aceitos/rejeitados e mostra até cinco motivos de rejeição. Registros válidos continuam sendo apenas metadados; não há inspeção de arquivos ROM nem verificação de compatibilidade.

**Limites:** persistência em `localStorage` do navegador, sujeita a quota. Sem varredura de diretórios Xbox, carregamento de jogos, integração ao `default.xex`, controle físico ou teste em hardware.


## v2.26b — importação/exportação mais segura

O importador agora exige ID explícito, nome não vazio e categoria suportada (`wii` ou `test`), além de rejeitar IDs duplicados. A exportação gera um objeto com `schema_version: 1` e apenas registros importados, normalizando os campos `id`, `name`, `category`, `path` e `cover`; itens fictícios e imagens embutidas não são exportados. A importação continua aceitando uma lista JSON ou um objeto com `games`.

**Limites:** validação estrutural de metadados, não de conteúdo de ROM. O armazenamento continua no navegador; não há verificação de arquivos, carregamento de jogos, integração ao `default.xex`, controle nativo ou teste em hardware.


## v2.27b — recuperação de erros de armazenamento

O carregamento do `localStorage` agora verifica a estrutura básica dos valores usados pelo catálogo, favoritos, recentes, filtros e preferências. JSON ilegível ou valores com formato incompatível recorrem aos valores padrão, e a interface apresenta um aviso quando detecta dados inválidos ou falha de persistência. Falhas de quota/armazenamento são capturadas para evitar que a rotina de salvar interrompa a interface.

**Limites:** a recuperação não restaura automaticamente dados corrompidos nem substitui um backup; mantenha cópias exportadas do catálogo. Tudo continua no armazenamento do navegador, sem integração ao armazenamento do Xbox 360, `default.xex`, carregamento de ROMs ou teste em hardware.


## v2.28b — diagnóstico da biblioteca

O botão **Diagnóstico** verifica a estrutura básica dos registros em memória, IDs duplicados, listas de favoritos e recentes, quantidade de capas embutidas e disponibilidade básica de gravação/leitura no `localStorage`. Os resultados aparecem na interface.

**Limites:** diagnóstico de metadados e armazenamento do navegador apenas. Não abre nem valida ROMs, não confirma que caminhos existem, não testa o núcleo PowerPC/GPU, não compila `default.xex` e não representa teste no Xbox 360.


## v2.29b — importação sem apagar registros existentes

A importação de catálogo agora mescla os registros válidos novos aos itens já cadastrados, em vez de substituir todo o catálogo importado. IDs já presentes no catálogo são tratados como duplicados e rejeitados, evitando que uma importação parcial ou com IDs repetidos apague silenciosamente registros anteriores. A mensagem informa quantos itens novos foram aceitos e quantos foram rejeitados.

**Limites:** a mudança protege os metadados armazenados no navegador, mas não cria backup automático, não valida arquivos ROM e não liga a biblioteca ao núcleo de emulação ou ao `default.xex`. A interface segue sendo um protótipo web; sem teste automatizado ou validação em Xbox 360 nesta etapa.

## v2.30b — documentação: controles, configurações, saves e instalação

### Descrição do projeto

O **WiiGalaxy360** é um projeto experimental que pretende desenvolver um emulador de Nintendo Wii para Xbox 360 RGH. A interface atual `ui/library.html` é um protótipo web com tema azul e branco, biblioteca demonstrativa, capas ilustrativas, pesquisa, filtros, favoritos, recentes e preferências.

**Estado real:** a interface web não executa jogos Wii. O núcleo Broadway/PowerPC está incompleto; não há implementação completa de GPU/Flipper, áudio, IOS ou inicialização de jogos, e não há compatibilidade de jogos demonstrada. O `default.xex` disponível no fluxo histórico é um executável de diagnóstico, não o emulador completo.

### Controles da interface web

- **Tab / Shift+Tab:** avançar ou voltar entre controles focáveis.
- **Setas ↑ ↓ ← →:** navegar entre cartões da biblioteca quando o foco estiver em um cartão.
- **Home / End:** ir ao primeiro ou ao último cartão.
- **Enter:** ativar o botão focado e abrir os detalhes do registro.
- **Mouse:** clicar em botões, filtros e opções de importação.
- **Pesquisar:** filtrar registros pelo nome.
- **Todos os jogos / Favoritos / Recentes:** alternar a lista.
- **Limpar filtros:** restaurar pesquisa, ordenação e categoria.
- **Importar catálogo JSON:** importar metadados.
- **Importar capas:** selecionar imagens locais para associar a registros compatíveis.
- **Exportar catálogo:** salvar os registros importados em JSON.
- **Diagnóstico:** verificar a estrutura básica do catálogo e o armazenamento do navegador.

**Controle físico do Xbox 360:** ainda não há mapeamento nativo comprovado nem teste do controle dentro do XEX. Os comandos acima são apenas do protótipo web.

### Configurações (Settings)

Na aba **Configurações**, o protótipo exibe:

| Opção | Valores | O que realmente faz |
|---|---|---|
| Perfil gráfico | Desempenho, Equilibrado, Qualidade | Preferência demonstrativa; não altera o renderizador |
| Filtro de textura | Nearest, Linear | Preferência demonstrativa |
| Resolução | Automática, 640×480, 1280×720 | Seleção visual; não configura a saída do Xbox |
| Som do menu | Desligado/Ligado quando implementado | Não reproduz áudio atualmente |
| Botão de áudio no cabeçalho | Liga/desliga o estado visual | Não toca música |

O botão **Salvar preferências** grava essas escolhas no armazenamento local do navegador, quando disponível. Os dados podem ser perdidos ao limpar os dados do navegador e não são configurações nativas do Xbox 360.

### Saves — Save Data e Save States

- `save_data/`: diretório reservado para futuros dados persistentes de jogos.
- `save_states/`: diretório reservado para futuros estados instantâneos da emulação.

**Essas funções ainda não estão implementadas.** O emulador não grava nem restaura saves de jogos Wii ou save states. O protótipo guarda somente metadados da biblioteca, favoritos, recentes, filtros e preferências no `localStorage` do navegador. Isso não é um save de jogo. Use **Exportar catálogo** para criar uma cópia dos metadados; capas não são incluídas no JSON exportado.

### Como instalar / visualizar

#### Abrir a interface de demonstração no computador

1. Abra o repositório no GitHub e selecione a branch desejada.
2. Baixe o ZIP da branch e extraia os arquivos.
3. Abra `ui/library.html` em um navegador moderno.
4. Use **Importar catálogo JSON** para metadados e **Importar capas** para selecionar imagens.

Isso abre apenas o protótipo no navegador; não instala o emulador no Xbox 360 nem executa jogos.

#### Consultar/compilar o XEX experimental

1. No GitHub, abra **Actions** e localize o workflow de toolchain Xenon disponível.
2. Se houver execução manual, use **Run workflow** na branch desejada; caso contrário, consulte as execuções disparadas por commits.
3. Abra a execução concluída e confira os logs e artefatos.
4. Baixe apenas o artefato correspondente à versão desejada e leia suas notas.

Nem toda branch produz um novo artefato automaticamente. O XEX histórico é experimental, sem assinatura/criptografia, e a execução em hardware RGH não foi confirmada. Não existe ainda um pacote de instalação de um emulador Wii completo.

### Estrutura de pastas planejada

```text
WiiGalaxy360/
├── config/          # preferências e exemplos de catálogo
├── covers/          # capas (estrutura planejada)
├── favorites/       # favoritos planejados
├── games/roms/      # diretório reservado para ROMs próprias
├── save_data/       # reservado para saves de jogos
├── save_states/     # reservado para estados de emulação
├── music/startup/   # áudio próprio/licenciado
├── music/menu/      # áudio próprio/licenciado
├── themes/          # tema azul e branco
└── ui/library.html  # protótipo web
```

A presença das pastas não significa que o XEX já as leia. O suporte real a controles, saves, áudio, diretórios e carregamento de jogos depende de implementação futura.

### Avisos

- O catálogo contém metadados; não carrega nem valida arquivos de jogos.
- Use apenas jogos e conteúdos para os quais tenha direito de uso.
- Não inclua músicas, capas ou outros materiais protegidos sem permissão.
- Não distribua o XEX de diagnóstico como se fosse uma versão funcional do emulador.

