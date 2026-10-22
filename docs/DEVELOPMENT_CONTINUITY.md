# WiiGalaxy360 — registro de continuidade do desenvolvimento

## Versões e histórico
- O resumo de trabalho trazido à conversa mencionava v0.5b; o README remoto já documentava versões posteriores. Não apagar nem renumerar silenciosamente essa referência de contexto.
- Histórico publicado avançado sequencialmente: v2.38b (`addi`) e v2.39b (`ori`), após v2.37b. `addic` está sendo desenvolvido dentro do marco de manutenção v2.39b; ainda não cria nova versão.
- Projeto: emulador experimental de Nintendo Wii para Xbox 360 RGH.

## Estado técnico
- `src/cpu/ppc_cpu.c`: reset e execução real das instruções `b`, `addi`, `addis`, `addic`, `ori`, `xori`, `subfc`, `add`, `and`, `or` e `xor`.
- `mfspr`/`mtspr`: SPR LR (8) e CTR (9) suportados; outros SPR rejeitados.
- `cmpwi`/`cmplwi`: comparação imediata assinada/não assinada com atualização do CR; L=1 de 64 bits não suportado. `addis`: imediato com sinal deslocado 16 bits; `xori`: XOR com imediato sem sinal, permitindo GPR0 como origem/destino.
- `addi`: imediato com sinal, RA=0 literal zero, PC +4.
- `ori`: imediato sem sinal, RS como fonte e RA como destino, GPR0 funciona como registrador de origem, PC +4.
- `bc`: salto condicional B-form com testes CR/CTR, deslocamento assinado e opções AA/LK.
- `b`: salto incondicional com deslocamento assinado, AA absoluto/relativo e LR quando LK está ativo.
- `subfc`: RB − RA com atualização carry-not-borrow em XER[CA].
- `addc` (X-form, XO 10) foi integrado ao dispatcher e aos testes: soma RA + RB, escreve RT e atualiza XER[CA] pelo carry-out. A suíte CI após a alteração ainda deve ser conferida.
- `add`: soma X-form sem alteração de XER[CA]. `and`, `or` e `xor` seguem o formato lógico PowerPC; variantes Rc=1 seguem não suportadas.
- Barramento-base adicionado em `include/wii/memory.h` e `src/memory/memory.c`: acessos de 8/16/32 bits big-endian, checagem de limites e falha segura para ponteiros nulos.
- `ppc_cpu_step_with_memory` conecta `lwz`, `lbz`, `lhz`, `stw`, `stb` e `sth` ao barramento. Falhas de endereço não avançam PC; acessos desalinhados de palavra/meia-palavra retornam `PPC_STEP_ALIGNMENT`. O mapa Wii, MMIO e vetores de exceção reais continuam pendentes. `lha` estende o sinal do halfword carregado.
- `addic`: soma com imediato sinalizado e atualização de XER[CA]; testes acrescentados para wraparound e ausência de carry. CI para os testes da CPU após expansão ainda precisa de resultado final.
- Testes C no dispatcher foram atualizados para esses caminhos. CI confirmou os commits anteriores de `addi` e `ori`. Os workflows mais recentes de integração CPU/memória precisam terminar antes de declarar a nova etapa validada.
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
- Próxima prioridade: validar CI de load/store de byte/meia-palavra/palavra; desenvolver mapa de memória, MMIO, alinhamento/exceções; continuar GPU/Flipper, áudio, IOS, UI e toolchain em marcos verificáveis.


## Continuação mais recente — addze
- Implementado `addze` (X-form XO 202) em `src/cpu/ppc_cpu.c`: soma o registrador RA com o carry de entrada XER[CA], grava RT e atualiza XER[CA] com o carry-out.
- Adicionados testes em `tests/test_ppc_cpu.c` para wraparound `0xffffffff + 1` e caso sem carry-in.
- Commits: implementação `7e3c1276a3b051cdded31cf3b53331552dc57009`; testes `2d11cb1e1731278ca9e09a03d58f2fe6f54d3b07`.
- Estado: implementação e testes enviados; não considerar validado até o workflow Host tests terminar com sucesso.


## Continuação — X-form subf
- Implementação adicionada em `src/cpu/ppc_cpu.c` para XO 40: `RT = RB - RA`, sem atualizar XER[CA].
- Testes adicionados em `tests/test_ppc_cpu.c` para subtração positiva, resultado negativo em 32 bits e preservação de XER[CA].
- Commits: código `970161392eabf9b801abe5ccba3399fcded81e6d`; testes `889622912bbfb53fb041e840c2df2f118be92a98`.
- CI ainda precisa confirmar os testes desta instrução. `addze` foi validado no workflow Host tests run 39302452017.


## Continuação — neg e mullw
- Implementadas instruções PowerPC X-form `neg` (XO 104, com RB reservado zero) e `mullw` (XO 235, produto assinado com resultado de 32 bits inferiores).
- Testes adicionados para negação de zero e valor positivo, codificação inválida de `neg`, produto negativo e wraparound de `mullw`.
- Commits: código `9efaf1da09a075cf84b15df8f67ab0b1b18df967`; testes `605e8445b5f88ffe7b5b14d31bcf22f83d540fa1`.
- CI dos novos testes pendente; não considerar concluído até resultado observado.


## Continuação — mulli
- Implementada instrução D-form `mulli` (opcode 7), com imediato de 16 bits com sinal e resultado limitado aos 32 bits inferiores.
- Testes adicionados para imediato negativo e wraparound do resultado.
- Commits: implementação `0d25060aa03a6dd4309fe09e9dc685aed4358785`; testes `09c392d52cafce2707a3e2ee14842932d714ebf7`.
- CI pendente para a nova cobertura.


## Continuação — subfic
- Implementada instrução D-form `subfic` (opcode 8), `RT = sign_extend(SI) - RA`, com `XER[CA]` definido quando não há borrow.
- Testes adicionados para diferença positiva, borrow e caso de wraparound.
- Commits: código `0a51d3d5ebd34662971897b8d1f280023e22d957`; testes `0f8fab78fd603c34775aec0ecce0b39cf23729db`.
- CI dos novos testes pendente.


## Continuação — nor
- Implementada instrução lógica X-form `nor` (XO 124), sem forma Rc=1 nesta cobertura.
- Testes adicionados para padrões mistos e `0 NOR 0 = 0xffffffff`.
- Commits: implementação `4eadd8bc0b996005b16fab6982cb7bbde8176a48`; testes `8b9234dbaecc5efa12c422cd18af9b184aafb05f`.
- CI dos novos testes pendente.


## Correção de teste — nor
- O primeiro teste de `nor` falhou porque verificava `RT` em vez de `RA`, que é o destino arquitetural da instrução.
- Corrigidas as asserções no commit `27c371f7922d2eb94fc5ee357f4c28766291c325`. Aguardar workflow antes de marcar como validado.


## Continuação — integração fetch/decode/execute
- Adicionada API `ppc_cpu_step_from_memory`, que lê a instrução de 32 bits em big-endian no PC, recusa PC desalinhado e delega a execução à CPU com a mesma região de memória para loads/stores.
- Testes cobrem execução sequencial de `addi` e `ori` a partir da memória, atualização de PC, alinhamento e endereço sem instrução executável suportada.
- Atenção: o PC é tratado como offset da região limitada; isso não equivale ao mapa de memória real do Wii.
- CI pendente no workflow https://github.com/MeiKinosaki/WiiGalaxy360/actions/runs/39763454475.
