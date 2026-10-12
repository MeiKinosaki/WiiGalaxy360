# WiiGalaxy360

Emulador experimental de Nintendo Wii para Xbox 360 RGH.

## Estado real do projeto — v2.2b

A base inclui barramento de memória big-endian, núcleo experimental Broadway com um subconjunto de instruções e testes automatizados. Nesta versão foram adicionados `addis` e salto relativo/absoluto `b` sem link (`LK=0), com testes que verificam o desvio de fluxo. O ramo de salto com link (`LK=1`) é deliberadamente interrompido até existir suporte correto ao registrador LR.

**Ainda não é um emulador de Wii funcional.** Faltam MMU e exceções completas, registradores especiais/CR/LR, dispositivos do Wii, GPU/Flipper, áudio, IOS, drive, boot e compatibilidade de jogos.

## Testar em host

Requisitos: C++17 e GNU Make.

```sh
make clean
make host-test
```

O GitHub Actions executa os testes em pushes e pull requests.

## Compilação Xbox 360

Nenhum `default.xex` é fornecido nesta etapa. É necessário configurar e validar uma toolchain Xenon/libXenon real. Testes de host não comprovam compilação PowerPC/Xenon nem execução no console.
