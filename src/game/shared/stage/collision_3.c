#include "global.h"
#include "rect.h"
#include "sprite.h"
#include "lib/m4a/m4a.h"
#include "game/globals.h"
#include "game/shared/stage/collision.h"
#include "game/shared/stage/dust_cloud.h"
#include "game/shared/stage/entities_manager.h"
#include "game/shared/stage/player.h"

#include "game/sa2/stage/cheese.h"
#include "game/shared/stage/entity.h"
#include "game/shared/stage/mp_event_mgr.h"
#include "game/shared/stage/rings_scatter.h"
#include "game/sa2/stage/trapped_animals.h"

#include "constants/sa2/animations.h"
#include "constants/sa2/player_transitions.h"
#include "constants/sa2/songs.h"

// COLLISION 3

#ifndef COLLECT_RINGS_ROM
u32 Coll_Player_Entity_RectIntersection(Sprite *s, s32 sx, s32 sy, Player *p, Rect8 *rectPlayer)
{
    u32 result = 0;

    if (!HITBOX_IS_ACTIVE(s->hitboxes[0]) || !IS_ALIVE(p)) {
        return result;
    }

    if (RECT_COLLISION(sx, sy, &s->hitboxes[0].b, I(p->qWorldX), I(p->qWorldY), rectPlayer)) {
        result |= COLL_FLAG_80000;
    }

    return result;
}

#if (GAME == GAME_SA1)
u32 Coll_AmyHammer_Spring(Sprite *s, s16 worldX, s16 worldY, Player *p)
{
    bool32 isColliding = FALSE;

    if (p->character == CHARACTER_AMY) {
        if ((p->charState == CHARSTATE_87) || (p->charState == CHARSTATE_88) || (p->charState == CHARSTATE_89)
            || (p->charState == CHARSTATE_90)) {
            if (p->spriteInfoBody->s.hitboxes[1].index != HITBOX_STATE_INACTIVE) {
                if (HB_COLLISION(worldX, worldY, s->hitboxes[0].b, I(p->qWorldX), I(p->qWorldY), p->spriteInfoBody->s.hitboxes[1].b)) {
                    isColliding = TRUE;
                }
            }
        }
    }

    return isColliding;
}

