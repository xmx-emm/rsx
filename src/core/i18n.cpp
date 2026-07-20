#include <pch.h>
#include <core/i18n.h>

#include <thirdparty/imgui/imgui.h>

// [i18n]: current UI language. plain read/write is fine here: worker threads only ever
// read this value and a stale read simply yields the previous language for one string.
static eUILanguage s_currentLanguage = eUILanguage::UILANG_ENGLISH;

void I18N_SetLanguage(const eUILanguage language)
{
    if (language < eUILanguage::UILANG_COUNT)
        s_currentLanguage = language;
}

eUILanguage I18N_GetLanguage()
{
    return s_currentLanguage;
}

// ==============================================================================================
// Simplified Chinese string table. key: english source string, value: translation.
// ==============================================================================================
static const std::unordered_map<std::string_view, const char*> s_StringsChineseSimplified =
{
    // main menu bar
    { "File", "文件" },
    { "Open", "打开" },
    { "Unload Files", "卸载文件" },
    { "Unable to open new files while file loading is still in progress", "文件仍在加载中，暂时无法打开新文件" },
    { "Edit", "编辑" },
    { "Settings", "设置" },
    { "Skin Finder", "皮肤查找器" },
    { "Logs", "日志" },

    // welcome box
    { "To get started, click the \"Open File\" button below to load a file, or visit the \"Edit > Settings\" menu to modify your settings.",
      "开始使用：点击下方“打开文件”按钮加载文件，或前往“编辑 > 设置”菜单调整设置。" },
    { "Open File...", "打开文件..." },
    { "Compatible Game Installations", "检测到的兼容游戏" },
    { "Path", "路径" },
    { "Open common.rpak", "打开 common.rpak" },
    { "Open Skin Finder", "打开皮肤查找器" },
    { "There's a new update available for RSX! Download the latest %s version from", "RSX 有新版本可用！点击下载最新 %s 版本：" },
    { "here!", "这里！" },

    // asset list
    { "Export", "导出" },
    { "Export selected asset", "导出选中的资产" },
    { "Export selected assets", "导出选中的资产" },
    { "Export all for selected type", "导出所选类型的全部资产" },
    { "Export all for selected pak", "导出所选 Pak 的全部资产" },
    { "Export all for selected pak and type", "导出所选 Pak 和类型的全部资产" },
    { "Export all", "导出全部" },
    { "Export list of asset names...", "导出资产名称列表..." },
    { "{} assets", "{} 个资产" },
    { "Double-click the name of an asset to export", "双击资产名称即可导出" },
    { "Filter (incl,-excl)", "过滤（包含,-排除）" },
    { "Type", "类型" },
    { "Name", "名称" },
    { "Copy asset names", "复制资产名称" },
    { "Copy asset guids", "复制资产 GUID" },
    { "Copied!", "已复制！" },
    { "Exported %lld asset!", "已导出 %lld 个资产！" },
    { "Exported %lld assets!", "已导出 %lld 个资产！" },

    // asset info
    { "Quick Export", "快速导出" },
    { "(none)", "（无）" },
    { "Number of Dependencies: %hu", "依赖数量：%hu" },
    { "This is the number of assets in the same .rpak file as this asset that are required by this asset",
      "与此资产位于同一 .rpak 文件中、且被此资产依赖的资产数量" },
    { "Number of Dependent Assets: %hu", "被依赖数量：%hu" },
    { "This is the number of assets in the same .rpak file as this asset that use this asset",
      "与此资产位于同一 .rpak 文件中、且使用此资产的资产数量" },
    { "Asset type '%s' does not currently support Asset Preview.", "资产类型 '%s' 暂不支持预览。" },
    { "This asset type was not loaded because of your current settings.\nYou can change this by visiting Settings > %s (%s) > Load Asset Type",
      "由于当前设置，此资产类型未被加载。\n可前往 设置 > %s (%s) > 加载资产类型 进行修改" },
    { "Asset type '%s' is not currently supported.", "资产类型 '%s' 暂不支持。" },
    { "No asset selected.", "未选择资产。" },

    // dependency table
    { "Asset Dependencies", "资产依赖" },
    { "Status", "状态" },
    { "Exported", "已导出" },
    { "Loaded", "已加载" },
    { "Not Loaded", "未加载" },

    // settings window
    { "Language", "语言" },
    { "Display language of the RSX user interface.", "RSX 界面的显示语言。" },
    { "General", "常规" },
    { "Check for updates", "检查更新" },
    { "RSX will check for updates against the GitHub repository when opened.\nIf there is a new update available, a message will be displayed on the RSX welcome dialog box",
      "RSX 启动时将通过 GitHub 仓库检查更新。\n如有新版本，将在欢迎界面显示提示信息" },
    { "Export full asset paths", "按完整路径导出资产" },
    { "Enables exporting of assets to their full path within the export directory, as shown by the listed asset names.\nWhen disabled, assets will be exported into the root-level of a folder named after the asset's type (e.g. \"material/\",\"ui_image/\").",
      "启用后，资产将按列表中显示的完整路径导出到导出目录。\n禁用时，资产将直接导出到以资产类型命名的文件夹根目录（例如 \"material/\"、\"ui_image/\"）。" },
    { "Export asset dependencies", "导出资产依赖" },
    { "Enables exporting of all dependencies that are associated with any asset that is being exported.",
      "启用后，导出资产时将同时导出与其关联的全部依赖资产。" },
    { "Disable CacheDB loading", "禁用缓存数据库加载" },
    { "Disables loading names from the cache file. The cache will still be updated, but RSX will not display any cached data",
      "禁止从缓存文件加载名称。缓存仍会更新，但 RSX 不会显示任何缓存数据" },
    { "Export (Textures)", "导出（纹理）" },
    { "Material Texture Naming", "材质纹理命名" },
    { "Naming scheme for exporting textures via materials options are as follows:\nGUID: exports only using the asset's GUID as a name.\nReal: exports texture using a real name (asset name or guid if no name).\nText: exports the texture with a text name always, generating one if there is none provided.\nSemantic: exports with a generated name all the time, useful for models.",
      "通过材质导出纹理时的命名方式：\nGUID：仅使用资产的 GUID 作为名称。\n真实名称：使用真实名称导出（无名称时回退为 GUID）。\n文本名称：始终使用文本名称导出，没有名称时自动生成。\n语义名称：始终使用自动生成的语义名称，适用于模型。" },
    { "Normal Recalculation", "法线重建" },
    { "None: exports the normal as it is stored.\nDirectX: exports with a generated blue channel.\nOpenGL: exports with a generated blue channel and inverts the green channel.",
      "无：按原始存储数据导出法线。\nDirectX：导出时生成蓝色通道。\nOpenGL：导出时生成蓝色通道并反转绿色通道。" },
    { "Export Material Textures", "导出材质纹理" },
    { "Enables exporting of all textures that are associated with any material asset that is being exported.",
      "启用后，导出材质资产时将同时导出与其关联的全部纹理。" },
    { "Export (Models)", "导出（模型）" },
    { "Export Sequences", "导出动画序列" },
    { "Enables exporting of all animation sequences that are associated with any rig or model asset that is being exported.",
      "启用后，导出骨架或模型资产时将同时导出与其关联的全部动画序列。" },
    { "Export Skin", "导出皮肤" },
    { "Enables exporting a model with the previewed skin.", "启用后，将按预览中选择的皮肤导出模型。" },
    { "Truncate Materials", "截断材质名" },
    { "Truncates material names on SMD.", "在 SMD 中截断材质名称。" },
    { "Enable QCI Files", "启用 QCI 文件" },
    { "QC file will be split into multiple include files.", "QC 文件将被拆分为多个 include 文件。" },
    { "QC Target Version", "QC 目标版本" },
    { "Desired version for QC files to be compatible with.", "QC 文件需要兼容的目标版本。" },
    { "Physics contents filter", "物理内容过滤器" },
    { "Filter physics meshes in or out based on selected contents.", "根据所选内容筛选（保留或排除）物理网格。" },
    { "Physics contents filter exclusive", "物理过滤器排除模式" },
    { "Exclude physics meshes containing any of the specified contents.", "排除包含任一指定内容的物理网格。" },
    { "Physics contents filter require all", "物理过滤器要求全部匹配" },
    { "Filter only physics meshes containing all specified contents.", "仅筛选包含全部指定内容的物理网格。" },
    { "Parsing", "解析" },
    { "Compression Level", "压缩等级" },
    { "Specifies the compression level used when storing parsed assets in memory.\nWARNING: Modify only if you know what you?re doing; otherwise, you may run out of memory.\nNone: no compression.\nSuper Fast: Fastest level with the lowest compression ratio.\nVery Fast: Standard setting; fastest level with a decent compression ratio.\nFast: Fastest level with a good compression ratio.\nNormal: Standard LZ speed with the highest compression ratio.",
      "指定在内存中存储已解析资产时使用的压缩等级。\n警告：请仅在了解其作用时修改，否则可能导致内存不足。\n无：不压缩。\n超快：速度最快，压缩率最低。\n很快：标准设置，速度很快且压缩率不错。\n快：速度较快，压缩率良好。\n标准：标准 LZ 速度，压缩率最高。" },
    { "Parse Threads", "解析线程数" },
    { "The number of CPU threads that will be used for loading files.\n\nIn general, the higher the number, the faster RSX will be able to load the selected files.",
      "用于加载文件的 CPU 线程数。\n\n一般来说，线程数越多，RSX 加载所选文件的速度越快。" },
    { "Export Threads", "导出线程数" },
    { "The number of CPU threads that will be used for exporting assets.\n\nA higher number of threads will usually make RSX export assets more quickly, however the increased disk usage may cause decreased performance.",
      "用于导出资产的 CPU 线程数。\n\n线程数越多导出通常越快，但磁盘占用增加可能反而降低性能。" },
    { "Preview", "预览" },
    { "Cull Distance", "剔除距离" },
    { "Distance at which render of 3D objects will stop.", "超过该距离后将停止渲染 3D 物体。" },
    { "Asset Settings", "资产设置" },
    { "Load Asset Type", "加载资产类型" },
    { "This setting determines whether this asset type should be processed by RSX when loading asset files.\n\nIMPORTANT: This setting can only be changed before selecting pak files to open.",
      "该设置决定 RSX 在加载资产文件时是否处理此资产类型。\n\n重要：只能在选择要打开的 pak 文件之前修改此设置。" },
    { "Format", "格式" },

    // export settings combo items (core/utils/exportsettings.h)
    { "None", "无" },
    { "Real", "真实名称" },
    { "Text", "文本名称" },
    { "Semantic", "语义名称" },
    { "Super Fast", "超快" },
    { "Very Fast", "很快" },
    { "Fast", "快" },
    { "Normal", "标准" },

    // per-asset export format names (translated transparently through TRCombo)
    { "PNG (Highest Mip)", "PNG（最高 Mip）" },
    { "PNG (All Mips)", "PNG（全部 Mip）" },
    { "DDS (Highest Mip)", "DDS（最高 Mip）" },
    { "DDS (All Mips)", "DDS（全部 Mip）" },
    { "DDS (Mipmapped)", "DDS（含 Mipmap）" },
    { "JSON (Metadata)", "JSON（元数据）" },
    { "PNG (Atlas)", "PNG（图集）" },
    { "PNG (Textures)", "PNG（子纹理）" },
    { "DDS (Atlas)", "DDS（图集）" },
    { "DDS (Textures)", "DDS（子纹理）" },
    { "Metadata (JSON)", "元数据（JSON）" },
    { "PNG (High Quality)", "PNG（高质量）" },
    { "PNG (Low Quality)", "PNG（低质量）" },
    { "DDS (High Quality)", "DDS（高质量）" },
    { "DDS (Low Quality)", "DDS（低质量）" },
    { "Base (Textures)", "基础（纹理）" },
    { "Uber (Raw)", "Uber（原始数据）" },
    { "Uber (Struct)", "Uber（结构体）" },
    { "Raw", "原始数据" },
    { "MSW (Packed)", "MSW（打包）" },
    { "STL (Valve Physics)", "STL（Valve 物理）" },
    { "STL (Respawn Physics)", "STL（Respawn 物理）" },
    { "OBJ (Hitboxes only)", "OBJ（仅碰撞盒）" },

    // asset type display names (AssetTypeBinding_t::name, display only)
    { "Model", "模型" },
    { "Animation Rig", "动画骨架" },
    { "Animation Sequence", "动画序列" },
    { "Animation Sequence Data", "动画序列数据" },
    { "Animation Recording", "动画录制" },
    { "Source Model", "Source 模型" },
    { "Source Sequence", "Source 动画序列" },
    { "Material", "材质" },
    { "Material Snapshot", "材质快照" },
    { "Texture", "纹理" },
    { "Texture Animation", "纹理动画" },
    { "Texture List", "纹理列表" },
    { "UI Image Atlas", "UI 图集" },
    { "UI Image", "UI 图片" },
    { "UI Font", "UI 字体" },
    { "Particle Effect", "粒子特效" },
    { "Particle Script", "粒子脚本" },
    { "Shader", "着色器" },
    { "Shader Set", "着色器集" },
    { "RUI", "RUI" },
    { "LCD Screen Effect", "LCD 屏幕特效" },
    { "RTK UI", "RTK UI" },
    { "Patch Master", "补丁主控" },
    { "DataTable", "数据表" },
    { "Settings Layout", "设置布局" },
    { "RSON", "RSON" },
    { "Subtitles", "字幕" },
    { "Localization", "本地化" },
    { "On Demand Loaded Asset", "按需加载资产" },
    { "Wrapped File", "封装文件" },
    { "Weapon Definition", "武器定义" },
    { "Impact Definition", "冲击定义" },
    { "Map", "地图" },
    { "Audio Source", "音频源" },
    { "Bluepoint Wrapped File", "Bluepoint 封装文件" },
    { "Cube", "立方体" },

    // skin finder
    { "This menu contains a list of all registered cosmetic items associated with each game character.\nFor all features to work properly, RSX must load common.rpak, common_early.rpak, and localization_english.rpak",
      "此菜单列出了每个游戏角色所有已注册的外观物品。\n为保证所有功能正常工作，RSX 需要加载 common.rpak、common_early.rpak 和 localization_english.rpak" },
    { "Refresh data", "刷新数据" },
    { "Search", "搜索" },
    { "These skins can be found by opening file '%s':", "打开文件 '%s' 可找到以下皮肤：" },
    { "Load Assets", "加载资产" },
    { "Quality", "品质" },
    { "Arms Model", "手臂模型" },
    { "Body Model", "身体模型" },

    // logs window
    { "Time", "时间" },
    { "Level", "级别" },
    { "Message", "消息" },

    // progress bar events
    { "Loading Paks..", "正在加载 Pak.." },
    { "Loading Model Files..", "正在加载模型文件.." },
    { "Loading Audio Banks..", "正在加载音频库.." },
    { "Loading Bluepoint Pak Files..", "正在加载 Bluepoint Pak 文件.." },
    { "Exporting asset list...", "正在导出所选资产..." },
    { "Exporting all assets...", "正在导出全部资产..." },
    { "Processing Assets Post Load...", "正在进行资产加载后处理..." },
    { "Preparing Assets...", "正在准备资产..." },
    { "Processing Assets...", "正在处理资产..." },
    { "Exporting Fonts..", "正在导出字体.." },
    { "Exporting Sequences..", "正在导出动画序列.." },
    { "Exporting Materials..", "正在导出材质.." },
    { "Cancel", "取消" },

    // texture preview
    { "Texture Array Index:", "纹理数组索引：" },
    { "Mip Levels", "Mip 层级" },
    { "Level", "级别" },
    { "Dimensions", "尺寸" },
    { "Origin", "来源" },
    { "PERM", "常驻" },
    { "STREAMED", "流式" },
    { "OPT STREAMED", "可选流式" },
    { "Scale: %.f%%", "缩放：%.f%%" },
    { "Hold CTRL and scroll to zoom", "按住 CTRL 并滚动滚轮进行缩放" },
    { "Failed to preview this texture.", "无法预览此纹理。" },
    { "If this is a streamed texture, please check that all required .STARPAK files are present",
      "如果这是流式纹理，请检查所需的 .STARPAK 文件是否齐全" },

    // ui image / atlas / font preview
    { "Atlas", "图集" },
    { "Index", "序号" },
    { "Positions", "位置" },
    { "Render Size", "渲染尺寸" },
    { "Low Quality", "低质量" },
    { "High Quality", "高质量" },
    { "Width: %hu", "宽度：%hu" },
    { "Height: %hu", "高度：%hu" },
    { "Shifted Width: %hu", "对齐宽度：%hu" },
    { "Shifted Height: %hu", "对齐高度：%hu" },
    { "Actual Width: %hu", "实际宽度：%hu" },
    { "Actual Height: %hu", "实际高度：%hu" },
    { "Num BC1 Tiles: %u", "BC1 图块数：%u" },
    { "Num BC7 Tiles: %u", "BC7 图块数：%u" },
    { "Failed to preview selected UI Image. This asset does not contain any valid image data.\n",
      "无法预览所选 UI 图片。此资产不包含任何有效的图像数据。\n" },
    { "Font:", "字体：" },

    // material preview
    { "Type: %s", "类型：%s" },
    { "Shaderset: %s (0x%llx)", "着色器集：%s (0x%llx)" },
    { "Material Snapshot: %s (0x%llx)", "材质快照：%s (0x%llx)" },
    { "unloaded", "未加载" },
    { "If a material uses a snapshot, the snapshot needs to be loaded for DX States preview to be accurate.\n",
      "如果材质使用了快照，需要加载该快照才能准确预览 DX 状态。\n" },
    { "Textures", "纹理" },
    { "Properties", "属性" },
    { "DX States", "DX 状态" },
    { "The material asset does not provide any resource type information for this texture entry",
      "材质资产未提供此纹理条目的资源类型信息" },
    { "This type represents the way that the material is used and rendered by the game.\n\n"
      "Possible types:\n"
      " - RGDU: Static Props with regular vertices\n"
      " - RGDP: Static Props with packed vertex positions\n"
      " - RGDC: Static Props with packed vertex positions\n\n"
      " - SKNU: Skinned model with regular vertices\n"
      " - SKNP: Skinned model with packed vertex positions\n"
      " - SKNC: Skinned model with packed vertex positions\n\n"
      " - WLDU: World geometry with regular vertices\n"
      " - WLDC: World geometry with packed vertex positions\n\n"
      " - PTCU: Particles with regular vertices\n"
      " - PTCS: Particles\n\n"
      " - RGBS: Currently Unknown\n\n"
      "Titanfall types:\n"
      " - SKN: Skinned model\n"
      " - FIX: Skinned model fixup for use as a static prop\n"
      " - WLD: World geometry\n"
      " - GEN: UI, general use, particles, etc\n",
      "该类型表示游戏使用和渲染此材质的方式。\n\n"
      "可能的类型：\n"
      " - RGDU：常规顶点的静态道具\n"
      " - RGDP：压缩顶点位置的静态道具\n"
      " - RGDC：压缩顶点位置的静态道具\n\n"
      " - SKNU：常规顶点的蒙皮模型\n"
      " - SKNP：压缩顶点位置的蒙皮模型\n"
      " - SKNC：压缩顶点位置的蒙皮模型\n\n"
      " - WLDU：常规顶点的世界几何体\n"
      " - WLDC：压缩顶点位置的世界几何体\n\n"
      " - PTCU：常规顶点的粒子\n"
      " - PTCS：粒子\n\n"
      " - RGBS：目前未知\n\n"
      "Titanfall 类型：\n"
      " - SKN：蒙皮模型\n"
      " - FIX：作为静态道具使用的蒙皮模型修正\n"
      " - WLD：世界几何体\n"
      " - GEN：UI、通用、粒子等\n" },

    // model preview
    { "Sequences", "动画序列" },
    { "No sequences associated with this model.", "此模型没有关联的动画序列。" },
    { "Refresh", "刷新" },
    { "Sequence", "序列" },
    { "(base pose)", "（基础姿态）" },
    { "Name: %s\nMetadata: %.1f fps, %i frames, %.3f seconds", "名称：%s\n元数据：%.1f fps，%i 帧，%.3f 秒" },
    { "Unsupported delta anim; Preview unavailable", "不支持的增量动画；无法预览" },
    { "Invalid anim data; Preview unavailable", "无效的动画数据；无法预览" },
    { "Pause", "暂停" },
    { "Play", "播放" },
    { "Stop", "停止" },
    { "Loop", "循环" },
    { "Frame", "帧" },
    { "Sequence has no anims!", "该序列不包含动画！" },
    { "Bones: %llu", "骨骼数：%llu" },
    { "LODs: %llu", "LOD 数：%llu" },
    { "Local Sequences: %i", "本地序列数：%i" },
    { "LOD Level", "LOD 级别" },
    { "Skins:", "皮肤：" },
    { "Bodypart:", "身体部位：" },
    { "Model:", "模型：" },
    { "Name: %s", "名称：%s" },
    { "Flags: 0x%x", "标志：0x%x" },
    { "Frame Rate: %f", "帧率：%f" },
    { "Frame Count: %i", "帧数：%i" },
    { "Duration: %.3f seconds", "时长：%.3f 秒" },
    { "Label: %s", "标签：%s" },
    { "Activity: %s", "活动：%s" },
    { "Animations:", "动画：" },

    // audio preview
    { "Sample Rate: %u", "采样率：%u" },

    // log levels
    { "INFO", "信息" },
    { "WARNING", "警告" },
    { "ERROR", "错误" },

    // window titles (used through TRW)
    { "Asset List", "资产列表" },
    { "Asset Info", "资产信息" },
    { "Scene", "场景" },
    { "reSource Xtractor", "reSource Xtractor" },
};

