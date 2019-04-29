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