u32 Coll_Player_Spring_Sideways(Sprite *s, CamCoord worldX, CamCoord worldY, Player *p)
{
    s8 rectDataPlayerA[4] = { -(p->spriteOffsetX + 5), (1 - p->spriteOffsetY), (p->spriteOffsetX + 5), (p->spriteOffsetY - 1) };
    s8 rectDataPlayerB[4] = { -(p->spriteOffsetX + 0), (0 - p->spriteOffsetY), (p->spriteOffsetX + 0), (p->spriteOffsetY + 0) };
    Rect8 *rectPlayerB;

    u32 moveState = 0;
    bool32 stoodOnCurrent = 0;

    if (s->hitboxes[0].index == -1) {
        return moveState;
    }

    if (!IS_ALIVE(p)) {
        return moveState;
    }

    moveState = p->moveState & MOVESTATE_IN_AIR;
    rectPlayerB = (Rect8 *)&rectDataPlayerB[0];
    if ((p->moveState & MOVESTATE_STOOD_ON_OBJ) && (p->stoodObj == s)) {
        p->moveState &= ~MOVESTATE_STOOD_ON_OBJ;
        moveState |= MOVESTATE_IN_AIR;
        stoodOnCurrent = 1;
    }

    if (moveState & MOVESTATE_IN_AIR) {
        if (HB_COLLISION(worldX, worldY, s->hitboxes[0].b, I(p->qWorldX), I(p->qWorldY), (*rectPlayerB))) {
            if (sub_800C934(s, worldX, worldY, (Rect8 *)&rectDataPlayerB, stoodOnCurrent, p, &moveState)) {
                return moveState;
            }
        } else if (HB_COLLISION(worldX, worldY, s->hitboxes[0].b, I(p->qWorldX), I(p->qWorldY), (*(Rect8 *)&rectDataPlayerA))) {
            if (I(p->qWorldX) <= worldX) {
                if (p->qSpeedAirX >= 0) {
                    p->qSpeedAirX = 0;
                    p->qWorldX = Q((worldX + s->hitboxes[0].b.left) - rectDataPlayerA[2]);
                    moveState |= MOVESTATE_20000;
                }
            } else if (p->qSpeedAirX <= 0) {
                p->qSpeedAirX = 0;
                p->qWorldX = Q(((worldX + s->hitboxes[0].b.right) - rectDataPlayerA[0]) + 1);
                moveState |= MOVESTATE_40000;
            }
        }
    }

    if (HB_COLLISION(worldX, worldY, s->hitboxes[0].b, I(p->qWorldX), I(p->qWorldY), (*(Rect8 *)&rectDataPlayerA))) {
        if (I(p->qWorldX) <= worldX) {
            if (p->qSpeedAirX >= 0) {
                moveState |= MOVESTATE_20000;

                if (p->qSpeedAirX > 0) {
                    moveState |= MOVESTATE_20;
                    moveState &= ~MOVESTATE_FACING_LEFT;
                    p->qWorldX = Q((worldX + s->hitboxes[0].b.left) - rectDataPlayerA[2]);
                }
            }
        } else {
            if (p->qSpeedAirX <= 0) {
                moveState |= MOVESTATE_40000;

                if (p->qSpeedAirX < 0) {
                    moveState |= MOVESTATE_20;
                    moveState |= MOVESTATE_FACING_LEFT;
                    p->qWorldX = Q(((worldX + s->hitboxes[0].b.right) - rectDataPlayerA[0]) + 1);
                }
            }
        }
    }

    return moveState;
}

// (99.92%) https://decomp.me/scratch/GFpFd
NONMATCH("asm/non_matching/game/shared/stage/collision__Coll_Player_Itembox.inc",
         u32 Coll_Player_Itembox(Sprite *s, CamCoord worldX, CamCoord worldY, Player *p))
{
    s8 rectDataPlayerA[4] = { -(p->spriteOffsetX + 5), (1 - p->spriteOffsetY), (p->spriteOffsetX + 5), (p->spriteOffsetY - 1) };
    s8 rectDataPlayerB[4] = { -(p->spriteOffsetX + 0), (0 - p->spriteOffsetY), (p->spriteOffsetX + 0), (p->spriteOffsetY + 0) };
    Rect8 *rectPlayerB = (Rect8 *)&rectDataPlayerB[0];

    u32 result;
    s32 middleX;
    s32 middleY;

    result = 0;
    if (s->hitboxes[0].index == -1) {
        return result;
    }

    if (!IS_ALIVE(p)) {
        return result;
    }

    if (p->spriteInfoBody->s.hitboxes[1].index != -1) {
        if (HB_COLLISION(worldX, worldY, s->hitboxes[0].b, I(p->qWorldX), I(p->qWorldY), p->spriteInfoBody->s.hitboxes[1].b)) {
            result |= 0x20;
            if (p->moveState & MOVESTATE_IN_AIR) {
                if (p->qSpeedAirY > 0) {
                    p->qSpeedAirY = -p->qSpeedAirY;
                }
            }
        }
    }

    if (p->moveState & MOVESTATE_IN_AIR) {
        middleX = worldX + ((s->hitboxes[0].b.left + s->hitboxes[0].b.right) >> 1);
        middleY = worldY + ((s->hitboxes[0].b.top + s->hitboxes[0].b.bottom) >> 1);

        if (HB_COLLISION(worldX, worldY, s->hitboxes[0].b, I(p->qWorldX), I(p->qWorldY), (*rectPlayerB))) {
            if ((!GRAVITY_IS_INVERTED && (I(p->qWorldY) <= middleY)) || (GRAVITY_IS_INVERTED && (I(p->qWorldY) >= middleY))) {
                if ((p->character == 1) && (p->SA2_LABEL(unk61) != 0)) {
                    Coll_Player_Platform(s, worldX, worldY, p);
                    return 0;
                } else if (p->qSpeedAirY >= 0) {
                    result |= 8;

                    if (p->qSpeedAirY > 0) {
                        p->qSpeedAirY = -p->qSpeedAirY;
                    }
                }
            } else if (p->qSpeedAirY < 0) {
                p->qSpeedAirY = 0;

                if (!GRAVITY_IS_INVERTED) {
                    p->qWorldY = p->qWorldY + (Q((worldY + s->hitboxes[0].b.bottom) - rectDataPlayerB[1]) - (0xFFFFFF00 & p->qWorldY));
                } else {
                    p->qWorldY = p->qWorldY - (Q((worldY + s->hitboxes[0].b.bottom) - rectDataPlayerB[1]) - (0xFFFFFF00 & p->qWorldY));
                }

                result |= 0x10000;
            }
        } else if (HB_COLLISION(worldX, worldY, s->hitboxes[0].b, I(p->qWorldX), I(p->qWorldY), (*(Rect8 *)&rectDataPlayerA[0]))) {
            if (I(p->qWorldX) <= middleX) {
                if (p->qSpeedAirX > 0) {
                    p->qSpeedAirX = 0;
                    p->qWorldX = Q(worldX + s->hitboxes[0].b.left - rectDataPlayerA[2]);
                }
            } else {
                if (p->qSpeedAirX < 0) {
                    p->qSpeedAirX = 0;
                    p->qWorldX = Q((worldX + s->hitboxes[0].b.right - rectDataPlayerA[0]) + 1);
                }
            }
        }
    }

    return result;
}
END_NONMATCH

