# Guia do Projeto — Survival dos Guri

Mapa de "onde fica cada coisa". Atualizar conforme o projeto cresce.

Um projeto Unreal tem **dois mundos**:
- **Código (C++)** em `Source/` → arquivos de texto, editados no Visual Studio. É a **lógica**.
- **Assets** em `Content/` → modelos, animações, sons, mapas, Blueprints. Binários, editados **só pela Unreal**.

---

## 1. Pastas na raiz

| Pasta / arquivo | O que é | Mexer? |
|---|---|---|
| `SurvivalDosGuri.uproject` | Abre o projeto na Unreal | Não |
| `Source/` | Código C++ | ✅ Visual Studio |
| `Content/` | Todos os assets | ✅ Só pela Unreal |
| `Config/` | Configurações `.ini` (mapa inicial, teclas, gráficos) | Raramente |
| `Docs/` | Roteiro, etapas e este guia | ✅ |
| `Binaries/` | Código compilado (`.dll`) | ❌ Gerado |
| `Intermediate/` | Temporários da compilação | ❌ Gerado |
| `Saved/` | Logs, saves, crashes | ❌ Gerado (logs ajudam a achar erros) |
| `DerivedDataCache/` | Cache de shaders/texturas | ❌ Gerado |
| `.vs/`, `.sln`, `.slnx` | Arquivos do Visual Studio | ❌ Gerados |

Pastas "Gerado" podem ser apagadas se algo quebrar feio — a Unreal recria. Não vão para o Git.

---

## 2. Extensões

| Extensão | O que é | Abre com |
|---|---|---|
| `.h` | C++ "índice": o que existe (variáveis, funções, valores padrão) | Visual Studio |
| `.cpp` | C++ "como funciona": o código que roda | Visual Studio |
| `.Build.cs` | Módulos da Unreal que o código usa | Visual Studio |
| `.Target.cs` | Como compilar (jogo / editor) | Quase nunca |
| `.uasset` | Qualquer asset (o tipo vem do prefixo do nome) | Unreal |
| `.umap` | Mapa / fase | Unreal |
| `.ini` | Configuração em texto | Qualquer editor |

---

## 3. Prefixos dos assets

| Prefixo | Tipo | Exemplo |
|---|---|---|
| `BP_` | Blueprint (objeto do jogo) | `BP_FirstPersonCharacter` |
| `ABP_` | Animation Blueprint (qual animação tocar) | `ABP_FP_Copy` |
| `Lvl_` | Mapa | `Lvl_FirstPerson` |
| `IA_` | Input Action (pular, mover, correr) | `IA_Jump`, `IA_Sprint` |
| `IMC_` | Input Mapping Context (tecla → ação) | `IMC_Default` |
| `SK_` / `SKM_` | Skeletal Mesh (modelo com esqueleto) | manequim |
| `SM_` | Static Mesh (modelo parado) | `LevelPrototyping/Meshes` |
| `PA_` | Physics Asset (física do corpo / ragdoll) | `PA_Mannequin` |
| `CR_` / `Ctrl` | Control Rig (ajuste de pose) | `CR_Mannequin_FootIK` |
| `M_` | Material | `M_FillBackground` |
| `MI_` | Material Instance (variação de material) | `MI_FirstPersonColorway` |
| `T_` | Textura | pastas `Textures` |
| `UI_` / `WBP_` | Widget (tela / HUD feito no editor) | `UI_Shooter` |
| `DT_` | Data Table | `DT_WeaponData` |
| `ST_` | State Tree (cérebro de IA) | `ST_Shooter` |
| `EQS_` | Consulta de ambiente da IA | `EQS_FindRoamLocation` |
| `..._Montage` | Animação tocada sob comando | `FP_Rifle_Shoot_Montage` |

---

## 4. Quem controla o quê

```
Mapa (Lvl_FirstPerson)
 └─ GameMode ............ regras: qual personagem usar, onde renascer
     └─ PlayerController ... o "jogador": teclado/mouse, cria o HUD
         └─ Character ...... o corpo: anda, pula, agacha
             └─ Componentes . câmera, malha 3D, vida/stamina
```

Quase tudo tem **classe C++** (comportamento) + **Blueprint filho** (valores e assets escolhidos).

| Assunto | Código (`Source/SurvivalDosGuri/`) | Unreal (`Content/`) |
|---|---|---|
| Vida, stamina, corrida, morte | `Survival/SurvivalStatsComponent` | componente **Survival Stats** no `BP_FirstPersonCharacter` |
| HUD (barras) | `Survival/SurvivalHUDWidget` | — (montado por código) |
| Personagem: teclas, pulo, agachar, renascer | `SurvivalDosGuriCharacter` | `FirstPerson/Blueprints/BP_FirstPersonCharacter` |
| Jogador / input / cria HUD | `SurvivalDosGuriPlayerController` | `FirstPerson/Blueprints/BP_FirstPersonPlayerController` |
| Regras da partida | `SurvivalDosGuriGameMode` | `FirstPerson/Blueprints/BP_FirstPersonGameMode` |
| Câmera | `SurvivalDosGuriCameraManager` | — |
| Teclas | Shift e C ligados no Character | `Input/IMC_Default` + `IA_*` |
| Modelo 3D do corpo | — | `Characters/Mannequins/Meshes` |
| Animações do corpo | — | `Characters/Mannequins/Anims` (Unarmed, Pistol, Rifle, Death) |
| Animação dos braços (1ª pessoa) | — | `FirstPerson/Anims/ABP_FP_Copy` |
| Ragdoll ao morrer | `HandleDeath` liga a física | `Characters/Mannequins/Rigs/PA_Mannequin` |
| Mapa | — | `FirstPerson/Lvl_FirstPerson` |

`Variant_Shooter` e `Variant_Horror` são **exemplos** do template. Vamos reaproveitar peças (armas, IA) e depois apagar o resto.

---

## 5. Como mudar coisas

**Números (sem compilar):** tudo que no `.h` tem `UPROPERTY(EditAnywhere)` ou `UPROPERTY(EditDefaultsOnly)` aparece no painel **Details** do Blueprint.
Ex.: `BP_FirstPersonCharacter` → painel Components → **Survival Stats** → Details → seção **Survival**.
Setinha amarela ↺ = volta ao valor do código.

**Código:**
- Mudou só `.cpp` → salve (Ctrl+S) e na Unreal aperte **Ctrl+Alt+F11** (Live Coding).
- Mudou `.h` → feche a Unreal e compile (Visual Studio: **Ctrl+Shift+B**), depois abra de novo.

**Cores** (`FLinearColor(R, G, B)`) vão de 0 a 1. De 0–255 para 0–1: divida por 255.

---

## 6. Regras de ouro

1. **Nunca mova/renomeie/apague `.uasset` pelo Windows Explorer** — só pelo Content Browser da Unreal.
2. **Onde algo é usado?** Botão direito no asset → **Reference Viewer**.
3. **Qual código controla um Blueprint?** Abra o Blueprint → canto superior direito, **Parent class**.
