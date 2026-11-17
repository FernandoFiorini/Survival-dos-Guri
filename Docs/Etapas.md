# Survival dos Guri — Etapas de desenvolvimento

Baseado no [Roteiro.txt](Roteiro.txt) (v0.1). Cada etapa termina com algo **jogável para testar**.
Tudo é feito já pensando em multiplayer (servidor manda, clientes recebem).

## Etapa 1 — Personagem sobrevivente (roteiro §15, §13)
- [x] Vida e stamina replicadas
- [x] Correr com Shift gasta stamina; sem stamina fica lento ("cansado")
- [x] HUD com barras de vida e stamina
- [x] Pulo gasta stamina (10); sem stamina não pula
- [x] Agachar com C (base para furtividade)
- [x] Morte (corpo cai em ragdoll) e renascer após 5 s
- [ ] Animação de agachar vista pelos outros jogadores (fica para quando trocarmos o modelo/animações)

**Teste:** 2 jogadores correm, cansam, levam dano, morrem e renascem.

## Etapa 2 — Fome, sede e inventário com peso (roteiro §15)
- [ ] Fome e sede caindo com o tempo; afetam a stamina máxima
- [ ] Inventário em grid com stack + peso total
- [ ] Faixas de peso: até 60% normal / 60–90% lento / acima de 90% sem correr
- [ ] Itens no chão para pegar (comida, água, bandagem)
- [ ] Regen de vida só por efeitos de cura (bandagem, remédio...), nunca por padrão. Hoje o `SurvivalStatsComponent` regenera sozinho após `HealthRegenDelay` (foi só um teste de aprendizado): trocar por um efeito de cura temporário

**Teste:** pegar itens, comer, beber, ficar pesado demais e não conseguir correr.

## Etapa 3 — Combate (roteiro §3.1, §11)
- [ ] Melee silencioso que gasta stamina (faca, machado)
- [x] Arma de fogo com dano no sistema de vida novo (personagem do Shooter usa o SurvivalStatsComponent, HUD e morte/renascer da Etapa 1; Lvl_Shooter é o mapa padrão)
- [ ] Armas funcionando em rede (hoje o template do Shooter só funciona para o jogador que hospeda)
- [ ] Barulho: tiros geram "ruído" que atrai inimigos
- [ ] Dano localizado: cabeça / tronco / pernas

**Teste:** matar um boneco de treino com faca e com arma; o tiro faz barulho.

## Etapa 4 — Primeiro inimigo: zumbi comum (roteiro §11)
- [ ] IA do zumbi: vagar, ouvir barulho, perseguir, atacar
- [ ] Só morre com dano na cabeça; tiro nas pernas faz rastejar
- [ ] Spawn de zumbis pelo mapa

**Teste:** sobreviver a um grupo de zumbis usando melee e tiro.

## Etapa 5 — Ciclo dia/noite e loot (roteiro §3.2, §16)
- [ ] Dia/noite com duração configurável
- [ ] Zumbis mais lentos de dia e mais perigosos à noite
- [ ] Baús/contêineres com loot por zona (residencial primeiro)

**Teste:** sair de dia, saquear casas, voltar antes de escurecer.

## Etapa 6 — Crafting básico (roteiro §18)
- [ ] Craft com as mãos: bandagem improvisada, tocha, lança
- [ ] Bancada de madeira: barricadas, ferramentas
- [ ] Coleta: cortar árvore, pegar pedras

**Teste:** coletar recursos, craftar lança e tocha.

## Etapa 7 — Tomar casa e base (roteiro §9)
- [ ] Item "Marco de Abrigo" para reivindicar uma casa
- [ ] Tábuas nas janelas, portas reforçadas
- [ ] Hordas atraídas por barulho, luz e cheiro

**Teste:** limpar uma casa, reivindicar, fortificar e defender uma horda.

## Etapa 8 — Habilidades por atividade (roteiro §14)
- [ ] XP por atividade (Atletismo, Corpo a corpo, Armas de fogo, Coleta...)
- [ ] Retornos decrescentes após ~70%

## Etapa 9 — Servidor e modos (roteiro §6, §10)
- [ ] Criar/entrar em servidor (público/privado)
- [ ] Opções: PvP / PvE / PvP+PvE, Hardcore
- [ ] Salvar o mundo (persistência)

## Etapa 10 em diante — Conteúdo
- Fases da Praga (§4), animais e infecção (§12), monstros do folclore (§11.1),
  Leviatãs (§11.2), mercador e Cuca (§17), airdrops (§19), temporadas e Noite Final (§5),
  corrupção regional (§7), balanceamento (§8), veículos (§13).
