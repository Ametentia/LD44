#if !defined(LUDUM_ASSETS_H_)
#define LUDUM_ASSETS_H_

enum Asset_Type {
    AssetType_Undefined = 0,
    AssetType_Texture,
    AssetType_Font,
    AssetType_Sound,
    AssetType_Music
};

struct Asset_Handle {
    u32 value;
};

struct Asset {
    bool loaded;
    Asset_Type type;
    union {
        sfTexture *texture;
        sfFont    *font;
        sfMusic   *music;
        sfSoundBuffer *sound;
    };
};

struct Asset_Manager {
    Asset *assets;
    u32 asset_count;
    u32 max_asset_count;
};

internal Asset_Handle LoadTexture(Asset_Manager *manager, char *filename);
internal Asset_Handle LoadFont(Asset_Manager *manager, char *filename);
internal Asset_Handle LoadSound(Asset_Manager *manager, char *filename);
internal Asset_Handle LoadMusic(Asset_Manager *manager, char *filename);

internal sfTexture *GetTexture(Asset_Manager *manager, Asset_Handle handle);
internal sfFont    *GetFont(Asset_Manager *manager, Asset_Handle handle);
internal sfMusic   *GetMusic(Asset_Manager *manager, Asset_Handle handle);
internal sfSoundBuffer *GetSound(Asset_Manager *manager, Asset_Handle handle);

#endif  // LUDUM_ASSETS_H_
