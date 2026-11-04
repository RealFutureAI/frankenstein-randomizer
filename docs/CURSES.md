# Curse mechanics and v0.5.22 balance

`L` is the effective skill level. Damage figures are base values before synergies, equipment, and enemy resistance. Native fixed-point damage precision, integer resistance percentages, and native immunity/slow rules affect the final result.

## Skill replacements

The game retains stable internal IDs and prerequisite links, while names and icons present the custom skills.

| Original internal skill | ID | Current role |
| --- | ---: | --- |
| Amplify Damage | 66 | Plague |
| Dim Vision | 71 | Curse Mastery |
| Weaken | 72 | Plague Lord |
| Iron Maiden | 76 | Soul Harvest |
| Terror | 77 | Infection |
| Confuse | 81 | Confuse with melee reflection |
| Life Tap | 82 | Elemental Marks |
| Attract | 86 | Attract with bonus physical damage and a death explosion |
| Decrepify | 87 | Time-growing Decrepify |
| Lower Resist | 91 | Reduction of all six resistance types |

## Plague and Infection

**Plague** starts at `25 + 12.5 × (L − 1)` poison damage per second. Its marked deaths trigger a 20% corpse-base-life explosion. Its three poison-damage synergies each give 10% per hard point: Infection, Corpse Explosion, and Poison Explosion.

**Infection** starts at `12.5 + 5 × (L − 1)` poison DPS. During one curse application, the poison rate is multiplied by `1.20` for each full elapsed second. Its initial cast area is small. On death, the target triggers a **33% corpse-base-life explosion**, split between physical and fire damage, and spreads Infection over a much larger area. The spread helper uses the same poison balance as the manually cast skill. Its synergies are Plague, Corpse Explosion, and Poison Explosion, each at 10% per hard point.

The two poison curses have independent rates and expiry tracking so they can coexist. Their poison damage is flat DPS, not target-maximum-life percentage damage. The on-death percentage describes the explosion's **damage**, not a 33% chance to spread.

## Confuse, Attract, and Soul Harvest

**Confuse** combines native confusion behavior with Iron Maiden-style reflection for physical melee attacks. It does not turn every ranged or spell hit into reflected damage. Elemental Marks and Attract each contribute 15% reflected-damage synergy per hard point.

**Attract** draws nearby enemies to the marked target and adds **50 + 15 × (L − 1)** physical damage per qualifying hit before synergies. Its death explosion starts at 60% of corpse base life. Confuse, Decrepify, and Infection each add 8% per hard point to both damage effects. The base explosion radius is 20 internal units and grows by one per additional level.

**Soul Harvest** marks enemies to drop a soul on death. Walk over a soul to restore **(1 + 0.3 × L)%** of maximum Life and Mana before synergies. Each invested point in Confuse or Attract adds a relative **3%** to this healing. For example, with a base heal of 7% and ten total hard points in those skills, each soul restores 9.1% of both resources.

Each soul also grants **(0.5 + 0.08 × L)%** maximum Life and Mana for **15 seconds**. Bonuses add together and expire independently. Only players can collect souls. The tooltip shows the calculated healing and maximum-resource bonus, with the two synergy sources below. The ground effect may be invisible even when pickup works.

## Decrepify

Decrepify starts with **zero strength**. Every complete second adds **3 percentage points** before synergies. Confuse, Attract, and Lower Resist each contribute a 5% relative growth-rate bonus per hard point.

For `S` total hard points across those sources and elapsed whole seconds `t`, the computed strength is:

```text
min(90, floor(3 × min(t, 30) × (100 + 5 × S) / 100))
```

Without synergies, it reaches the 90% computed cap after 30 seconds if the curse lasts that long. The progression applies independently to all affected targets, including eligible targets on Hell. The cast radius grows with skill level. Native physical resistance and movement/attack-speed limits can affect the result.

## Lower Resist

Lower Resist reduces fire, cold, lightning, poison, physical, and magic resistance. The previous unsynergized base curve is halved in v0.5.13. The original skill-level curve remains in use, with a **0.75% relative synergy bonus per hard point** from Plague, Infection, and Elemental Marks.

The formulas use integer ratios to preserve the fractional coefficient:

```text
floor(previous_base_curve × (400 + 3 × synergy_hard_points) / 800)
```

The tooltip prints the fractional synergy explicitly rather than asking the integer formula formatter to display a decimal value.

## Elemental Marks

All three base damage curves and their per-level growth were halved in v0.5.13. Confuse and Lower Resist each add 15% relative elemental damage per hard point.

### Fire

- Base pulse damage: `9.375 + 3.75 × (L − 1)`.
- An immediate first pulse, then at most one pulse every 75 game frames: **3 seconds per target**.
- A compact impact effect replaces the persistent ground-fire trails.
- A marked-target native missile hit handles damage and resistance; it is consumed once to avoid duplicate hits.

The earlier “triple damage every three seconds” cadence is retained. The newest balance reduction halves that pulse's base and level growth.

### Cold

- Frost Nova base damage: `37.5 + 15 × (L − 1)`.
- Triggers on every third registered hit; the hit counter has a five-frame per-target interval.
- Uses a larger 32-ray nova.
- Cold duration: `8 + 0.8 × (L − 1)` seconds before resistance and difficulty processing. Nightmare and Hell apply their native duration reductions.

### Lightning

- Damage range: `0.25` to `25 + 7.5 × (L − 1)`.
- Qualifying hits trigger Chain Lightning with a one-second per-target cooldown.
- Maximum five hits total: the first target plus four additional jumps.
- The chain can jump to uncursed enemies.

The elemental helper effects have recursion guards. A proc cannot continuously proc its own elemental effects. Trigger eligibility and native hit registration matter; the text does not describe an independent percentage roll for each element.

## Passive curses and skeletons

**Curse Mastery** has one investable point and allows different supported curses to coexist. The same curse still refreshes rather than stacking duplicate instances. Its condition is checked for the casting player.

**Plague Lord** applies to normal Skeleton Warriors from Raise Skeleton. A successful physical hit rolls `min(100, 1 + L)%` for one of the eight active curses. A successful proc has a one-second cooldown per skeleton. The caster owner is resolved through the native minion AI, including already-summoned warriors.

Raise Skeleton remains usable without a corpse. This convenience does not apply to every summon type. Neither passive participates in the synergy system.

## Regeneration correction

Earlier fire testing showed enemies repeatedly returning to their original health. Investigation found that the adapter added another regeneration event after the native regeneration function had already scheduled its own successor. The native scheduler allocates a new timer instead of deduplicating those calls.

Version 0.5.12 removed the extra successor and the fire/Decrepify regeneration seeds. Curse progression uses an independent server-frame pump. Poison transfer starts regeneration only if no suitable timer is already pending. Native healing rates remain unchanged; the fix does not force enemy regeneration to zero or manually subtract health to hide a reset.

The queue regression checks that 75 frames retain one native event chain and exactly 75 native-rate regeneration updates. Broader post-fix combat confirmation is still requested.