// Used by Security Gate and Breakable Wall.
u32 Coll_Player_Gate(Sprite *s, CamCoord worldX, CamCoord worldY, Player *p, u32 arg4)
{
    u32 result = 0;
    s8 rectPlayer[4] = { -(p->spriteOffsetX + 5), (1 - p->spriteOffsetY), (p->spriteOffsetX + 5), (p->spriteOffsetY - 1) };

    if (s->hitboxes[0].index == -1) {
        return result;
    }

    if (!IS_ALIVE(p)) {
        return result;
    }

    if (arg4 != 0) {
        if (p->spriteInfoBody->s.hitboxes[1].index != -1) {
            if (HB_COLLISION(worldX, worldY, s->hitboxes[0].b, I(p->qWorldX), I(p->qWorldY), p->spriteInfoBody->s.hitboxes[1].b)
                || HB_COLLISION(worldX, worldY, s->hitboxes[0].b, I(p->qWorldX), I(p->qWorldY), (*(Rect8 *)&rectPlayer[0]))) {

                result |= 8;
                if ((p->moveState & MOVESTATE_IN_AIR) && (p->qSpeedAirY > 0)) {
                    p->qSpeedAirY = -p->qSpeedAirY;
                }

                return result;
            }
        }
    }

    sub_800CBBC(s, worldX, worldY, (Rect8 *)&rectPlayer, 0, p, &result);
    return result;
}

