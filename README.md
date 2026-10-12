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
