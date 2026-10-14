# Estado e roteiro de desenvolvimento

## Estado confirmado

- O repositório possui documentação e um protótipo de biblioteca de interface.
- Existem branches de desenvolvimento com modelos Python de referência para operações PowerPC, memória e classificação de falhas.
- Esses modelos não estão ligados a um núcleo de emulação completo.
- Não há comprovação registrada de execução de jogos Wii no Xbox 360 RGH.

## Marcos seguintes

1. Identificar o conteúdo real de todas as branches e consolidar apenas arquivos coerentes.
2. Criar/identificar o núcleo C/C++ real da CPU Broadway.
3. Adicionar testes executáveis que chamem esse núcleo.
4. Definir e testar memória, exceções e MMIO.
5. Preparar uma compilação Xenon reproduzível.
6. Validar por etapas no hardware e documentar resultados.

## Critério de conclusão

Uma etapa só deve ser marcada como concluída quando houver código real, comandos de teste reproduzíveis e resultados observados. Documentação e modelos de referência, por si sós, não demonstram emulação funcional.
