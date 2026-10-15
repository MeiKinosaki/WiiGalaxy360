# Estado e roteiro de desenvolvimento

## Estado confirmado — v2.39b

- Núcleo C da CPU em `src/cpu/ppc_cpu.c`: reset e dispatcher para `b`, `addi`, `addic`, `ori`, `subfc`, `add` e operações lógicas X-form `and`, `or`, `xor`.
- `b`: salto incondicional I-form com deslocamento com sinal, seleção AA absoluto/relativo e atualização de LR quando LK=1; testes cobrem salto relativo com link e salto absoluto.
- `addi`: extensão de sinal do imediato, RA=0 como literal zero, PC +4 e preservação de CR/XER[CA].
- `ori`: OR com imediato zero-extendido, RS como fonte, RA como destino, PC +4 e preservação de CR/XER[CA]. O registrador GPR0 continua sendo fonte normal para `ori`.
- `addic`: soma com imediato com sinal e atualiza XER[CA] conforme o carry-out de 32 bits; testes cobrem wraparound e ausência de carry.
- `subfc`: cálculo RB − RA com atualização XER[CA] carry-not-borrow.
- X-form `add` não altera XER[CA]; `and`, `or` e `xor` escrevem no campo RA conforme o formato PowerPC e rejeitam variantes Rc=1 ainda não suportadas.
- Novo barramento-base `wii_memory_t` em `include/wii/memory.h` e `src/memory/memory.c`: leitura/escrita de 8/16/32 bits em ordem big-endian, verificação de limites e tratamento de ponteiros nulos.
- A CPU ganhou `ppc_cpu_step_with_memory`, integrado a `lwz`, `lbz`, `lhz`, `stw`, `stb` e `sth`; endereços desalinhados em meia-palavra/palavra retornam `PPC_STEP_ALIGNMENT`, e acessos fora dos limites retornam `PPC_STEP_UNSUPPORTED`, sem avançar PC. Ainda não há mapa de memória Wii real nem MMIO. O alinhamento é detectado, mas o tratamento ainda é um resultado de erro do núcleo, não um vetor de exceção PowerPC completo.
- `tests/test_ppc_cpu.c` cobre instruções aritméticas/lógicas, imediatos, carry, PC e integração load/store de byte, meia-palavra e palavra. `tests/test_memory.c` cobre endianness e acessos fora dos limites. CI dos commits mais recentes de integração e meia-palavra precisa ser conferida.
- O workflow Host tests confirmou sucesso para os commits de documentação de v2.39b; o resultado final após a inclusão de `addic` ainda precisa ser verificado.
- O núcleo continua sendo um subconjunto pequeno, não um processador Broadway completo.
- Não há comprovação registrada de jogos Wii funcionando no Xbox 360 RGH.

## Próximos marcos

1. Confirmar CI para v2.39b e corrigir eventuais regressões.
2. Confirmar CI da integração load/store e da API de memória.
3. Adicionar mapeamento de RAM/ROM e MMIO; depois implementar alinhamento e exceções reais.
4. Implementar exceções e fluxo de controle e expandir a cobertura PowerPC.
5. Validar a toolchain Xenon reproduzivelmente e guardar logs/artefatos ELF32.
6. Separar claramente o XEX de diagnóstico do executável funcional; testar em RGH apenas com evidência concreta.
7. Desenvolver GPU/Flipper, áudio, IOS e startup por marcos verificáveis.
8. Integrar a biblioteca azul/branca, capas, favoritos, configurações e saves ao app nativo; protótipo HTML não conta como integração.

## Critério de conclusão

Só marcar uma etapa como concluída quando existir código real, testes reproduzíveis e resultado observado. Documentação e modelos de referência não provam emulação funcional.
