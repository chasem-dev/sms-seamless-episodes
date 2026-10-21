# Seamless Episodes

A mod for the [Super Mario Sunshine PC port](https://github.com/chasem-dev/sms-pc-port): after a Shine Sprite is saved, the world's next episode starts right away.

In the game, a Shine that ends an episode sends Mario out to Delfino Plaza, and back in through the world's painting or pipe and its episode select. With Seamless Episodes, once the Shine Get is over and the Shine is saved (by the save prompt or Auto Save), the game loads the same world's next episode straight away. It starts as any episode does, with its opening cutscene and its name.

- **Next episode:** the one the episode select would offer: the episode after the newest one you have. Its setting in the game's Mods menu (F1), *After a Shine, go on to*, can pick the episode after the one you just played instead.
- **Back to the plaza when a world is done:** once a world has no episode left to get, a Shine sends you to the plaza as usual. So do the Airstrip, the plaza's own secret courses and Corona Mountain.
- **Secret courses:** a secret course's Shine is its episode's, so it goes on to the next episode too.
- **Multiplayer:** it decides on the host only. In singleplayer, or when you host, the game goes on to the next episode, and the players who joined follow you there, as they follow you anywhere. In a game you joined, it does nothing.
- **Everything else stays the same:** you save as before, the coin count starts again from zero in the new episode as it would after the plaza, and the plaza's story moments still happen the next time you go there.
- **Not in Super Mario Eclipse,** whose stages are its own.

## Install

In [SMS Launcher](https://github.com/chasem-dev/sms-launcher): **Settings → Mods → Seamless Episodes → Download**, and switch it on or off there. The launcher keeps it up to date.

By hand: unzip a [release](https://github.com/chasem-dev/sms-seamless-episodes/releases) into the game's `mods/` folder, so that you have `mods/seamless-episodes/mod.ini`. The game's Mods menu (F1) switches it on and off.

It needs a game with mod API 1.4 or newer. An older game leaves it off and says so in its log.

## How it works

It uses two things from the [mod SDK](https://github.com/chasem-dev/sms-mod-sdk)'s API 1.4:

- **`SMS_EV_STAGE_LEAVING`** says why the game is leaving a stage (`reason == SMS_LEAVE_SHINE`) and which Shine it was, once the Shine Get and the save are over, and lets a mod say where the game goes instead. A world's area with an episode named goes straight to that episode, past the episode select, as if it had been chosen there.
- **`api->world`** says which world an area is part of (`world_of`) and which episode comes next (`next_episode`, `shine_episode`, `episode_name`).

The whole mod is [`src/seamless_episodes.c`](src/seamless_episodes.c).

## Build

```sh
cmake -S . -B build -DSMS_MODS_OUT=/path/to/the/game/mods   # -DSMS_MOD_ARCH=32 for the 32-bit Linux game
cmake --build build
```

CMake fetches the SDK at the version in `CMakeLists.txt` and puts the mod in `<SMS_MODS_OUT>/seamless-episodes/`.
A tag (`v1.0.1`) builds it for Windows, Linux and macOS and releases one zip, which the launcher downloads.

## License

MIT
