#if !defined(SLIME_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Junjie Mao $
   $Notice: $
   ======================================================================== */

#define MAX_SLIME 50
#define MAX_MASS 3


#include "game_util.h"

//  ========================================================================
//              NOTE: Slime Structs
//  ========================================================================

enum SlimeState
{
    MOVE_STATE,
    SPLIT_STATE,
    FREEZE_STATE,
};

struct Slime
{
    SlimeState state = MOVE_STATE;

    IVec2 tile;
    IVec2 attachDir;

    Vector2 pivot;

    int mass = 1;
    int maxMass = 3;

    Sprite sprite;
    SpriteID id;

    bool show = true;
    bool split = false;
    bool attach = false;
};

struct SlimeAnimation
{
    IVec2 startTilePos;
    IVec2 endTilePos;
    
    Slime * currentSlime;
};

struct Player
{
    int moveSteps;
    int motherIndex;

    Color color = YELLOW;

    Array<Slime, MAX_SLIME> slimes;

};


//  ========================================================================
//              NOTE: Slime Globals
//  ========================================================================
bool animationPlaying = false;
int animateSlimeCount = 0;
SlimeAnimation animateSlimes[MAX_SLIME] = { 0 };

//  ========================================================================
//              NOTE: Slime Functions
//  ========================================================================

void UnFreezeAllSlime(Player & player)
{
    for (int i = 0; i < player.slimes.count; i++)
    {
        player.slimes[i].state = MOVE_STATE;
    }
}

Slime * CheckSlime(IVec2 tilePos, Player & player)
{
    Slime * result = nullptr;

    for (int i = 0; i < player.slimes.count; i++)
    {
        Slime * s = &player.slimes[i];
        if (s->tile == tilePos)
        {
            result = s; 
        }
    }

    return result;
    
}


SlimeAnimation CreateSlimeAnimation(Slime * refSlime,
                                    IVec2 startTile, IVec2 endTile,
                                    int startMass, int endMass)
{

    animationPlaying = true;

    SlimeAnimation result = { 0 };
    result.startTilePos = startTile;
    result.endTilePos = endTile;
    result.currentSlime = refSlime;
    
    return result;
    
}

void BounceSlime(Player & player, IVec2 dest)
{
    
    SM_ASSERT(player.slimes[player.motherIndex].mass > 0, "Slime is too small to bounce");
    
    bool merge = false;
    int mergedIndex = -1;

    IVec2 prevTile = player.slimes[player.motherIndex].tile;

    player.slimes[player.motherIndex].tile = dest;

    animateSlimes[animateSlimeCount++] =
        CreateSlimeAnimation(&player.slimes[player.motherIndex],
                             prevTile, dest,
                             player.slimes[player.motherIndex].mass, player.slimes[player.motherIndex].mass);
    
}

void SplitSlime(Player & player, IVec2 destPos, IVec2 prevPos)
{
    SM_ASSERT(player.slimes[player.motherIndex].mass > 0, "Slime is too small to split");
        
    Slime * refSlime = nullptr;
    
    // NOTE: Create a small slime and shoot at the destination
    {
        Slime s = { };
        s.tile = destPos;
        s.mass = 1;
        s.show = false;
        s.split = true;

        int index = player.slimes.Add(s);
        refSlime = &player.slimes[index];

        // NOTE: split Animation
        animateSlimes[animateSlimeCount++] =
            CreateSlimeAnimation(refSlime,
                                 prevPos, destPos,
                                 s.mass, s.mass);
    }

}

void Posses(Player & player, int childIndex)
{
    player.motherIndex = childIndex;
}

void Merge(Player & player, SlimeAnimation & sa)
{

    for (int j = 0; j < player.slimes.count; j++)
    {
                            
        Slime * slime = &player.slimes[j];

        if (sa.currentSlime != slime && !slime->split)
        {

            Rectangle rec1 = { sa.currentSlime->pivot.x, sa.currentSlime->pivot.y,
                MAP_TILE_SIZE, MAP_TILE_SIZE };

            Rectangle rec2 = { slime->pivot.x, slime->pivot.y, MAP_TILE_SIZE, MAP_TILE_SIZE };

            IVec2 tilePos = PixelPositionToTilePosition(sa.currentSlime->pivot.x, sa.currentSlime->pivot.y);

            // IMPORTANT: This is framerate dependent for not, to make it framerate independent,
            //            You need to implement fix timestep see:
            //              https://gafferongames.com/post/fix_your_timestep/ 
            //              https://www.youtube.com/watch?v=VpSWuywFlC8
            //  NOTE:    I think I should test if low frame rate actually affect the detection, then thinking of the above impl
            if (tilePos == slime->tile) 
                // if (CheckCollisionRecs(rec1, rec2))
            {
                sa.currentSlime->mass += slime->mass;
                if (sa.currentSlime->mass > 3)
                {
                    sa.currentSlime->mass = 3;
                }

                player.slimes.RemoveIdxAndSwap(j);
                
                j--;

                if (sa.currentSlime == &player.slimes[player.motherIndex])
                {
                    for (int k = 0; k < animateSlimeCount; k++)
                    {
                        if (player.slimes[j].tile == animateSlimes[k].currentSlime->tile)
                        {
                            animateSlimes[k].currentSlime = &player.slimes[j];
                        }
                    }
                }
                else if (player.slimes[j].tile == sa.currentSlime->tile)
                {
                    sa.currentSlime = slime;
                }
                break;
            }
        }
    }
}


#define SLIME_H
#endif
