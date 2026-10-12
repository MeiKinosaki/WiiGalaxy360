# WiiGalaxy360

Emulador experimental de Nintendo Wii para Xbox 360 RGH.

## Estado real — v2.12b: diagnóstico de inicialização do núcleo

A v2.11b gerou na nuvem um XEX de smoke test de 331.776 bytes com cabeçalho XEX2 validado. A v2.12b substitui o `main()` vazio por um diagnóstico determinístico: carrega quatro instruções na memória, executa `addi`, `stw` e `lwz` no núcleo Broadway e confere o valor de retorno. O workflow recompila, empacota e publica os artefatos para validar essa inicialização.

## Como executar

1. Abra a branch `v2.12b-runtime-startup-check`.
2. Vá para **Actions** → **Cloud Xenon toolchain**.
3. Execute **Run workflow** ou acompanhe o workflow disparado pela branch.
4. Aguarde a execução da v2.12b e baixe o artefato `WiiGalaxy360-v2.12b-runtime-startup` após todos os passos passarem.
5. O pacote contém `default.xex`, ELF, objetos e inventário da toolchain.

## O que o workflow valida

- Testes de host.
- Compilação dos arquivos atuais do núcleo com `xenon-g++`.
- Execução, nos testes de host, de uma rotina diagnóstica que verifica `addi`, `stw` e `lwz` no núcleo Broadway.
- Linkagem do smoke test e conversão/validação de ELF32 PowerPC.
- Execução do empacotador XEX externo em modo `--no-sign`.
- Cabeçalho `XEX2` e tamanho mínimo do arquivo resultante.

## Limites importantes

- `default.xex` nesta versão é um **XEX de diagnóstico sem assinatura e sem criptografia**, com uma rotina mínima que exercita parte do núcleo Broadway; não é um executável funcional do emulador Wii.
- A validação automatizada verifica o contêiner e o cabeçalho XEX2, mas não prova que o arquivo inicia em hardware Xbox 360 RGH.
- O modo `--no-sign` não produz a imagem assinada/criptografada exigida por um console retail/devkit padrão. O comportamento em um RGH depende do ambiente de execução e não foi testado aqui.
- O empacotador é obtido de um repositório externo e fixado em commit; consulte a licença e os avisos do projeto RXDK360 antes de reutilizá-lo ou redistribuí-lo.
- O núcleo continua experimental. A rotina de diagnóstico ainda precisa ser confirmada no ambiente Xenon e no console RGH; não há compatibilidade de jogos Wii demonstrada, nem implementação completa de GPU/Flipper, áudio, IOS e inicialização de jogos.

Referências:
- Toolchain libXenon: https://github.com/Free60Project/libxenon
- Empacotador RXDK360: https://github.com/Team-Resurgent/RXDK360
