# WiiGalaxy360

Emulador experimental de Nintendo Wii para Xbox 360 RGH.

## Estado real — v2.8b (toolchain na nuvem)

Esta branch adiciona um workflow do GitHub Actions que executa os testes de host e tenta compilar os arquivos do núcleo para objetos PowerPC/Xenon dentro da imagem Docker `free60/libxenon:latest`. O processo roda nos servidores do GitHub Actions; não exige instalar a toolchain no computador do usuário.

## Como executar

1. Abra a branch `v2.8b-cloud-toolchain` no GitHub.
2. Entre em **Actions** e selecione **Cloud Xenon toolchain**.
3. Use **Run workflow** para iniciar manualmente, ou faça push/abra um pull request para disparar o workflow.
4. Confira os dois jobs. Se a compilação cruzada passar, baixe o artefato `WiiGalaxy360-v2.8b-xenon-objects` no resumo da execução.

## O que esta versão entrega — e o que não entrega

- O job de host executa os testes C++ existentes.
- O job Xenon tenta localizar um compilador cruzado PowerPC/Xenon dentro da imagem libXenon e compila `memory_bus.cpp` e `broadway_cpu.cpp` em arquivos objeto.
- O workflow falha de forma explícita se a imagem não contiver um compilador adequado.
- **Esta versão não gera `default.xex`.** Objetos compilados não são um executável Xbox 360.
- Não há ainda uma etapa de linkagem ELF validada, empacotamento XEX validado ou teste num Xbox 360 RGH.
- O núcleo continua experimental e não é um emulador de Wii funcional; GPU/Flipper, áudio, IOS, inicialização de jogos e compatibilidade real não estão implementados/demonstrados.

A imagem de toolchain usada é publicada pelo projeto Free60/libxenon: https://github.com/Free60Project/libxenon . O workflow deve ser tratado como uma tentativa de build até que a execução do GitHub Actions termine com sucesso.
