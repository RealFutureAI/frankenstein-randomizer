# Changelog

## 0.5.22

- Soul Harvest replaces Life Tap. Each collected soul restores 1% plus 0.3 percentage points per effective skill level of both Life and Mana. Confuse and Attract add 3% relative healing per invested point.
- Souls also grant 0.5% plus 0.08 percentage points per level to both maximums for 15 seconds. Each pickup has its own expiry.
- The skill tooltip displays calculated healing, the temporary maximum-resource bonus, and both synergy sources.
- Attract adds flat physical damage to qualifying hits and gains explosion radius per level. Its three synergies apply to both damage effects.
- Decrepify's cast radius grows with skill level, and its timed progression works on eligible targets on Hell.
- The source includes the Soul Harvest regression and English comments on the custom C++ code.

## 0.5.13

- Halved Lower Resist's base effect and reduced its hard-point synergy coefficient to 0.75%.
- Changed Decrepify to start at 0%, growing by 3 percentage points per second before synergies, with the 90% maximum retained.
- Reduced Plague and Infection poison-damage synergies to 10% per hard point.
- Reduced Infection's death explosion to 33% of corpse base life, retaining its spread behavior.
- Replaced Lower Resist with Infection as an Attract synergy source.
- Halved base damage and level growth for all three Elemental Marks damage types.
- Updated tooltips, source metadata, and regression expectations.

## 0.5.12

- Changed fire pulses to a three-second interval with three times the previous nominal pulse damage.
- Replaced the fire helper's very large percentage workaround with ordinary native damage coefficients.
- Removed duplicate regeneration successor events that could multiply timer chains, heal enemies rapidly, and stall the game.
- Stopped fire/Decrepify from seeding health-regeneration events; their server-frame timing remains independent.
- Added pending-timer detection before starting poison regeneration.
- Reduced dense-pack fire logging and added a regeneration-queue regression plus a native timer probe.

## 0.5.11

- Added direct marked-target resolution through the native missile-hit handler, instead of relying on a very short projectile flight to collide.
- Added consumed-target and expiry guards plus fire-damage diagnostics.

## 0.5.10

- Replaced Elemental Marks' persistent Blaze-style floor fire with compact periodic fire pulses.
- Initial combat feedback exposed unreliable fire delivery; corrected in subsequent builds.

## Earlier customizations retained

- Complete custom curse tree, hard-point synergies, independent poison curses, and Infection death chains.
- Plague Lord's working Skeleton Warrior owner resolution and corpse-free Raise Skeleton.
- Custom skill icons and native overhead color-keyframe adjustments.
- Three-hit larger Frost Nova, longer cold duration, and five-hit Chain Lightning.
- Separate `FMMP_Randomizer` save environment and the prebuilt Frankenstein randomizer data.

The version number describes the local curse customization layer. The bundled base mod identifies itself as FMMP Experimental R67 0.22.2 + Randomizer.
