# WiiGalaxy360 — registro de continuidade do desenvolvimento

## Estado de continuidade
- Resumo trazido para esta conversa: versão de trabalho informada como v0.5b.
- Estado do repositório remoto consultado em 2026-10-15: README contém documentação de versões posteriores, chegando a v2.21b. Não reescrever a história nem renumerar silenciosamente; reconciliar o resumo de trabalho com os registros do repositório antes de criar uma nova versão.
- Projeto: emulador experimental de Nintendo Wii para Xbox 360 RGH.
- Objetivo técnico imediato: expandir a execução de instruções da CPU Broadway com testes unitários integrados, sem afirmar compatibilidade de jogos ou execução em hardware sem evidência.

## Estado técnico verificado
- src/cpu/ppc_cpu.c: reset inicializa 32 GPRs, PC, LR, CTR, CR e XER CA. ppc_cpu_step implementa atualmente opcode 31 / XO 8 (subfc), grava RT, incrementa PC em 4 e retorna PPC_STEP_UNSUPPORTED para outras instruções.
- src/cpu/ppc_alu.c: subfc, addc e helpers de comparação/AND/OR/XOR.
- include/ppc/ppc_cpu.h e include/ppc/ppc_alu.h definem interfaces e estado parcial.
- tests/test_ppc_cpu.c verifica reset, subfc com e sem empréstimo, atualização de CA, PC e instrução não suportada.
- Makefile executa test_ppc_alu e test_ppc_cpu.
- Workflow .github/workflows/host-tests.yml executa make test no Ubuntu.
- Run de Actions verificado: 38692026246; job ppc-alu concluiu com sucesso, incluindo compilação e execução dos testes de host. Isso não valida a toolchain Xenon nem o console.

## Toolchain e executável
- A toolchain de compilação cruzada Xenon precisa de validação reproduzível.
- A documentação do repositório descreve um default.xex de diagnóstico não assinado/criptografado; não é o emulador Wii completo.
- Cabeçalho/empacotamento XEX validado estruturalmente não comprova execução em Xbox 360 RGH.
- Não há compatibilidade de jogos Wii comprovada.

## Escopo preservado
- CPU Broadway e ALU PowerPC.
- Integração com memória e ciclo busca/decodificação/execução.
- GPU/Flipper, áudio, IOS e inicialização de jogos.
- Toolchain na nuvem e geração/validação de ELF32 PowerPC e default.xex.
- Interface azul/branca inspirada no Aurora, biblioteca/capas/favoritos, configurações, saves/save states e tela/música de abertura; recursos visuais devem ser distinguidos de integração funcional.

## Regras de versão e evidência
- Não perder funcionalidades nem histórico.
- Não pular números de versão.
- Não declarar testes aprovados sem logs reais.
- Não contar plano ou protótipo como integração concluída.
- Próximo marco: ampliar ppc_cpu_step e testes unitários; depois executar CI, registrar resultados e reconciliar a numeração da versão antes de anunciar nova versão.
