# WiiGalaxy360

Emulador experimental de Nintendo Wii para Xbox 360 RGH.

## Estado real — v2.16b: XER e carry com addc

A v2.16b adiciona estado XER e implementa `addc` (XO=10) no subconjunto X-form. O bit CA (0x20000000) é atualizado com o carry unsigned da soma de 32 bits; `Rc=1` também atualiza CR0 a partir do resultado. O reset limpa XER. Os testes cobrem carry ligado, carry desligado apesar de overflow com sinal, CR0 e reset. O diagnóstico de inicialização agora verifica `addc`, CR0, XER, `stw` e `lwz`. A validação da v2.16b passou: os testes de host, o diagnóstico de inicialização, a compilação/ligação Xenon e o empacotamento/validação estrutural do `default.xex` foram concluídos com sucesso. Logs: https://github.com/MeiKinosaki/WiiGalaxy360/actions/runs/38295685506.

## Como executar

1. Abra a branch `v2.16b-xer-carry-addc`.
2. Vá para **Actions** → **Cloud Xenon toolchain**.
3. Execute **Run workflow** ou acompanhe o workflow disparado pela branch.
4. Abra https://github.com/MeiKinosaki/WiiGalaxy360/actions/runs/38295685506 e baixe o artefato `WiiGalaxy360-v2.16b-xer-carry-addc`.
5. O pacote contém `default.xex`, ELF, objetos e inventário da toolchain.

## O que o workflow valida

- Testes de host.
- Compilação dos arquivos atuais do núcleo com `xenon-g++`.
- Testes de host para `addi`, `stw`, `lwz`, reset, diagnósticos de falha, instruções X-form, CR0 e XER/CA.
- Rotina de startup que verifica `addi`, `add.` (Rc=1), `addc`, `stw`, `lwz`, CR0 e XER/CA.
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
