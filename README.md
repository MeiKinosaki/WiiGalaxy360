# WiiGalaxy360

Emulador experimental de Nintendo Wii para Xbox 360 RGH.

## Estado real do projeto — v2.1b

Esta versão cria uma base de código verificável: barramento de memória simples, núcleo inicial de execução PowerPC/Broadway com um subconjunto de instruções e testes de host. **Ainda não é um emulador de Wii funcional.** Faltam MMU e exceções completas, dispositivos do Wii, GPU/Flipper, áudio, IOS, drive, boot e compatibilidade de jogos.

## Compilar e testar no computador

Requisitos: C++17 e GNU Make.

```sh
make host-test
make clean
```

O GitHub Actions executa os testes de host em pushes e pull requests.

## Compilação para Xbox 360

Nenhum `default.xex` é fornecido nesta etapa. A compilação Xenon exige uma toolchain libXenon compatível instalada e ainda precisa ser configurada e validada. Passar nos testes de host **não** comprova compilação PowerPC/Xenon nem execução no console.

## Próxima etapa

Expandir os testes e o conjunto de instruções de forma incremental, documentando o que está implementado e o que ainda falta.
