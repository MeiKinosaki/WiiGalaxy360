# WiiGalaxy360 — registro de continuidade do desenvolvimento

## Versões e histórico
- Resumo trazido para esta conversa: o usuário indicou v0.5b como versão de trabalho. Esse número não coincide com o histórico remoto do README.
- O histórico remoto documenta versões até v2.37b; o novo marco de código foi registrado como **v2.38b**. Não apagar nem renumerar silenciosamente a referência v0.5b: ela pode ser um contexto de trabalho diferente do histórico publicado.
- Projeto: emulador experimental de Nintendo Wii para Xbox 360 RGH.

## Estado técnico confirmado
- `src/cpu/ppc_cpu.c`: reset, instrução D-form `addi` (opcode 14) e X-form `subfc` (opcode 31 / XO 8).
- `addi` implementa extensão de sinal de SI, RA=0 como literal zero, incremento de PC em 4 e preservação de CR/XER[CA].
- `subfc` grava RT e atualiza XER[CA] conforme carry-not-borrow.
- `tests/test_ppc_cpu.c` adiciona casos para `addi` positivo, negativo, RA=0 e preservação de estado, além de regressões de `subfc`.
- O workflow Host tests foi acionado pelos commits `e00d6f1`, `76ca913` e `73523b5`; no momento deste registro, as execuções novas ainda não tinham resultado final confirmado. Não afirmar que passaram até consultar os logs.
- Workflow anterior `38692026246` concluiu com sucesso, mas testava o código anterior, sem `addi`.

## Toolchain e executável
- A toolchain Xenon precisa de validação reproduzível no estado atual.
- O `default.xex` descrito no README é um XEX de diagnóstico, não o emulador Wii completo.
- Empacotamento/cabeçalho XEX não prova boot no Xbox 360 RGH.
- Não há compatibilidade de jogos Wii comprovada.

## Escopo preservado
- CPU Broadway/PowerPC e ALU.
- Memória/barramento, ciclo de execução, exceções e MMIO.
- GPU/Flipper, áudio, IOS e inicialização de jogos.
- Toolchain na nuvem e artefatos ELF32 PowerPC / `default.xex`.
- UI azul/branca inspirada no Aurora, biblioteca, capas, favoritos, configurações, saves/save states e tela/música de abertura. Distinguir protótipo visual de integração real.

## Regras de desenvolvimento
- Preservar histórico e funcionalidades existentes.
- Avançar versões sequencialmente; v2.38b é o próximo marco do histórico publicado após v2.37b.
- Não declarar testes aprovados sem resultado/logs.
- Não contar planos ou protótipos como integração concluída.
- Próxima prioridade: confirmar CI; então expandir instruções CPU, integrar memória e avançar na toolchain cloud.
