# WiiGalaxy360 — registro de continuidade do desenvolvimento

## Versões e histórico
- O resumo de trabalho trazido à conversa mencionava v0.5b; o README remoto já documentava versões posteriores. Não apagar nem renumerar silenciosamente essa referência de contexto.
- Histórico publicado avançado sequencialmente: v2.38b (`addi`) e v2.39b (`ori`), após v2.37b. `addic` está sendo desenvolvido dentro do marco de manutenção v2.39b; ainda não cria nova versão.
- Projeto: emulador experimental de Nintendo Wii para Xbox 360 RGH.

## Estado técnico
- `src/cpu/ppc_cpu.c`: reset e execução real das instruções `addi`, `addic`, `ori` e `subfc`.
- `addi`: imediato com sinal, RA=0 literal zero, PC +4.
- `ori`: imediato sem sinal, RS como fonte e RA como destino, GPR0 funciona como registrador de origem, PC +4.
- `subfc`: RB − RA com atualização carry-not-borrow em XER[CA].
- `addic`: soma com imediato sinalizado e atualização de XER[CA]; testes acrescentados para wraparound e ausência de carry. CI para o commit dos testes ainda está em andamento.
- Testes C no dispatcher foram atualizados para esses caminhos. CI confirmou os commits anteriores de `addi` e `ori`. Os workflows de `addic` ainda precisam terminar antes de declarar a nova alteração validada.
- O núcleo não é um Broadway completo. Memória/barramento, exceções, MMIO, GPU/Flipper, áudio, IOS e startup não estão completos ou não têm validação confirmada.

## Toolchain e executável
- A toolchain Xenon ainda exige validação reproduzível no estado atual.
- O `default.xex` histórico é de diagnóstico, não o emulador Wii completo.
- Um cabeçalho XEX válido não comprova boot no Xbox 360 RGH; jogos Wii funcionando não foram demonstrados.

## Escopo preservado
CPU/ALU, memória, exceções, GPU/Flipper, áudio, IOS, cloud toolchain, ELF32/`default.xex`, UI azul/branca estilo Aurora, biblioteca/capas/favoritos/configurações/saves e abertura/música. Distinguir estrutura/protótipo de integração funcional.

## Regras
- Preservar histórico e funcionalidades.
- Versões sequenciais sem pular números: v2.39b é o marco publicado atual.
- Não afirmar teste aprovado sem logs reais.
- Não contar planos ou protótipos como implementação concluída.
- Próxima prioridade: confirmar CI de `addic`, continuar o conjunto de instruções com testes, integrar memória/barramento e validar a toolchain na nuvem.
