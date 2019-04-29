internal void InitialiseAssetManager(Asset_Manager *manager, u32 count) {
    manager->max_asset_count = count;
    manager->asset_count = 1;

    umm assets_size = sizeof(Asset) * manager->max_asset_count;
    manager->assets = cast(Asset *) Alloc(assets_size);
    memset(manager->assets, 0, assets_size);
}

internal Asset_Handle LoadTexture(Asset_Manager *manager, char *filename) {
    Asset_Handle result = { manager->asset_count++ };
    Assert(result.value < manager->max_asset_count);

    Asset *asset = &manager->assets[result.value];

    asset->texture = sfTexture_createFromFile(filename, 0);

    asset->type = AssetType_Texture;
    asset->loaded = true;
    return result;
}

internal Asset_Handle LoadFont(Asset_Manager *manager, char *filename) {
    Asset_Handle result = { manager->asset_count++ };
    Assert(result.value < manager->max_asset_count);

    Asset *asset = &manager->assets[result.value];

    asset->font = sfFont_createFromFile(filename);

    asset->type = AssetType_Font;
    asset->loaded = true;
    return result;
}

internal Asset_Handle LoadSound(Asset_Manager *manager, char *filename) {
    Asset_Handle result = { manager->asset_count++ };
    Assert(result.value < manager->max_asset_count);

    Asset *asset = &manager->assets[result.value];

    asset->sound = sfSoundBuffer_createFromFile(filename);

    asset->type = AssetType_Sound;
    asset->loaded = true;
    return result;
}

internal Asset_Handle LoadMusic(Asset_Manager *manager, char *filename) {
    Asset_Handle result = { manager->asset_count++ };
    Assert(result.value < manager->max_asset_count);

    Asset *asset = &manager->assets[result.value];

    asset->music = sfMusic_createFromFile(filename);

    asset->type = AssetType_Music;
    asset->loaded = true;
    return result;
}

internal Asset *GetAsset(Asset_Manager *manager, u32 index) {
    Assert(index < manager->max_asset_count);
    Asset *result = &manager->assets[index];
    return result;
}

internal sfTexture *GetTexture(Asset_Manager *manager, Asset_Handle handle) {
    if (handle.value == 0) { return 0; }

    Asset *asset = GetAsset(manager, handle.value);
    Assert(asset->type == AssetType_Texture && asset->loaded);

    sfTexture *result = asset->texture;
    return result;
}

internal sfFont *GetFont(Asset_Manager *manager, Asset_Handle handle) {
    if (handle.value == 0) { return 0; }

    Asset *asset = GetAsset(manager, handle.value);
    Assert(asset->type == AssetType_Font && asset->loaded);

    sfFont *result = asset->font;
    return result;
}

internal sfSoundBuffer *GetSound(Asset_Manager *manager, Asset_Handle handle) {
    if (handle.value == 0) { return 0; }

    Asset *asset = GetAsset(manager, handle.value);
    Assert(asset->type == AssetType_Sound && asset->loaded);

    sfSoundBuffer *result = asset->sound;
    return result;
}

internal sfMusic *GetMusic(Asset_Manager *manager, Asset_Handle handle) {
    if (handle.value == 0) { return 0; }

    Asset *asset = GetAsset(manager, handle.value);
    Assert(asset->type == AssetType_Music && asset->loaded);

    sfMusic *result = asset->music;
    return result;
}

internal Animation CreateAnimation(Asset_Manager *assets, Asset_Handle texture_handle,
        u32 rows, u32 columns, f32 time_per_frame)
{
    Animation result = {};

    result.handle = texture_handle;

    sfTexture *texture = GetTexture(assets, texture_handle);
    sfVector2u size = sfTexture_getSize(texture);

    result.width  = size.x / columns;
    result.height = size.y / rows;

    result.rows = rows;
    result.columns = columns;
    result.total_frames = rows * columns;
    result.current_frame = 0;
    result.accumulator = 0;

    result.time_per_frame = time_per_frame;

    return result;
}

internal void UpdateAnimation(Animation *animation, f32 dt) {
    animation->accumulator += dt;
    if (animation->accumulator >= animation->time_per_frame) {
        animation->accumulator = 0;
        u32 current_frame = animation->current_frame;
        animation->current_frame = (current_frame + 1) % animation->total_frames;
    }
}

internal void RenderAnimation(Game_State *state,
        Animation *animation, v2 position, v2 scale = V2(1, 1))
{
    u32 current_frame = animation->current_frame;

    sfSprite *sprite = sfSprite_create();
    sfSprite_setOrigin(sprite, V2(animation->width / 2, animation->height / 2));
    sfSprite_setPosition(sprite, position);
    sfSprite_setScale(sprite, scale);

    sfTexture *texture = GetTexture(&state->assets, animation->handle);
    sfSprite_setTexture(sprite, texture, true);

    u32 row = cast(u32) ((cast(f32) current_frame) / (cast(f32) animation->columns));
    u32 col = current_frame % animation->columns;

    sfIntRect texture_rect = {
         cast(s32) (col * animation->width), cast(s32) (row * animation->height),
         cast(s32) animation->width,         cast(s32) animation->height
    };
    sfSprite_setTextureRect(sprite, texture_rect);

    sfRenderWindow_drawSprite(state->renderer, sprite, 0);

    sfSprite_destroy(sprite);
}

