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

    Color color = YELLOW;

    Slime mother;

    Array<Slime, MAX_SLIME> children;

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
    player.mother.state = MOVE_STATE;
    for (int i = 0; i < player.children.count; i++)
    {
        player.children[i].state = MOVE_STATE;
    }
}

Slime * CheckSlime(IVec2 tilePos, Player & player)
{
    Slime * result = nullptr;
    if (player.mother.tile == tilePos)
    {
        result = &player.mother;
    }

    for (int i = 0; i < player.children.count; i++)
    {
        Slime * s = &player.children[i];
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
    
    SM_ASSERT(player.mother.mass > 0, "Slime is too small to bounce");
    
    bool merge = false;
    int mergedIndex = -1;

    IVec2 prevTile = player.mother.tile;

    player.mother.tile = dest;

    animateSlimes[animateSlimeCount++] =
        CreateSlimeAnimation(&player.mother,
                             prevTile, dest,
                             player.mother.mass, player.mother.mass);
    
}

void SplitSlime(Player & player, IVec2 destPos, IVec2 prevPos)
{
    SM_ASSERT(player.mother.mass > 0, "Slime is too small to split");
        
    Slime * refSlime = nullptr;
    
    // NOTE: Create a small slime and shoot at the destination
    {
        Slime s = { };
        s.tile = destPos;
        s.mass = 1;
        s.show = false;
        s.split = true;

        int index = player.children.Add(s);
        refSlime = &player.children[index];

        // NOTE: split Animation
        animateSlimes[animateSlimeCount++] =
            CreateSlimeAnimation(refSlime,
                                 prevPos, destPos,
                                 s.mass, s.mass);
    }

}

void Posses(Slime * mother, Slime * child)
{
    Slime tmp = *mother;
    *mother = *child;
    *child = tmp;
}

void Merge(Player & player, SlimeAnimation & sa)
{

    for (int j = 0; j < player.children.count; j++)
    {
                            
        Slime * slime = &player.children[j];

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

                player.children.RemoveIdxAndSwap(j);
                
                j--;

                if (sa.currentSlime == &player.mother)
                {
                    for (int k = 0; k < animateSlimeCount; k++)
                    {
                        if (player.children[j].tile == animateSlimes[k].currentSlime->tile)
                        {
                            animateSlimes[k].currentSlime = &player.children[j];
                        }
                    }
                }
                else if (player.children[j].tile == sa.currentSlime->tile)
                {
                    sa.currentSlime = slime;
                }
                break;
            }
        }
    }
}

void SetSlimePosition(Slime & slime, IVec2 tilePos)
{
    slime.tile = tilePos;
    slime.pivot = GetTilePivot(tilePos);
}


#define SLIME_H
#endif
