# WiiGalaxy360

Emulador experimental de Nintendo Wii para Xbox 360 RGH.

## Estado real do projeto — v2.3b

Esta versão amplia o núcleo Broadway experimental com o registrador de link (LR), chamadas \`bl\` e retorno \`blr\` (codificado como \`bclr\`, XO 16). O subconjunto de instruções suportado agora inclui \`addi\`, \`addis\`, \`ori\`, \`lwz\`, \`stw\`, \`b\`, \`bl\` e \`blr\`. Há testes de host para uma chamada e retorno simples.

**Limitações:** este não é um emulador de Wii funcional. Não há ainda MMU, exceções PowerPC completas, CR/XER/CTR, decodificação completa, GPU/Flipper, áudio, IOS, drive, inicialização real de jogos nem compatibilidade demonstrada.

## Testes de host

Requisitos: C++17 e GNU Make.

\`\`\`sh
make clean
make host-test
\`\`\`

O workflow GitHub Actions compila e executa os testes de host em pushes e pull requests.

## Xbox 360 / Xenon

Ainda não existe \`default.xex\` verificado nesta versão. A compilação para console exige uma toolchain Xenon/libXenon real e validação do artefato gerado. Testes de host não comprovam compilação cruzada nem execução no hardware.