u32 Coll_Player_SkatingStone(Sprite *s, CamCoord worldX, CamCoord worldY, Player *p)
{
    s32 moveState;
    s32 var_sb;

    s8 rectPlayer[4] = { -p->spriteOffsetX, -p->spriteOffsetY, +p->spriteOffsetX, +p->spriteOffsetY };

    u32 result;

    result = 0;
    var_sb = 0;

    if ((s->hitboxes[0].index == -1) || (((0x80 & p->moveState) != 0))) {
        return 0U;
    }

    moveState = p->moveState & MOVESTATE_IN_AIR;
    if ((p->moveState & MOVESTATE_STOOD_ON_OBJ) && (p->stoodObj == s)) {
        p->moveState = p->moveState & ~MOVESTATE_STOOD_ON_OBJ;
        moveState |= MOVESTATE_IN_AIR;
        var_sb = 1;
    }

    if (((moveState == 0) || !sub_800C934(s, worldX, worldY, (Rect8 *)rectPlayer, var_sb, p, &result))
        && !sub_800C934(s, worldX, worldY, (Rect8 *)rectPlayer, var_sb, p, &result)) {
        if (var_sb) {
            if (!(p->moveState & MOVESTATE_STOOD_ON_OBJ)) {
                p->moveState = (p->moveState & ~MOVESTATE_20) | MOVESTATE_IN_AIR;
            }
        }
    }

    return result;
}
#endif

// (Link included because of register-match)
// (100.00%) https://decomp.me/scratch/0Ro0I
u32 Coll_Player_PlatformCrumbling(Sprite *s, s32 sx, s32 sy, Player *p)
{
    s8 rectPlayer[4] = { -p->spriteOffsetX, -p->spriteOffsetY, +p->spriteOffsetX, +p->spriteOffsetY };

    u32 result = COLL_NONE;
    bool32 ip = FALSE;

    if (!HITBOX_IS_ACTIVE(s->hitboxes[0]) || !IS_ALIVE(p)) {
        return result;
    }

    if ((p->moveState & MOVESTATE_STOOD_ON_OBJ) && (p->stoodObj == s)) {
        p->moveState &= ~MOVESTATE_STOOD_ON_OBJ;
        ip = TRUE;
    }

    if (RECT_COLLISION_2(sx, sy, &s->hitboxes[0].b, p->qWorldX, p->qWorldY, (Rect8 *)rectPlayer) && (p->qSpeedAirY >= 0)) {

#ifndef NON_MATCHING
        register s32 y asm("r1");
#else
        s32 y;
#endif

        rectPlayer[1] = -p->spriteOffsetY;
        rectPlayer[3] = +p->spriteOffsetY;
        p->moveState |= MOVESTATE_STOOD_ON_OBJ;
        result |= COLL_FLAG_8;

        if (!ip) {
            p->rotation = 0;
        }

        p->stoodObj = s;
        p->qSpeedAirY = 0;

        if (GRAVITY_IS_INVERTED) {
            y = s->hitboxes[0].b.bottom;
            y += sy;
            y += rectPlayer[3];
        } else {
            y = s->hitboxes[0].b.top;
            y += sy;
            y -= rectPlayer[3];
        }
        y = Q(y);
        p->qWorldY = Q_24_8_FRAC(p->qWorldY) + (y);
    } else if (ip && !(p->moveState & MOVESTATE_STOOD_ON_OBJ)) {
        p->moveState &= ~MOVESTATE_20;
        p->moveState |= MOVESTATE_IN_AIR;
    }

    return result;
}
#endif

#if (GAME == GAME_SA2)
bool32 Coll_Player_Entity_HitboxN(Sprite *s, s32 sx, s32 sy, s16 hbIndex, Player *p, s16 hbIndexPlayer)
{
    PlayerSpriteInfo *psi = p->spriteInfoBody;
    Sprite *sprPlayer = &psi->s;

    if (!IS_ALIVE(p)) {
        return FALSE;
    }

    if (!HITBOX_IS_ACTIVE(s->hitboxes[hbIndex])) {
        return FALSE;
    }

    if (!HITBOX_IS_ACTIVE(psi->s.hitboxes[hbIndexPlayer])) {
        return FALSE;
    }

    if ((HB_COLLISION(sx, sy, s->hitboxes[hbIndex].b, I(p->qWorldX), I(p->qWorldY), sprPlayer->hitboxes[hbIndexPlayer].b))) {
        return TRUE;
    }

    return FALSE;
}

