# Testes

Os testes devem distinguir claramente entre:

- **Testes de referência:** modelos independentes que ilustram uma regra.
- **Testes unitários integrados:** executam funções reais do código-fonte do emulador.
- **Testes de integração:** verificam a interação entre CPU, memória e dispositivos.
- **Testes no console:** compilação e execução no Xbox 360 RGH.

Os scripts `*_reference.py` em branches de desenvolvimento são modelos auxiliares e não comprovam o funcionamento do núcleo real. Não declare compatibilidade de jogos sem resultados reproduzíveis.
