# Estado e roteiro de desenvolvimento

## Estado confirmado — atualização v2.38b

- Núcleo C da CPU: `src/cpu/ppc_cpu.c` implementa reset, `addi` (opcode 14) e `subfc` (opcode 31 / XO 8).
- `addi`: imediato de 16 bits com extensão de sinal; RA=0 usa literal zero; PC avança 4 após sucesso; CR e XER[CA] são preservados.
- `subfc`: calcula RB − RA e atualiza carry-not-borrow por meio da ALU.
- Testes C: `tests/test_ppc_cpu.c` cobre `addi` positivo/negativo, RA=0, preservação de estado, `subfc`, PC e instrução não suportada.
- O workflow Host tests foi acionado por commits recentes. O resultado dos novos testes ainda deve ser consultado antes de declarar a etapa validada.
- O dispatcher é um subconjunto mínimo, não um processador Broadway completo.
- A biblioteca HTML é um protótipo de interface, não frontend integrado ao XEX.
- Não há comprovação registrada de execução de jogos Wii no Xbox 360 RGH.

## Próximos marcos técnicos

1. Confirmar resultado verde do CI para os commits de `addi`; corrigir regressões se aparecerem.
2. Implementar instruções PowerPC adicionais uma por vez, com testes no dispatcher real (próximo alvo: instruções aritméticas X-form com semântica de flags definida).
3. Projetar uma API de memória/barramento big-endian integrada ao núcleo, com limites, alinhamento e MMIO.
4. Implementar exceções e fluxo de controle antes de tentar executar blocos maiores de código.
5. Validar a toolchain Xenon em workflow reproduzível; registrar compilador, linker, conversão ELF32 e artefatos.
6. Distinguir XEX de diagnóstico de um executável real do emulador; testar no RGH somente quando houver ambiente e evidência.
7. Avançar por etapas em GPU/Flipper, áudio, IOS e inicialização do sistema, sem declarar compatibilidade de jogos antes de testes concretos.
8. Integrar gradualmente biblioteca/capas/favoritos/configurações/saves à aplicação nativa; os diretórios e protótipos atuais não provam essas funções.

## Critério de conclusão

Uma etapa só é concluída quando há código real, testes reproduzíveis e resultados observados. Documentação, planos, modelos Python de referência e protótipos visuais não demonstram emulação funcional.