static const std::unordered_map<std::string_view, const char*>* GetStringTable(const eUILanguage language)
{
    switch (language)
    {
    case eUILanguage::UILANG_CHINESE_SIMPLIFIED:
        return &s_StringsChineseSimplified;
    default:
        return nullptr;
    }
}

const char* TR(const char* const text)
{
    if (!text)
        return text;

    const std::unordered_map<std::string_view, const char*>* const table = GetStringTable(s_currentLanguage);
    if (!table)
        return text;

    const auto it = table->find(text);
    return it != table->end() ? it->second : text;
}

const char* TRW(const char* const windowName)
{
    if (s_currentLanguage == eUILanguage::UILANG_ENGLISH || !windowName)
        return windowName;

    // cache "<translated>###<english>" strings so the returned pointer stays stable and
    // no allocation happens on subsequent frames. only the UI thread calls this.
    static std::unordered_map<std::string, std::string> cachedTitles[static_cast<uint32_t>(eUILanguage::UILANG_COUNT)];

    std::unordered_map<std::string, std::string>& cache = cachedTitles[static_cast<uint32_t>(s_currentLanguage)];

    const auto it = cache.find(windowName);
    if (it != cache.end())
        return it->second.c_str();

    const auto emplaced = cache.emplace(windowName, std::format("{}###{}", TR(windowName), windowName));
    return emplaced.first->second.c_str();
}

bool TRCombo(const char* const label, int* const currentItem, const char* const* const items, const int itemCount)
{
    if (s_currentLanguage == eUILanguage::UILANG_ENGLISH)
        return ImGui::Combo(label, currentItem, items, itemCount);

    std::vector<const char*> translated(itemCount);
    for (int i = 0; i < itemCount; i++)
        translated[i] = TR(items[i]);

    return ImGui::Combo(label, currentItem, translated.data(), itemCount);
}
