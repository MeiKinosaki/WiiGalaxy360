# WiiGalaxy360 — snapshot v2.31b

Este branch preserva o histórico existente e registra uma publicação da fotografia atual do projeto, baseada na branch `v2.31b-gamepad-navigation`.

## Conteúdo incluído

- Protótipo de biblioteca em `ui/library.html`.
- Catálogo com validação e importação/exportação seguras, tratamento de erros de armazenamento e diagnósticos locais.
- Navegação experimental com teclado, mouse e Gamepad API do navegador.
- README com instruções, controles do protótipo, limitações e plano de validação.

## Limitações importantes

- A interface de biblioteca é um protótipo de navegador; não é a interface integrada ao `default.xex`.
- A Gamepad API é experimental e não foi validada em hardware Xbox 360.
- O `default.xex` é diagnóstico e não há execução funcional de jogos Wii demonstrada.
- CPU Broadway, GPU/Flipper, áudio, IOS e inicialização de jogos continuam incompletos.
- Não foram executados testes automatizados ou testes em hardware nesta publicação.

Este branch não reescreve nem apaga o histórico original do repositório. O commit com a mensagem “Initial commit: publish current v2.31b snapshot” registra esta publicação sobre o histórico existente; não é um novo histórico órfão do Git.