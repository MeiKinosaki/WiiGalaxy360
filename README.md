# WiiGalaxy360

Emulador experimental de Nintendo Wii para Xbox 360 RGH.

## Estado real — v2.15b: registrador de condição CR0

A v2.15b implementa atualização do campo CR0 quando as instruções suportadas `add`, `subf`, `xor`, `or` e `and` usam Rc=1. CR0 registra LT, GT ou EQ conforme o resultado; o bit SO permanece zero porque o XER/SO ainda não é modelado. O reset limpa o registrador CR. Os testes agora verificam os três resultados, e o programa de diagnóstico executado na toolchain testa `add.` seguido de `stw` e `lwz`. A execução da v2.15b passou: testes de host e diagnóstico inicial aprovados, compilação/ligação Xenon concluídas e `default.xex` empacotado e validado estruturalmente. Logs: https://github.com/MeiKinosaki/WiiGalaxy360/actions/runs/38293723470.

## Como executar

1. Abra a branch `v2.15b-condition-register-cr0`.
2. Vá para **Actions** → **Cloud Xenon toolchain**.
3. Execute **Run workflow** ou acompanhe o workflow disparado pela branch.
4. Abra https://github.com/MeiKinosaki/WiiGalaxy360/actions/runs/38293723470 e baixe o artefato `WiiGalaxy360-v2.15b-condition-register-cr0`.
5. O pacote contém `default.xex`, ELF, objetos e inventário da toolchain.

## O que o workflow valida

- Testes de host.
- Compilação dos arquivos atuais do núcleo com `xenon-g++`.
- Testes de host para `addi`, `stw`, `lwz`, reset, diagnósticos de falha, instruções X-form e CR0 (LT/GT/EQ).
- Rotina de startup que verifica `addi`, `add.` (Rc=1), `stw`, `lwz` e CR0.
- Empacotamento Xenon de um XEX de diagnóstico; a execução da v2.15b passou.
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
