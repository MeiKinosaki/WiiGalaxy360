# Memória e barramento

Objetivo: implementar o mapa de memória do Wii, acessos big-endian e interfaces entre CPU e dispositivos.

## Estado

Este diretório documenta o componente planejado; ainda não há confirmação de um barramento completo integrado ao emulador.

## Próximos passos

- Definir regiões e permissões com base em documentação técnica confiável.
- Implementar leituras/escritas com larguras explícitas e limites.
- Integrar MMIO e tratamento de falhas por tipo de acesso.
- Criar testes que chamem diretamente o código C/C++ real.
