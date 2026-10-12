# WiiGalaxy360

Emulador experimental de Nintendo Wii para Xbox 360 RGH.

## Estado real — v2.14b: instruções aritméticas e lógicas

A v2.14b adiciona um subconjunto de instruções X-form ao núcleo Broadway: `add`, `subf`, `xor`, `or` e `and`. Os testes cobrem os resultados aritméticos/lógicos e confirmam que variantes com Rc=1, que exigem atualização de CR ainda não implementada, e XO desconhecidos geram falha explícita em vez de serem executados incorretamente. Os diagnósticos de falha da v2.13b foram preservados. A validação da v2.14b passou: os testes de host foram aprovados, o diagnóstico de inicialização executou, e a toolchain Xenon compilou/ligou e empacotou `default.xex`. Execução: https://github.com/MeiKinosaki/WiiGalaxy360/actions/runs/38226290321.

## Como executar

1. Abra a branch `v2.14b-arithmetic-logical-opcodes`.
2. Vá para **Actions** → **Cloud Xenon toolchain**.
3. Execute **Run workflow** ou acompanhe o workflow disparado pela branch.
4. Abra https://github.com/MeiKinosaki/WiiGalaxy360/actions/runs/38226290321 e baixe `WiiGalaxy360-v2.14b-arithmetic-logical-opcodes`.
5. O pacote contém `default.xex`, ELF, objetos e inventário da toolchain.

## O que o workflow valida

- Testes de host.
- Compilação dos arquivos atuais do núcleo com `xenon-g++`.
- Testes de host para `addi`, `stw`, `lwz`, reset, diagnósticos de falha e instruções X-form adicionadas na v2.14b.
- Rotina de startup que verifica `addi`, `stw` e `lwz`; o resultado anterior na v2.12b foi `PASS: Broadway addi/stw/lwz; executed=4 r3=42`.
- Empacotamento Xenon de um XEX de diagnóstico; a execução da v2.14b passou.
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
