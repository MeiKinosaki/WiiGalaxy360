# WiiGalaxy360

Emulador experimental de Nintendo Wii para Xbox 360 RGH.

## Estado real — v2.9b: linkagem ELF32 na nuvem

A branch v2.8b demonstrou compilação cruzada do núcleo com `xenon-g++`, mas os objetos gerados foram identificados como ELF64. Esta versão corrige isso ao usar as opções de ABI Xenon do projeto libXenon (`-m32 -mpowerpc64 -maltivec -mhard-float -fno-pic -DXENON`) e adiciona uma etapa de linkagem de smoke test.

## Executar

1. Abra a branch `v2.9b-xenon-elf32-link`.
2. Acesse **Actions** → **Cloud Xenon toolchain**.
3. Execute **Run workflow** ou acompanhe a execução do pull request.
4. Se o job passar, baixe `WiiGalaxy360-v2.9b-xenon-elf32`, que contém os objetos, o ELF ligado e a variante ELF32 convertida.

## Limites

- O smoke test comprova apenas que a toolchain compila e liga um programa mínimo junto com os arquivos atuais do núcleo, e que a saída ELF32 reporta a arquitetura PowerPC.
- O programa de smoke test não é o runtime do emulador e não testa hardware.
- **Ainda não há `default.xex`.** O formato ELF32 não é automaticamente um XEX e esta versão não implementa nem valida um empacotador XEX.
- A imagem Docker é referenciada como `free60/libxenon:latest`; portanto, sua composição pode mudar. A execução do workflow é a verificação real para cada commit.
- WiiGalaxy360 continua experimental: não há compatibilidade de jogos demonstrada nem GPU/Flipper, áudio, IOS e inicialização completa de jogos.

Referência da toolchain: https://github.com/Free60Project/libxenon
