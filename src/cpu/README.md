# CPU PowerPC / Broadway

Objetivo: implementar o conjunto de instruções PowerPC usado pela CPU Broadway do Wii.

## Estado

**Planejado/em desenvolvimento; implementação integrada ainda não confirmada.** Os modelos Python existentes em branches de desenvolvimento são referências de teste, não o interpretador real.

## Próximos passos

1. Localizar ou criar o dispatcher de instruções em C/C++.
2. Definir estado de registradores GPR, CR, LR, CTR e XER.
3. Implementar instruções em incrementos pequenos e testáveis.
4. Executar testes contra a implementação real e registrar resultados.
