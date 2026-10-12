# WiiGalaxy360

Emulador experimental de Nintendo Wii para Xbox 360 RGH.

## Estado real — v2.10b: auditoria de empacotamento XEX

A v2.9b foi compilada com sucesso no GitHub Actions: os testes de host passaram, o núcleo foi compilado, um ELF foi ligado e a saída convertida foi verificada como ELF de 32 bits para PowerPC. O artefato foi publicado pelo workflow.

A v2.10b adiciona uma auditoria automatizada do conteúdo da imagem `free60/libxenon:latest` para procurar ferramentas executáveis cujo nome contenha `xex`, além de inventariar ferramentas binutils relevantes. O objetivo é não inventar ou simular um empacotador.

## Como executar

1. Abra a branch `v2.10b-xex-packer-audit`.
2. Acesse **Actions** → **Cloud Xenon toolchain**.
3. Execute o workflow ou acompanhe a execução automática.
4. Quando concluir, baixe o artefato `WiiGalaxy360-v2.10b-cloud-build-audit` e examine `xex-tool-inventory.txt`.

## Limites conhecidos

- A v2.9b gerou um ELF32 PowerPC válido para o smoke test, não um XEX.
- A v2.10b procura ferramentas XEX disponíveis na imagem; o resultado ainda depende da execução do workflow.
- O workflow não declara `default.xex` pronto e não renomeia ELF para XEX.
- Ainda não há execução demonstrada no Xbox 360 RGH, nem compatibilidade de jogos Wii demonstrada.
- O entry point usado é um smoke test mínimo, não o runtime final do emulador.

Referência: https://github.com/Free60Project/libxenon
