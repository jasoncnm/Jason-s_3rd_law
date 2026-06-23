/* date = June 22nd 2026 7:47 pm */

#ifndef SOUND_H
#define SOUND_H

#define MAX_SOUNDS 10

enum SoundType
{
    PLAYER_MOVE_SOUND,
    PLAYER_PUSH_SOUND,
    PLAYER_PROJ_SOUND,
    BLOCK_MOVE_SOUND,
    BLOCK_PROJ_SOUND,
    DOOR_OPEN_SOUND,
    STAR_COLLECT_SOUND,
    SOUNT_COUNT,
};

struct SoundData
{
    using SoundArray = Array<Sound, MAX_SOUNDS>;
    SoundArray audioClips[SOUNT_COUNT];
    
    int currentSound;
};

SoundData * LoadSoundData(BumpAllocator * allocator)
{
    SoundData * data = (SoundData *)BumpAlloc(allocator, sizeof(SoundData));
    
    return data;
}

#endif //SOUND_H
