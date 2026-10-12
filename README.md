# WiiGalaxy360

Emulador experimental de Nintendo Wii para Xbox 360 RGH.

## Estado real — v2.13b: diagnóstico de falhas do núcleo

A v2.13b amplia o núcleo Broadway com diagnósticos estruturados de falhas. O processador agora registra o motivo, o endereço da instrução e a instrução que falhou, distinguindo busca fora da memória, leitura fora da memória, escrita fora da memória e instrução não suportada. O reset limpa o estado de falha. O workflow de host passou nos testes de falhas e inicialização. O workflow Xenon também passou na compilação, ligação, empacotamento e validação estrutural do XEX; execução registrada em https://github.com/MeiKinosaki/WiiGalaxy360/actions/runs/38224414314.

## Como executar

1. Abra a branch `v2.13b-cpu-fault-diagnostics`.
2. Vá para **Actions** → **Cloud Xenon toolchain**.
3. Execute **Run workflow** ou acompanhe o workflow disparado pela branch.
4. A execução validada está em https://github.com/MeiKinosaki/WiiGalaxy360/actions/runs/38224414314 . Baixe o artefato `WiiGalaxy360-v2.13b-cpu-fault-diagnostics`.
5. O pacote contém `default.xex`, ELF, objetos e inventário da toolchain.

## O que o workflow valida

- Testes de host.
- Compilação dos arquivos atuais do núcleo com `xenon-g++`.
- Testes de host para `addi`, `stw`, `lwz`, reset e diagnóstico de falhas do processador.
- Rotina de startup que verifica `addi`, `stw` e `lwz`; o resultado anterior na v2.12b foi `PASS: Broadway addi/stw/lwz; executed=4 r3=42`.
- Empacotamento Xenon de um XEX de diagnóstico; a execução da v2.13b passou.
- Linkagem do smoke test e conversão/validação de ELF32 PowerPC.
- Execução do empacotador XEX externo em modo `--no-sign`.
- Cabeçalho `XEX2` e tamanho mínimo do arquivo resultante.

## Limites importantes

- `default.xex` nesta versão é um **XEX de diagnóstico sem assinatura e sem criptografia**, com uma rotina mínima que exercita parte do núcleo Broadway e imprime informações de falha; não é um executável funcional do emulador Wii.
- A validação automatizada verifica os testes de host, a compilação e o contêiner/cabeçalho XEX2, mas não prova que o arquivo inicia em hardware Xbox 360 RGH.
- O modo `--no-sign` não produz a imagem assinada/criptografada exigida por um console retail/devkit padrão. O comportamento em um RGH depende do ambiente de execução e não foi testado aqui.
- O empacotador é obtido de um repositório externo e fixado em commit; consulte a licença e os avisos do projeto RXDK360 antes de reutilizá-lo ou redistribuí-lo.
- O núcleo continua experimental. A rotina de diagnóstico ainda precisa ser confirmada no ambiente Xenon e no console RGH; não há compatibilidade de jogos Wii demonstrada, nem implementação completa de GPU/Flipper, áudio, IOS e inicialização de jogos.

Referências:
- Toolchain libXenon: https://github.com/Free60Project/libxenon
- Empacotador RXDK360: https://github.com/Team-Resurgent/RXDK360
