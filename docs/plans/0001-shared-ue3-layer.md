---
id: 0001
title: One shared UE3 layer for every UE3 port, on x86 and Xbox 360
status: future (not started; PSX is the active project)
created: 2026-11-12
---

# One shared UE3 layer for every UE3 port

## Request

The user, 2026-11-12: "can we make something like OpenUE3 or something to run Borderlands 2,
Gears of War 1/2/3 and such?", and then "I wanted to make a plan first, for future".

## Decision

There will be no clean-room engine. UnrealScript is portable bytecode, but each title carries a
large amount of its own native C++, such as Gears' cover system and Borderlands 2's skills and
physics, on engine forks from 2006 to 2013. A replacement engine would still need every title's
natives rewritten, which is a full port per title.

Instead, each title keeps running its own executable as a native/dynarec hybrid (dynarec-port).
All of them share one UE3 layer that knows the engine, sitting above whichever CPU runtime the
title uses:

```text
title adapters (bl2, gears1, gears2, gears3, judgment, ...)
        addresses, schema versions, title natives
                         |
              shared/ue3port (this repo, renamed)
   object system · script events · RHI and native renderer · config, saves, log, malloc
                         |
           guest ABI adapter: x86 | Xbox 360
                         |
          shared/x86port          shared/x360port -> Xenia
```

## What the shared layer owns

Each item is title-neutral. A title supplies its addresses and layout versions as data.

- **Object system:**
  - Reads `GNames`, `GObjects`, `UObject`, `UClass`, `UProperty` and `UFunction` through a
    title-supplied layout schema (S001 already validates schemas).
  - Finds objects by path and walks classes and properties.
  - Byte order and pointer width come from the ABI adapter, since Xbox 360 is big-endian.
- **Script events:** hooks `ProcessEvent` and `CallFunction` with typed parameters, so native
  owners and tools act through the game's own functions.
- **Rendering:** the existing RHI semantic contract (S002, S003), executed by one native Vulkan
  renderer that replaces each title's D3D9 or Xenos path pass by pass (ue3-native-pass).
- **Services:**
  - `GLog` into Lucent logging, `GMalloc` onto the host allocator.
  - Ini and config redirection, and the save location (release).
- **Tools:**
  - A control channel view of the object tree, through `lucent::http::Server`.
  - The dynamic scan, which reports owners that never fire.

What it does not own:
- guest CPU, memory, imports and invalidation (x86port, x360port);
- title addresses and gameplay (title adapters);
- anything from `shared/ue3`, which stays reference only and is never compiled, linked or copied.

## Titles

| Title | Runtime | Repo | Today |
|---|---|---|---|
| Borderlands 2 (PC) | x86port | `pc/bl2` | entry point runs under the JIT; main menu missing (S005) |
| Gears of War (360) | x360port | `x360/gears1` | focus: gameplay past `WarCheckpoint_3` |
| Gears of War 2, 3, Judgment (360 only) | x360port | `x360/gears1` (its G001 already names them) | not started |
| Further UE3 PC titles, for example Bioshock Infinite, Mirror's Edge, Arkham Asylum and City, Borderlands: The Pre-Sequel | x86port | new title repos | candidates; each needs a prior-art check first |

A title that already has a good port is modded instead of ported again (see "mod existing ports").

## Phases

0. **Prior art and engine survey.** For each title:
   - the exact UE3 build (changelist), engine fork and licensee changes;
   - community SDKs and symbol maps, with their licences and build matches proven by
     cross-checked addresses. Examples: unrealsdk, already used by bl2, the Borderlands mod SDKs,
     UE Explorer and UModel;
   - existing native ports.
   Recorded in each repo's `docs/prior-art.md`.
1. **Two running titles first.** BL2 reaches its main menu (bl2 S005), and Gears 1 plays past its
   current focus. The shared layer is extracted from what both titles actually use, not designed
   ahead of them.
2. **Rename and split.** `x360ue3` becomes `ue3port` with an ABI-adapter seam. Add the Xbox 360
   adapter (from today's code) and an x86 adapter. Move BL2's engine anchors (`docs/RE/engine-anchors.md`)
   and Gears' equivalents onto the object-system and script-event owners. Both titles keep
   working through the split.
3. **Shared native renderer.** The RHI contract is executed by one Vulkan backend, proved pass by
   pass on BL2 (D3D9) and Gears 1 (Xenos) against the original frames.
4. **Gears 2, 3 and Judgment** as title adapters on the same layer.
5. **Further titles**, one repo each, in the order the user picks.

## Open decisions (for when this starts)

- The repo name and the move: rename `shared/x360ue3` in place, or create `shared/ue3port` and
  delete `x360ue3` in the same change.
- Gears 1: keep the Xbox 360 build, as gears1 G001 says, or also take the PC (GFWL) exe on x86port.
- Order of further titles.
