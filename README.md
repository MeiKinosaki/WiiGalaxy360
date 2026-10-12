# WiiGalaxy360

Emulador experimental de Nintendo Wii para Xbox 360 RGH.

## Estado real — v2.11b: geração de default.xex na nuvem

A v2.9b comprovou compilação e ligação de um smoke test ELF32 PowerPC no GitHub Actions. A v2.10b confirmou que a imagem base libXenon não fornecia um empacotador XEX óbvio. A v2.11b adiciona um passo de empacotamento com `elf2xex.py` do projeto RXDK360, fixado no commit `ca06a4db2bc91a44cecfa1f2895adb8f23e91ba3`.

## Como executar

1. Abra a branch `v2.11b-cloud-xex-packaging`.
2. Vá para **Actions** → **Cloud Xenon toolchain**.
3. Execute **Run workflow** ou acompanhe o workflow disparado pela branch.
4. Quando todos os passos passarem, baixe o artefato `WiiGalaxy360-v2.11b-cloud-xex`.
5. O pacote contém `default.xex`, ELF, objetos e inventário da toolchain.

## O que o workflow valida

- Testes de host.
- Compilação dos arquivos atuais do núcleo com `xenon-g++`.
- Linkagem do smoke test e conversão/validação de ELF32 PowerPC.
- Execução do empacotador XEX externo em modo `--no-sign`.
- Cabeçalho `XEX2` e tamanho mínimo do arquivo resultante.

## Limites importantes

- `default.xex` nesta versão é um **XEX de smoke test sem assinatura e sem criptografia**, baseado em um entry point mínimo; não é o executável funcional do emulador Wii.
- A validação automatizada verifica o contêiner e o cabeçalho XEX2, mas não prova que o arquivo inicia em hardware Xbox 360 RGH.
- O modo `--no-sign` não produz a imagem assinada/criptografada exigida por um console retail/devkit padrão. O comportamento em um RGH depende do ambiente de execução e não foi testado aqui.
- O empacotador é obtido de um repositório externo e fixado em commit; consulte a licença e os avisos do projeto RXDK360 antes de reutilizá-lo ou redistribuí-lo.
- O núcleo continua experimental. Não há compatibilidade de jogos Wii demonstrada, nem implementação completa de GPU/Flipper, áudio, IOS e inicialização de jogos.

Referências:
- Toolchain libXenon: https://github.com/Free60Project/libxenon
- Empacotador RXDK360: https://github.com/Team-Resurgent/RXDK360