#ifndef COLLECT_RINGS_ROM
bool32 Coll_Player_Boss_Attack(Sprite *s, s32 sx, s32 sy, s16 hbIndex, Player *p)
{
    PlayerSpriteInfo *psi = p->spriteInfoBody;
    Sprite *sprPlayer = &psi->s;

    if (!IS_ALIVE(p)) {
        return FALSE;
    }

    if (!HITBOX_IS_ACTIVE(s->hitboxes[hbIndex])) {
        return FALSE;
    }

    if (!HITBOX_IS_ACTIVE(sprPlayer->hitboxes[1])) {
        return FALSE;
    }

    if ((HB_COLLISION(sx, sy, s->hitboxes[hbIndex].b, I(p->qWorldX), I(p->qWorldY), sprPlayer->hitboxes[1].b))) {
        Coll_Player_Enemy_AdjustSpeed(p);
        return TRUE;
    }

    return FALSE;
}

bool32 Coll_Cheese_Enemy_Attack(Sprite *sprTarget, s32 sx, s32 sy, s16 hbIndex, Player *p)
{
    if (!IS_ALIVE(p)) {
        return FALSE;
    }

    if (!HITBOX_IS_ACTIVE(sprTarget->hitboxes[hbIndex])) {
        return FALSE;
    }

    if (gCheese) {
        Cheese *cheese = gCheese;

        if (!HITBOX_IS_ACTIVE(cheese->s.hitboxes[1])) {
            return FALSE;
        }

        if ((HB_COLLISION(sx, sy, sprTarget->hitboxes[hbIndex].b, I(cheese->posX), I(cheese->posY), cheese->s.hitboxes[1].b))) {
            return TRUE;
        }
    }

    return FALSE;
}
#endif
#endif

#ifndef COLLECT_RINGS_ROM
bool32 Coll_Player_Enemy_Attack(Sprite *s, s32 sx, s32 sy, u8 hbIndex)
{
    Player *player = &gPlayer;
    Sprite *sprPlayer = &player->spriteInfoBody->s;

    bool32 dead;
    u32 movestate;
    EnemyBase *eb;

    if (!HITBOX_IS_ACTIVE(s->hitboxes[hbIndex])) {
        return FALSE;
    }

    eb = TASK_DATA(gCurTask);
    dead = player->moveState & MOVESTATE_DEAD;
    movestate = player->moveState;

    if (!dead) {
        if (IS_MULTI_PLAYER && ((s8)eb->base.me->x == MAP_ENTITY_STATE_MINUS_THREE)) {
            CreateDustCloud(sx, sy);
            CreateTrappedAnimal(sx, sy);
            return TRUE;
        }

        if (!(movestate & MOVESTATE_IN_SCRIPTED)) {
            if (HITBOX_IS_ACTIVE(sprPlayer->hitboxes[1])) {
                if (HB_COLLISION(sx, sy, s->hitboxes[hbIndex].b, I(player->qWorldX), I(player->qWorldY), sprPlayer->hitboxes[1].b)) {
                    if (IS_MULTI_PLAYER) {
                        RoomEvent_EnemyDestroy *roomEvent = CreateRoomEvent();
                        roomEvent->type = ROOMEVENT_TYPE_ENEMY_DESTROYED;
                        roomEvent->x = eb->base.regionX;
                        roomEvent->y = eb->base.regionY;
                        roomEvent->id = eb->base.id;
                    }

                    Coll_Player_Enemy_AdjustSpeed(player);

                    CreateDustCloud(sx, sy);
                    CreateTrappedAnimal(sx, sy);
                    CreateEnemyDefeatScoreAndManageLives(sx, sy);

                    return TRUE;
                }
            }

            if (HITBOX_IS_ACTIVE(sprPlayer->hitboxes[0])
                && (HB_COLLISION(sx, sy, s->hitboxes[hbIndex].b, I(player->qWorldX), I(player->qWorldY), sprPlayer->hitboxes[0].b))) {
                if (!(player->itemEffect & PLAYER_ITEM_EFFECT__INVINCIBILITY)) {
                    Coll_DamagePlayer(player);
                } else {
                    if (IS_MULTI_PLAYER) {
                        RoomEvent_EnemyDestroy *roomEvent = CreateRoomEvent();
                        roomEvent->type = ROOMEVENT_TYPE_ENEMY_DESTROYED;
                        roomEvent->x = eb->base.regionX;
                        roomEvent->y = eb->base.regionY;
                        roomEvent->id = eb->base.id;
                    }

                    CreateDustCloud(sx, sy);
                    CreateTrappedAnimal(sx, sy);
                    CreateEnemyDefeatScoreAndManageLives(sx, sy);

                    return TRUE;
                }
            }
        }

        if (gCheese != NULL) {
            Cheese *cheese = gCheese;
            if (cheese->s.hitboxes[1].index != -1
                && ((HB_COLLISION(sx, sy, s->hitboxes[hbIndex].b, I(cheese->posX), I(cheese->posY), cheese->s.hitboxes[1].b)))) {
                if (IS_MULTI_PLAYER) {
                    RoomEvent_EnemyDestroy *roomEvent = CreateRoomEvent();
                    roomEvent->type = ROOMEVENT_TYPE_ENEMY_DESTROYED;
                    roomEvent->x = eb->base.regionX;
                    roomEvent->y = eb->base.regionY;
                    roomEvent->id = eb->base.id;
                }

                CreateDustCloud(sx, sy);
                CreateTrappedAnimal(sx, sy);
                CreateEnemyDefeatScoreAndManageLives(sx, sy);

                return TRUE;
            }
        }
    }

    return FALSE;
}

