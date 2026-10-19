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
