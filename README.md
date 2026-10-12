# WiiGalaxy360

Emulador experimental de Nintendo Wii para Xbox 360 RGH.

## Estado real — v2.17b: subfc e carry do XER

A v2.17b implementa `subfc` (XO=8), calculando RB − RA e atualizando CA do XER segundo a semântica carry-not-borrow: CA é definido quando RB ≥ RA e limpo quando ocorre empréstimo. Mantém `addc`, CR0 e as instruções anteriores. Testes adicionados cobrem subtração com e sem empréstimo, incluindo o resultado de 32 bits com wraparound. O diagnóstico inicial também executa `subfc`. A aprovação da versão depende da execução dos workflows desta branch.

## Como executar

1. Abra a branch `v2.17b-xer-subfc`.
2. Vá para **Actions** → **Cloud Xenon toolchain**.
3. Execute **Run workflow** ou acompanhe o workflow disparado pela branch.
4. Acompanhe os workflows da branch e baixe `WiiGalaxy360-v2.17b-xer-subfc` quando concluídos.
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