bool32 Coll_Player_Projectile(Sprite *s, s32 sx, s32 sy)
{
    Player *p;
    Sprite *sprPlayer;
    bool32 result = FALSE;

    if (gPlayer.moveState & MOVESTATE_IN_SCRIPTED) {
        return result;
    }

    if (HITBOX_IS_ACTIVE(s->hitboxes[0])) {
        p = &gPlayer;
        sprPlayer = &p->spriteInfoBody->s;

        if ((!PLAYER_IS_ALIVE) || !HITBOX_IS_ACTIVE(sprPlayer->hitboxes[0])) {
            return result;
        }

        if ((HB_COLLISION(sx, sy, s->hitboxes[0].b, I(p->qWorldX), I(p->qWorldY), sprPlayer->hitboxes[0].b))) {
            Coll_DamagePlayer(p);
            result = TRUE;
        }
    }

    return result;
}
#endif

#if (GAME == GAME_SA2)
// Completely different to SA1
bool32 Coll_Player_ItemBox(Sprite *s, s32 sx, s32 sy)
{
    bool32 result = FALSE;

    Player *p = &gPlayer;
    PlayerSpriteInfo *psi = p->spriteInfoBody;
    Sprite *sprPlayer = &psi->s;

    if (PLAYER_IS_ALIVE && HITBOX_IS_ACTIVE(sprPlayer->hitboxes[1]) && (HITBOX_IS_ACTIVE(s->hitboxes[0]))) {
        if (HB_COLLISION(sx, sy, s->hitboxes[0].b, I(p->qWorldX), I(p->qWorldY), sprPlayer->hitboxes[1].b)) {
            result = TRUE;
        }
    }

    return result;
}
#endif

#ifndef COLLECT_RINGS_ROM
bool32 Coll_Player_Enemy(Sprite *s, s32 sx, s32 sy, s16 hbIndex, Player *p)
{
    PlayerSpriteInfo *psi = p->spriteInfoBody;
    Sprite *sprPlayer = &psi->s;

    if (IS_ALIVE(p) && (HITBOX_IS_ACTIVE(s->hitboxes[hbIndex]) && HITBOX_IS_ACTIVE(sprPlayer->hitboxes[0]))) {
        if (HB_COLLISION(sx, sy, s->hitboxes[hbIndex].b, I(p->qWorldX), I(p->qWorldY), sprPlayer->hitboxes[0].b)) {
            Coll_DamagePlayer(p);
            return TRUE;
        }
    }

    return FALSE;
}

