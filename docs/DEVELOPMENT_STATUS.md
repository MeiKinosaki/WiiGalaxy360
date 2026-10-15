# Estado e roteiro de desenvolvimento

## Estado confirmado — v2.39b

- Núcleo C da CPU em `src/cpu/ppc_cpu.c`: reset e dispatcher para `addi` (opcode 14), `ori` (opcode 24) e `subfc` (opcode 31 / XO 8).
- `addi`: extensão de sinal do imediato, RA=0 como literal zero, PC +4 e preservação de CR/XER[CA].
- `ori`: OR com imediato zero-extendido, RS como fonte, RA como destino, PC +4 e preservação de CR/XER[CA]. O registrador GPR0 continua sendo fonte normal para `ori`.
- `subfc`: cálculo RB − RA com atualização XER[CA] carry-not-borrow.
- `tests/test_ppc_cpu.c` cobre os três caminhos de instrução, casos de imediatos e PC/estado.
- O workflow Host tests já confirmou sucesso para o marco anterior `addi` (runs 38692587241, 38692591062 e 38692594563). Os testes específicos de `ori` ainda precisam de resultado final confirmado.
- O núcleo continua sendo um subconjunto pequeno, não um processador Broadway completo.
- Não há comprovação registrada de jogos Wii funcionando no Xbox 360 RGH.

## Próximos marcos

1. Confirmar CI para v2.39b e corrigir eventuais regressões.
2. Acrescentar instruções aritméticas X-form com semântica de flags definida, cada uma com testes no dispatcher.
3. Integrar uma API de memória/barramento big-endian com limites, alinhamento e MMIO.
4. Implementar exceções e fluxo de controle.
5. Validar a toolchain Xenon reproduzivelmente e guardar logs/artefatos ELF32.
6. Separar claramente o XEX de diagnóstico do executável funcional; testar em RGH apenas com evidência concreta.
7. Desenvolver GPU/Flipper, áudio, IOS e startup por marcos verificáveis.
8. Integrar a biblioteca azul/branca, capas, favoritos, configurações e saves ao app nativo; protótipo HTML não conta como integração.

## Critério de conclusão

Só marcar uma etapa como concluída quando existir código real, testes reproduzíveis e resultado observado. Documentação e modelos de referência não provam emulação funcional.
