/* Seamless Episodes: a native mod for the Super Mario Sunshine PC port.
 *
 * A Shine that ends an episode normally sends Mario out to Delfino Plaza,
 * and back in through the painting and the episode select. With this mod,
 * once the Shine Get is over and the Shine is saved (by the save prompt or
 * Auto Save), the game goes straight on to the world's next episode, which
 * starts as any does: its opening cutscene and its name.
 *
 * It decides on the host only: in singleplayer, or when this game hosts
 * multiplayer (the players follow the host, as they always do). In a game
 * that joined someone else's it does nothing.
 *
 * How (mod API 1.4): SMS_EV_STAGE_LEAVING says why the stage is left
 * (SMS_LEAVE_SHINE) and which Shine it was, and lets a handler choose where
 * the game goes; api->world says which world an area is part of and which
 * episode comes next, as the episode select would offer it.
 */
#include <sms_mod.h>

#include <stddef.h>
#include <stdio.h>
#include <string.h>

static const SmsApi* api;

/* the "next" option */
#define NEXT_NEWEST 0 /* the episode the episode select offers: after the newest one got */
#define NEXT_AFTER  1 /* the episode after the one just played */

/* This game decides where the game goes: singleplayer, or the host of a
   multiplayer game. */
static int deciding(void)
{
	return !SMS_API_HAS(api, net) || api->net->role() != SMS_NET_CLIENT;
}

/* The episode to go on to after `shine` (from `episode` of `world`), or -1:
   none is left, and the game goes to the plaza as ever. */
static int next_episode(SmsMod* mod, int world, int episode, int shine)
{
	const int count = api->world->episode_count(world);
	int next;
	if (api->config->get_int(mod, "next", NEXT_NEWEST) == NEXT_AFTER) {
		int area = -1;
		const int played = shine >= 0 ? api->world->shine_episode(shine, &area) : -1;
		const int from   = played >= 0 && area == world ? played : episode;
		if (from < 0 || from + 1 >= count)
			return -1; /* the world's last episode */
		return from + 1;
	}
	next = api->world->next_episode(world);
	/* every episode got (the select offers the last again): back to the plaza */
	if (next < 0 || api->game->has_shine(api->world->episode_shine(world, next)))
		return -1;
	return next;
}

static int on_leaving(SmsMod* mod, SmsEvent* ev, void* user)
{
	SmsEvStage* to = (SmsEvStage*)ev->data;
	SmsGameState st;
	char name[96];
	int world, next;
	(void)user;

	/* a game too old to say why it leaves (API 1.3 and before) */
	if (!to || ev->size < offsetof(SmsEvStage, shine) + sizeof to->shine)
		return SMS_CONTINUE;
	/* a Shine ended the stage, and the game goes back to the plaza */
	if (to->reason != SMS_LEAVE_SHINE || to->area != 1 || !deciding())
		return SMS_CONTINUE;

	memset(&st, 0, sizeof st);
	st.size = sizeof st;
	api->game->state(&st);
	world = st.area >= 0 ? api->world->world_of(st.area) : -1;
	if (world < 0 || api->world->episode_count(world) <= 0)
		return SMS_CONTINUE; /* the Airstrip, the plaza's secret courses, Corona Mountain */

	next = next_episode(mod, world, st.episode, to->shine);
	if (next < 0) {
		api->core->log(mod, SMS_LOG_INFO, "Shine %d: no episode left in %s, to the plaza", to->shine,
		               api->game->area_name(world));
		return SMS_CONTINUE;
	}
	if (api->world->episode_name(world, next, name, sizeof name) <= 0)
		snprintf(name, sizeof name, "episode %d", next + 1);
	api->core->log(mod, SMS_LOG_INFO, "Shine %d: on to %s, episode %d (%s)", to->shine, api->game->area_name(world),
	               next + 1, name);
	to->area    = world;
	to->episode = next;
	return SMS_CONTINUE;
}

SMS_MOD_DEFINE()

SMS_MOD_EXTERN_C SMS_MOD_EXPORT int sms_mod_load(SmsMod* mod, const SmsApi* a)
{
	static const char* const kNext[] = {"The newest episode", "The episode after this one"};
	SmsOption next;
	api = a;

	if (!SMS_API_HAS(api, world)) {
		api->core->log(mod, SMS_LOG_ERROR, "needs a game with the mod API's worlds (api->world, API 1.4; this game has %u.%u): update the game", api->major,
		               api->minor);
		return 1;
	}

	memset(&next, 0, sizeof next);
	next.size         = sizeof next;
	next.key          = "next";
	next.label        = "After a Shine, go on to";
	next.type         = SMS_OPTION_CHOICE;
	next.def          = NEXT_NEWEST;
	next.choices      = kNext;
	next.choice_count = 2;
	api->config->add_option(mod, &next);

	/* late: after mods that pick a stage of their own, which then win */
	api->core->subscribe(mod, SMS_EV_STAGE_LEAVING, on_leaving, NULL, SMS_PRIORITY_LATE);
	api->core->log(mod, SMS_LOG_INFO, "loaded");
	return 0;
}