#if (GAME == GAME_SA2)
void Coll_Player_Enemy_AdjustSpeed(Player *p)
{
    if (p->moveState & MOVESTATE_BOOST_EFFECT_ON) {
        // Also triggered on homing-attack.
        // Slight boost upwards for the player.
        p->transition = PLTRANS_HOMING_ATTACK_RECOIL;
        p->qSpeedAirX = 0;
        p->qSpeedAirY = 0;
    } else if (IS_BOSS_STAGE(gCurrentLevel)) {
        s32 speedX = -(p->qSpeedAirX >> 1);
        p->qSpeedAirY = -p->qSpeedAirY;
        // BUG: using the camera DX here is not really fair, since
        // this will throw the player forwards if the camera is moving
        // towards the boss.
        // In reality this should use a fixed value of +Q(5) since that's
        // the boss moving speed
        p->qSpeedAirX = speedX - Q(gCamera.dx);
    } else if (p->qSpeedAirY > 0) {
        // Bounce off of enemies
        p->qSpeedAirY = -p->qSpeedAirY;
    }

    gPlayer.moveState |= MOVESTATE_4000;
}
#endif
#endif

// (100.00%) https://decomp.me/scratch/verla
// TODO: Register fake-match
bool32 Coll_DamagePlayer(Player *p)
{
    if (p->timerInvincibility > 0 || p->timerInvulnerability > 0) {
        return FALSE;
    }

    p->timerInvulnerability = PLAYER_INVULNERABLE_DURATION;

    if (p->moveState & MOVESTATE_1000000) {
        PlayerSpriteInfo *psi;

        p->layer = PLAYER_LAYER__BACK;

        p->moveState &= ~MOVESTATE_1000000;
        p->itemEffect &= ~PLAYER_ITEM_EFFECT__TELEPORT;

        p->spriteInfoBody->s.frameFlags &= ~SPRITE_FLAG_MASK_PRIORITY;
        p->spriteInfoBody->s.frameFlags |= SPRITE_FLAG(PRIORITY, 2);
    }

    if (!(p->moveState & MOVESTATE_1000000)) {
        p->transition = PLTRANS_HURT;
    }

    p->itemEffect &= ~PLAYER_ITEM_EFFECT__TELEPORT;

    if (!HAS_SHIELD(p)) {
        if (gRingCount > 0) {
            u32 rings = gRingCount;
            if (gGameMode == GAME_MODE_MULTI_PLAYER_COLLECT_RINGS) {
#ifndef NON_MATCHING
                register u32 rings2 asm("r0") = rings;
#else
                u32 rings2 = rings;
#endif
                if (rings > 10) {
                    rings2 = 10;
                }

                rings = rings2;
            }

            InitScatteringRings(I(p->qWorldX), I(p->qWorldY), rings);

            if (IS_MULTI_PLAYER) {
                RoomEvent_RingLoss *roomEvent = CreateRoomEvent();
                roomEvent->type = ROOMEVENT_TYPE_PLAYER_RING_LOSS;
                roomEvent->ringCount = rings;
            }

            gRingCount -= rings;
        } else if (!(gStageFlags & STAGE_FLAG__DEMO_RUNNING)) {
            p->moveState |= MOVESTATE_DEAD;
        }
    }
#ifndef COLLECT_RINGS_ROM
    else {
        m4aSongNumStart(SE_LIFE_LOST);
        p->itemEffect &= ~(PLAYER_ITEM_EFFECT__SHIELD_MAGNETIC | PLAYER_ITEM_EFFECT__SHIELD_NORMAL);
    }
#endif

    return TRUE;
}
