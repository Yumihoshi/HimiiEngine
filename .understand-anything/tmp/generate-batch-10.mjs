import { readFileSync, writeFileSync } from 'fs';

const extractData = JSON.parse(readFileSync('D:/Himii-Engine/.understand-anything/tmp/ua-file-extract-results-10.json', 'utf-8'));
const inputData = JSON.parse(readFileSync('D:/Himii-Engine/.understand-anything/tmp/batch-input-10.json', 'utf-8'));

const batchImportData = inputData.batchImportData;

// ---- significance filter helpers ----
function isSignificant(fn) {
  const lines = fn.endLine - fn.startLine + 1;
  return lines >= 10;
}
function isExportedInFile(fileResult, name) {
  return fileResult.exports && fileResult.exports.some(e => e.name === name);
}
function isDestructor(name) { return name.startsWith('~'); }

const nodes = [];
const edges = [];

// ---- File nodes ----
for (const fr of extractData.results) {
  const fp = fr.path;
  const fileName = fp.split('/').pop();
  const nonEmpty = fr.nonEmptyLines;
  let complexity = nonEmpty < 50 ? 'simple' : (nonEmpty < 200 ? 'moderate' : 'complex');

  let summary = '', tags = [];

  if (fp.includes('/Asset/')) {
    if (fp.endsWith('Asset.h')) {
      summary = '定义 Asset 基类、AssetType 枚举以及类型与字符串互相转换的工具函数，为引擎资源系统提供抽象基类。';
      tags = ['data-model', '基类', '资源系统', '类型定义'];
    } else if (fp.endsWith('AssetManager.cpp')) {
      summary = '实现资产管理器的所有核心功能，包括资产导入、注册表序列化、精灵注册/解析、纹理元数据管理和图集网格切片。';
      tags = ['资源管理', '序列化', '精灵系统', '纹理管理'];
    } else if (fp.endsWith('AssetManager.h')) {
      summary = '声明资产管理器单例类及其数据结构，包括 SpriteRegistryEntry 和完整的资产管理公开 API。';
      tags = ['资源管理', '数据结构', 'api', '精灵系统'];
    } else if (fp.endsWith('AssetMetadata.h')) {
      summary = '定义 AssetMetadata 结构体，描述资源的运行时元数据（句柄、类型、文件路径、加载状态）。';
      tags = ['数据结构', '资源系统', '元数据'];
    } else if (fp.endsWith('AssetSerializer.cpp')) {
      summary = '实现粒子发射器、精灵动画、TileSet 和 TileMapData 四种资产的 YAML 序列化与反序列化逻辑。';
      tags = ['序列化', 'yaml', '资产管道', '精灵动画', 'tilemap'];
    } else if (fp.endsWith('AssetSerializer.h')) {
      summary = '声明四个资产序列化器类（ParticleEmitter、SpriteAnimation、TileSet、TileMapData），提供统一的 Serialize/Deserialize 接口。';
      tags = ['序列化', 'api', '资源系统', '精灵动画', 'tilemap'];
    } else if (fp.endsWith('PackFile.cpp')) {
      summary = '实现 PackFile 打包文件的读写功能，包括二进制文件头的解析、索引构建、条目读取和打包文件生成。';
      tags = ['文件系统', '二进制格式', '资源打包', 'i/o'];
    } else if (fp.endsWith('PackFile.h')) {
      summary = '声明 PackFileReader 和 PackFileWriter 两个类，提供打包文件的打开、读取和写入功能。';
      tags = ['文件系统', 'i/o', '资源打包', 'api'];
    } else if (fp.endsWith('PackFileFormat.h')) {
      summary = '定义 PackFile 二进制文件格式的数据结构（PackFileHeader 和 PackFileIndexEntry），包含魔数、版本号和索引布局。';
      tags = ['数据结构', '二进制格式', '资源打包', '文件格式'];
    } else if (fp.endsWith('Sprite.h')) {
      summary = '定义精灵系统相关的数据结构：SpriteDefinition（精灵定义）、TextureImportData（纹理导入数据）和 SpriteResolved（解析后的精灵）。';
      tags = ['数据结构', '精灵系统', '纹理', '渲染'];
    } else if (fp.endsWith('SpriteSheetUtility.cpp')) {
      summary = '实现精灵图集工具函数：UV 坐标计算（GPU/世界/ImGui 空间）、像素矩形转换、默认精灵创建、网格切片和渲染变换构建。';
      tags = ['工具函数', '精灵系统', 'uv-坐标', '渲染', '图集'];
    } else if (fp.endsWith('SpriteSheetUtility.h')) {
      summary = '声明 SpriteSheetUtility 静态工具类，提供 14 个精灵图集操作函数和 ImGuiImageUvCorners 辅助结构体。';
      tags = ['工具函数', '精灵系统', 'uv-坐标', 'api'];
    } else if (fp.endsWith('TextureImportSerializer.cpp')) {
      summary = '实现纹理导入元数据（.meta 文件）的 YAML 序列化/反序列化，包括精灵模式、像素矩形和精灵定义列表。';
      tags = ['序列化', '纹理', 'yaml', '资产管道', '元数据'];
    } else if (fp.endsWith('TextureImportSerializer.h')) {
      summary = '声明 TextureImportSerializer 类，提供纹理导入元数据的序列化接口（GetMetaPath、MetaExists、Serialize、Deserialize）。';
      tags = ['序列化', '纹理', 'api', '元数据'];
    }
  } else if (fp.includes('/Core/')) {
    if (fp.endsWith('Application.cpp')) {
      summary = '实现引擎 Application 类的核心生命周期：构造初始化（窗口/渲染器/脚本引擎）、主循环 Run()、事件分发和层管理。';
      tags = ['入口点', '应用框架', '生命周期', '事件系统'];
    } else if (fp.endsWith('EntryPoint.h')) {
      summary = '定义跨平台的 main() 入口点宏，初始化日志/性能分析、创建 Application 实例并启动主循环。';
      tags = ['入口点', 'main函数', '引导'];
    } else if (fp.endsWith('FileSystem.cpp')) {
      summary = '实现虚拟文件系统：支持松散文件和 PackFile 打包文件的统一读取、路径标准化、文件物化和缓存目录管理。';
      tags = ['文件系统', 'i/o', '资源打包', '虚拟文件系统'];
    } else if (fp.endsWith('FileSystem.h')) {
      summary = '声明 FileSystem 静态类，提供统一的文件读取（ReadBytes/ReadText）、存在性检查和材质化接口。';
      tags = ['文件系统', 'api', 'i/o'];
    } else if (fp.endsWith('FileWatcher.cpp')) {
      summary = '实现文件监视器：递归扫描目录、跟踪文件修改时间、防抖检测变化并触发回调。';
      tags = ['文件系统', '监视器', '热重载'];
    } else if (fp.endsWith('FileWatcher.h')) {
      summary = '声明 FileWatcher 类，提供目录监视、变化检测和防抖机制的 API。';
      tags = ['文件系统', 'api', '监视器'];
    } else if (fp.endsWith('Timer.h')) {
      summary = '定义 Timer 类（Header-Only），提供高精度计时、重置和毫秒级耗时查询功能。';
      tags = ['工具类', '计时器', '性能分析'];
    } else if (fp.endsWith('UUID.cpp')) {
      summary = '实现 UUID 类的构造函数，使用随机数引擎生成 64 位唯一标识符。';
      tags = ['工具类', '唯一标识符'];
    } else if (fp.endsWith('UUID.h')) {
      summary = '定义 UUID 类，提供 64 位全局唯一标识符的生成和比较功能。';
      tags = ['工具类', '数据结构', '唯一标识符'];
    }
  } else if (fp.includes('/Editor/')) {
    if (fp.endsWith('EditorSettings.cpp')) {
      summary = '实现编辑器设置的 YAML 序列化/反序列化，持久化默认脚本 IDE 类型和自定义路径等配置。';
      tags = ['编辑器', '配置', '序列化', 'yaml'];
    } else if (fp.endsWith('EditorSettings.h')) {
      summary = '声明 EditorSettings 类，定义编辑器持久化配置（默认脚本 IDE 类型）和 Load/Save 接口。';
      tags = ['编辑器', '配置', 'api'];
    }
  }

  nodes.push({
    id: `file:${fp}`,
    type: 'file',
    name: fileName,
    filePath: fp,
    summary,
    tags,
    complexity
  });
}

// ---- Function and Class nodes ----
for (const fr of extractData.results) {
  const fp = fr.path;

  // Function nodes
  for (const fn of (fr.functions || [])) {
    const lines = fn.endLine - fn.startLine + 1;
    // Destructors usually short, skip unless significant
    if (isDestructor(fn.name) && lines < 3) continue;
    if (!isSignificant(fn) && !isExportedInFile(fr, fn.name)) continue;
    if (lines < 3 && !isExportedInFile(fr, fn.name)) continue;

    // Skip trivial constructors/destructors (< 4 lines)
    if (lines < 4 && (fn.name === fr.path.split('/').pop().replace(/\.[^.]+$/, '') || isDestructor(fn.name))) continue;
    // Skip trivial helper/inline functions (< 8 lines) unless exported
    if (lines < 8 && !isExportedInFile(fr, fn.name)) continue;

    let summary = '', tags = [];
    const shortName = fn.name;

    if (fp.includes('Asset.h')) {
      if (shortName === 'AssetTypeToString') { summary = '将 AssetType 枚举值转换为对应的字符串名称。'; tags = ['类型转换', '工具函数']; }
      else if (shortName === 'AssetTypeFromString') { summary = '根据字符串名称查找并返回对应的 AssetType 枚举值。'; tags = ['类型转换', '工具函数']; }
    } else if (fp.includes('AssetManager.cpp')) {
      if (shortName === 'GetAsset') { summary = '根据资源句柄加载或获取已缓存的资源对象，支持多种资源类型的反序列化和默认元数据生成。'; tags = ['资源管理', '缓存', '加载']; }
      else if (shortName === 'ImportAsset') { summary = '将文件路径导入为引擎资源，自动识别资源类型并注册到全局注册表。'; tags = ['资源管理', '导入']; }
      else if (shortName === 'GetAssetTypeFromExtension') { summary = '根据文件扩展名（不区分大小写）匹配对应的 AssetType 枚举值。'; tags = ['类型判断', '工具函数']; }
      else if (shortName === 'SerializeAssetRegistry') { summary = '将当前资产注册表序列化为 YAML 文件，持久化所有已导入资源的元数据。'; tags = ['序列化', '注册表']; }
      else if (shortName === 'DeserializeAssetRegistry') { summary = '从 YAML 文件反序列化资产注册表，恢复上次会话导入的所有资源信息。'; tags = ['序列化', '注册表']; }
      else if (shortName === 'RegisterSpritesFromImportData') { summary = '从纹理导入数据中将精灵定义注册到全局精灵注册表。'; tags = ['精灵系统', '注册']; }
      else if (shortName === 'UnregisterSpritesForTexture') { summary = '注销指定纹理关联的所有精灵条目。'; tags = ['精灵系统', '注销']; }
      else if (shortName === 'LoadTextureImportData') { summary = '加载指定纹理的导入元数据（精灵模式、切片设置等）并注册精灵。'; tags = ['纹理管理', '加载']; }
      else if (shortName === 'SaveTextureImportData') { summary = '保存指定纹理的导入元数据到 .meta 文件并更新精灵注册表。'; tags = ['纹理管理', '保存']; }
      else if (shortName === 'EnsureDefaultTextureMeta') { summary = '确保纹理存在默认导入元数据，若缺失则根据纹理尺寸自动创建单精灵定义。'; tags = ['纹理管理', '默认值']; }
      else if (shortName === 'ApplyGridSliceToTexture') { summary = '对纹理按网格参数切片生成多个精灵定义并保存导入数据。'; tags = ['精灵系统', '网格切片']; }
      else if (shortName === 'ResolveSprite') { summary = '解析精灵句柄，返回对应的 SpriteResolved 结构（含纹理指针和 UV 坐标）。'; tags = ['精灵系统', '解析']; }
      else if (shortName === 'ResolveSpriteFromTexture') { summary = '根据纹理句柄和精灵索引解析精灵数据，计算 UV 坐标和像素尺寸。'; tags = ['精灵系统', '解析', 'uv']; }
      else if (shortName === 'GetDefaultSpriteHandleForTexture') { summary = '获取纹理默认精灵（索引 0）的资源句柄。'; tags = ['精灵系统', '查询']; }
      else if (shortName === 'GetTextureHandleForSprite') { summary = '根据精灵资源句柄反向查找其所属的纹理句柄。'; tags = ['精灵系统', '查询']; }
    } else if (fp.includes('AssetSerializer.cpp')) {
      if (shortName === 'Serialize' && fn.startLine === 35) { summary = '将 ParticleEmitterAsset 序列化为 YAML 文件。'; tags = ['序列化', '粒子系统']; }
      else if (shortName === 'Deserialize' && fn.startLine === 64) { summary = '从 YAML 文件反序列化 ParticleEmitterAsset。'; tags = ['序列化', '粒子系统']; }
      else if (shortName === 'WriteSpriteAnimationClip') { summary = '将精灵动画片段的帧数据和循环模式写入 YAML 节点。'; tags = ['序列化', '精灵动画']; }
      else if (shortName === 'ReadSpriteAnimationClipAtlasCoordinates') { summary = '从 YAML 节点读取图集坐标帧数据。'; tags = ['序列化', '精灵动画']; }
      else if (shortName === 'ReadSpriteAnimationClipFrames') { summary = '从 YAML 节点读取独立帧数据（非图集模式）。'; tags = ['序列化', '精灵动画']; }
      else if (shortName === 'Serialize' && fn.startLine === 164) { summary = '将 SpriteAnimation 序列化为 YAML 文件，支持图集和独立帧两种模式。'; tags = ['序列化', '精灵动画']; }
      else if (shortName === 'Deserialize' && fn.startLine === 189) { summary = '从 YAML 文件反序列化 SpriteAnimation，支持新版多动画和旧版单动画格式迁移。'; tags = ['序列化', '精灵动画', '格式迁移']; }
      else if (shortName === 'Serialize' && fn.startLine === 302) { summary = '将 TileSet 序列化为 YAML 文件，包含图集源和 Tile 定义列表。'; tags = ['序列化', 'tilemap']; }
      else if (shortName === 'Deserialize' && fn.startLine === 352) { summary = '从 YAML 文件反序列化 TileSet，恢复图集源引用和 Tile 属性。'; tags = ['序列化', 'tilemap']; }
      else if (shortName === 'Serialize' && fn.startLine === 437) { summary = '将 TileMapData 序列化为 YAML 文件，包含 TileSet 引用、网格尺寸和 Chunk 数据。'; tags = ['序列化', 'tilemap']; }
      else if (shortName === 'Deserialize' && fn.startLine === 478) { summary = '从 YAML 文件反序列化 TileMapData，支持新旧格式迁移（稠密 Tile 数组到稀疏 Chunk）。'; tags = ['序列化', 'tilemap', '格式迁移']; }
    } else if (fp.includes('PackFile.cpp')) {
      if (shortName === 'Open') { summary = '打开并解析 PackFile 二进制文件，验证魔数和版本，构建内存索引映射。'; tags = ['文件系统', '解析']; }
      else if (shortName === 'ReadEntry') { summary = '根据相对路径从 PackFile 中读取二进制条目数据。'; tags = ['文件系统', '读取']; }
      else if (shortName === 'Write') { summary = '将条目列表打包写入 PackFile 二进制文件，生成文件头、索引区和数据区。'; tags = ['文件系统', '写入', '打包']; }
    } else if (fp.includes('SpriteSheetUtility.cpp')) {
      if (shortName === 'ComputeGpuUvEdgesFromPixelRect') { summary = '根据像素矩形和纹理尺寸计算四个 GPU UV 边界值。'; tags = ['uv-坐标', 'gpu']; }
      else if (shortName === 'PixelRectToImGuiQuadUVsForScreenCorners') { summary = '将像素矩形转换为 ImGui Image 控件所需的四个角 UV 坐标。'; tags = ['uv-坐标', 'imgui']; }
      else if (shortName === 'PixelRectToWorldQuadUVs') { summary = '将像素矩形转换为世界空间四边形渲染所需的纹理坐标，处理左上角为原点的坐标系。'; tags = ['uv-坐标', '渲染']; }
      else if (shortName === 'PixelRectToImGuiImageUVCorners') { summary = '将像素矩形转换为 ImGui 图像的四个角 UV 坐标（TopLeft + BottomRight）。'; tags = ['uv-坐标', 'imgui']; }
      else if (shortName === 'CreateDefaultSingleSprite') { summary = '为纹理创建默认完整图精灵定义（覆盖整张纹理）。'; tags = ['精灵系统', '默认值']; }
      else if (shortName === 'ApplyGridSlice') { summary = '按网格参数对导入数据进行切片，生成命名的精灵定义列表。'; tags = ['精灵系统', '网格切片']; }
      else if (shortName === 'BuildSpriteRenderTransform') { summary = '根据实体变换、像素尺寸和锚点构建精灵渲染的世界矩阵。'; tags = ['渲染', '变换', '矩阵']; }
      else if (shortName === 'ComputeSpriteVisualTransform') { summary = '计算精灵最终视觉变换（结合 BuildSpriteRenderTransform 和负缩放校正）。'; tags = ['渲染', '变换']; }
      else if (shortName === 'CorrectTransformNegativeScaleForSprite') { summary = '处理精灵在负缩放情况下的变换校正，提取缩放绝对值和旋转角度并重建矩阵。'; tags = ['渲染', '变换', '校正']; }
      else if (shortName === 'ApplySpriteUvFacing') { summary = '根据校正标志水平或垂直翻转精灵 UV 坐标。'; tags = ['uv-坐标', '渲染', '翻转']; }
      else if (shortName === 'AtlasGridCoordsToPixelRect') { summary = '将图集网格坐标转换为像素矩形。'; tags = ['图集', '坐标转换']; }
      else if (shortName === 'PixelRectToImGuiImageUv') { summary = '将像素矩形转换为 ImGui Image 的标准 UV 格式。'; tags = ['uv-坐标', 'imgui']; }
      else if (shortName === 'AtlasGridCoordsToWorldQuadUVs') { summary = '将图集网格坐标通过像素矩形转换为世界空间 UV。'; tags = ['图集', 'uv-坐标']; }
    } else if (fp.includes('TextureImportSerializer.cpp')) {
      if (shortName === 'Deserialize') { summary = '从 .meta YAML 文件反序列化纹理导入数据，包括精灵模式、网格参数和精灵定义列表。'; tags = ['序列化', '纹理']; }
      else if (shortName === 'Serialize') { summary = '将纹理导入数据序列化为 .meta YAML 文件，保存精灵定义和切片参数。'; tags = ['序列化', '纹理']; }
      else if (shortName === 'ReadPixelRect') { summary = '从 YAML 节点读取像素矩形（x, y, w, h）。'; tags = ['序列化', '解析']; }
    } else if (fp.includes('Application.cpp')) {
      if (shortName === 'Application') { summary = 'Application 构造函数：初始化窗口、渲染器和脚本引擎，设置事件回调和层堆栈。'; tags = ['构造', '初始化']; }
      else if (shortName === 'OnEvent') { summary = '全局事件分发：先传层堆栈（反向），再调用窗口关闭/大小改变的特化处理。'; tags = ['事件系统', '分发']; }
      else if (shortName === 'SetEnvironmentVariables') { summary = '初始化应用运行环境：设置工作目录、动态库搜索路径、引擎资源根路径和 MONO_PATH。'; tags = ['初始化', '环境配置']; }
      else if (shortName === 'OnWindowResize') { summary = '窗口大小变化回调：当尺寸非零时通知渲染器调整视口。'; tags = ['事件处理', '窗口']; }
      else if (shortName === 'Run') { summary = '主循环：测量帧时间、更新层状态、渲染 ImGui 并同步窗口缓冲区。'; tags = ['主循环', '帧更新']; }
    } else if (fp.includes('EntryPoint.h')) {
      if (shortName === 'main') { summary = '引擎的 main() 入口点：初始化日志和性能分析、创建 Application 并进入 Run 主循环。'; tags = ['入口点', 'main函数']; }
    } else if (fp.includes('FileSystem.cpp')) {
      if (shortName === 'Init') { summary = '初始化虚拟文件系统：打开 PackFile 打包文件，设置根目录和缓存路径。'; tags = ['初始化', '文件系统']; }
      else if (shortName === 'Shutdown') { summary = '关闭虚拟文件系统，释放 PackFile 资源和缓存信息。'; tags = ['清理', '文件系统']; }
      else if (shortName === 'ReadLooseBytes') { summary = '从松散文件系统读取指定路径的二进制数据。'; tags = ['文件系统', '读取']; }
      else if (shortName === 'Exists') { summary = '检查文件是否存在于松散目录或 PackFile 打包文件中。'; tags = ['文件系统', '查询']; }
      else if (shortName === 'ReadBytes') { summary = '统一读取接口：优先从松散文件读取，回退到 PackFile，最后尝试材质化。'; tags = ['文件系统', '读取']; }
      else if (shortName === 'ReadText') { summary = '以文本模式读取文件内容，返回 std::string。'; tags = ['文件系统', '读取']; }
      else if (shortName === 'MaterializeLooseFile') { summary = '从 PackFile 中提取文件到临时目录，用于需要真实文件路径的场景。'; tags = ['文件系统', '材质化']; }
    } else if (fp.includes('FileWatcher.cpp')) {
      if (shortName === 'Watch') { summary = '启动目录监视：设置监视路径、变更回调和防抖间隔，并执行首次全量扫描。'; tags = ['监视器', '初始化']; }
      else if (shortName === 'Clear') { summary = '清空所有监视数据和文件时间戳记录。'; tags = ['监视器', '清理']; }
      else if (shortName === 'Update') { summary = '周期性检查文件变化：防抖计数、触发回调、定期增量扫描。'; tags = ['监视器', '轮询']; }
      else if (shortName === 'ScanDirectory') { summary = '递归扫描监视目录，对比文件修改时间戳以检测新增或修改的文件。'; tags = ['监视器', '扫描']; }
    } else if (fp.includes('EditorSettings.cpp')) {
      if (shortName === 'Load') { summary = '从 YAML 配置文件加载编辑器设置（脚本 IDE 类型和自定义路径）。'; tags = ['配置', '加载']; }
      else if (shortName === 'Save') { summary = '将当前编辑器设置持久化为 YAML 配置文件。'; tags = ['配置', '保存']; }
    }
    // Fallback for unmatched functions
    if (!summary) { summary = `${shortName} 函数。`; tags = ['函数']; }

    nodes.push({
      id: `function:${fp}:${shortName}`,
      type: 'function',
      name: shortName,
      filePath: fp,
      lineRange: [fn.startLine, fn.endLine],
      summary,
      tags,
      complexity: lines < 15 ? 'simple' : (lines < 40 ? 'moderate' : 'complex')
    });
  }

  // Class nodes
  for (const cls of (fr.classes || [])) {
    const lines = cls.endLine - cls.startLine + 1;
    const methodCount = (cls.methods || []).length;
    if (methodCount < 2 && lines < 20) continue;

    let summary = '', tags = [];

    if (fp.includes('Asset.h') && cls.name === 'Asset') { summary = '引擎资源的抽象基类，定义资源类型枚举、Handle 属性和类型转换静态方法。'; tags = ['基类', '数据模型', '资源系统']; }
    else if (fp.includes('AssetManager.h') && cls.name === 'SpriteRegistryEntry') { summary = '精灵注册表条目，记录纹理句柄和精灵索引的映射关系。'; tags = ['数据结构', '精灵系统']; }
    else if (fp.includes('AssetManager.h') && cls.name === 'AssetManager') { summary = '资产管理器单例类，管理资产注册表、加载/缓存、纹理元数据和精灵注册/解析。'; tags = ['单例', '资源管理', '精灵系统']; }
    else if (fp.includes('AssetMetadata.h') && cls.name === 'AssetMetadata') { summary = '资产的运行时元数据结构体，包含句柄、类型、文件路径和加载状态。'; tags = ['数据结构', '元数据']; }
    else if (fp.includes('AssetSerializer.h') && cls.name === 'ParticleEmitterAssetSerializer') { summary = '粒子发射器资产的 YAML 序列化器。'; tags = ['序列化', '粒子系统']; }
    else if (fp.includes('AssetSerializer.h') && cls.name === 'SpriteAnimationSerializer') { summary = '精灵动画资产的 YAML 序列化器，支持图集模式和独立帧模式。'; tags = ['序列化', '精灵动画']; }
    else if (fp.includes('AssetSerializer.h') && cls.name === 'TileSetSerializer') { summary = 'TileSet 资产的 YAML 序列化器。'; tags = ['序列化', 'tilemap']; }
    else if (fp.includes('AssetSerializer.h') && cls.name === 'TileMapDataSerializer') { summary = 'TileMapData 资产的 YAML 序列化器，支持新旧格式迁移。'; tags = ['序列化', 'tilemap']; }
    else if (fp.includes('PackFile.h') && cls.name === 'PackFileReader') { summary = 'PackFile 二进制打包文件的读取器，支持打开、验证、索引构建和按路径读取条目。'; tags = ['文件系统', '读取器', '二进制格式']; }
    else if (fp.includes('PackFile.h') && cls.name === 'PackFileWriter') { summary = 'PackFile 二进制打包文件的写入器，支持条目排序、文件头和索引区生成。'; tags = ['文件系统', '写入器', '二进制格式']; }
    else if (fp.includes('PackFileFormat.h') && cls.name === 'PackFileHeader') { summary = 'PackFile 文件头结构体，定义魔数、版本、条目数和偏移量。'; tags = ['数据结构', '二进制格式', '文件头']; }
    else if (fp.includes('PackFileFormat.h') && cls.name === 'PackFileIndexEntry') { summary = 'PackFile 索引条目结构体，描述每个打包文件路径的长度。'; tags = ['数据结构', '二进制格式']; }
    else if (fp.includes('Sprite.h') && cls.name === 'SpriteDefinition') { summary = '精灵定义结构体，包含句柄、名称、像素矩形、锚点和边框信息。'; tags = ['数据结构', '精灵系统']; }
    else if (fp.includes('Sprite.h') && cls.name === 'TextureImportData') { summary = '纹理导入数据结构体，包含精灵模式、网格切片参数和精灵定义列表。'; tags = ['数据结构', '纹理', '精灵系统']; }
    else if (fp.includes('Sprite.h') && cls.name === 'SpriteResolved') { summary = '解析后的精灵结构体，包含纹理指针、UV 坐标、锚点、像素尺寸和有效标志。'; tags = ['数据结构', '精灵系统', '渲染']; }
    else if (fp.includes('SpriteSheetUtility.h') && cls.name === 'ImGuiImageUvCorners') { summary = 'ImGui 图像 UV 角点结构体，含 TopLeft 和 BottomRight。'; tags = ['数据结构', 'uv-坐标', 'imgui']; }
    else if (fp.includes('SpriteSheetUtility.h') && cls.name === 'SpriteSheetUtility') { summary = '精灵图集静态工具类，提供 14 个 UV 计算、精灵创建、网格切片和变换构建函数。'; tags = ['工具类', '精灵系统', 'uv-坐标']; }
    else if (fp.includes('TextureImportSerializer.h') && cls.name === 'TextureImportSerializer') { summary = '纹理导入序列化器，提供 .meta 文件路径生成、存在检查和序列化/反序列化功能。'; tags = ['序列化', '纹理', '元数据']; }
    else if (fp.includes('FileSystem.h') && cls.name === 'FileSystem') { summary = '虚拟文件系统静态类，统一管理松散文件和 PackFile 的读取、存在检查和材质化。'; tags = ['文件系统', '工具类', 'i/o']; }
    else if (fp.includes('FileWatcher.h') && cls.name === 'FileWatcher') { summary = '文件监视器类，提供递归目录扫描、文件修改时间戳跟踪和防抖变化通知。'; tags = ['监视器', '数据结构']; }
    else if (fp.includes('Timer.h') && cls.name === 'Timer') { summary = '高精度计时器类（Header-Only），封装 std::chrono 提供秒级和毫秒级耗时测量。'; tags = ['工具类', '计时器']; }
    else if (fp.includes('UUID.h') && cls.name === 'UUID') { summary = '64 位全局唯一标识符类，支持随机生成和从 uint64_t 构造。'; tags = ['数据结构', '唯一标识符']; }
    else if (fp.includes('EditorSettings.h') && cls.name === 'EditorSettings') { summary = '编辑器持久化设置类，管理默认脚本 IDE 类型和自定义路径，支持 YAML Load/Save。'; tags = ['编辑器', '配置']; }
    // Fallback for unmatched classes
    if (!summary) { summary = `${cls.name} 类。`; tags = ['类']; }

    nodes.push({
      id: `class:${fp}:${cls.name}`,
      type: 'class',
      name: cls.name,
      filePath: fp,
      lineRange: [cls.startLine, cls.endLine],
      summary,
      tags,
      complexity: lines < 30 ? 'simple' : (lines < 80 ? 'moderate' : 'complex')
    });
  }
}

// ---- EDGES ----

// 1. imports edges (from batchImportData)
for (const [srcPath, imports] of Object.entries(batchImportData)) {
  for (const importPath of imports) {
    edges.push({
      source: `file:${srcPath}`,
      target: `file:${importPath}`,
      type: 'imports',
      direction: 'forward',
      weight: 0.7
    });
  }
}

// 2. contains edges
for (const node of nodes) {
  if (node.type === 'function' || node.type === 'class') {
    edges.push({
      source: `file:${node.filePath}`,
      target: node.id,
      type: 'contains',
      direction: 'forward',
      weight: 1.0
    });
  }
}

// 3. exports edges (for key public API items)
for (const node of nodes) {
  if (node.type !== 'function' && node.type !== 'class') continue;
  // Check if this item is in the exported list of its file
  const fr = extractData.results.find(r => r.path === node.filePath);
  if (!fr) continue;
  if (fr.exports && fr.exports.some(e => e.name === node.name)) {
    edges.push({
      source: `file:${node.filePath}`,
      target: node.id,
      type: 'exports',
      direction: 'forward',
      weight: 0.8
    });
  }
}

// 4. calls edges (select important cross-function calls that were detected)
const importantCalls = [
  // AssetManager internal calls
  { src: 'function:Engine/src/Himii/Asset/AssetManager.cpp:GetAsset', tgt: 'function:Engine/src/Himii/Asset/AssetManager.cpp:EnsureDefaultTextureMeta' },
  { src: 'function:Engine/src/Himii/Asset/AssetManager.cpp:ImportAsset', tgt: 'function:Engine/src/Himii/Asset/AssetManager.cpp:GetAssetTypeFromExtension' },
  { src: 'function:Engine/src/Himii/Asset/AssetManager.cpp:DeserializeAssetRegistry', tgt: 'function:Engine/src/Himii/Asset/AssetManager.cpp:LoadTextureImportData' },
  { src: 'function:Engine/src/Himii/Asset/AssetManager.cpp:LoadTextureImportData', tgt: 'function:Engine/src/Himii/Asset/AssetManager.cpp:RegisterSpritesFromImportData' },
  { src: 'function:Engine/src/Himii/Asset/AssetManager.cpp:SaveTextureImportData', tgt: 'function:Engine/src/Himii/Asset/AssetManager.cpp:RegisterSpritesFromImportData' },
  { src: 'function:Engine/src/Himii/Asset/AssetManager.cpp:EnsureDefaultTextureMeta', tgt: 'function:Engine/src/Himii/Asset/AssetManager.cpp:LoadTextureImportData' },
  { src: 'function:Engine/src/Himii/Asset/AssetManager.cpp:ApplyGridSliceToTexture', tgt: 'function:Engine/src/Himii/Asset/AssetManager.cpp:SaveTextureImportData' },
  { src: 'function:Engine/src/Himii/Asset/AssetManager.cpp:ResolveSprite', tgt: 'function:Engine/src/Himii/Asset/AssetManager.cpp:ResolveSpriteFromTexture' },
  { src: 'function:Engine/src/Himii/Asset/AssetManager.cpp:ResolveSpriteFromTexture', tgt: 'function:Engine/src/Himii/Asset/AssetManager.cpp:EnsureDefaultTextureMeta' },
  { src: 'function:Engine/src/Himii/Asset/AssetManager.cpp:GetDefaultSpriteHandleForTexture', tgt: 'function:Engine/src/Himii/Asset/AssetManager.cpp:EnsureDefaultTextureMeta' },
  { src: 'function:Engine/src/Himii/Asset/AssetManager.cpp:EnsureDefaultTextureMeta', tgt: 'function:Engine/src/Himii/Asset/SpriteSheetUtility.cpp:CreateDefaultSingleSprite' },
  { src: 'function:Engine/src/Himii/Asset/AssetManager.cpp:ResolveSpriteFromTexture', tgt: 'function:Engine/src/Himii/Asset/SpriteSheetUtility.cpp:PixelRectToWorldQuadUVs' },
  { src: 'function:Engine/src/Himii/Asset/AssetManager.cpp:ApplyGridSliceToTexture', tgt: 'function:Engine/src/Himii/Asset/SpriteSheetUtility.cpp:ApplyGridSlice' },
  // PackFile calls
  { src: 'function:Engine/src/Himii/Asset/PackFile.cpp:Open', tgt: 'function:Engine/src/Himii/Asset/PackFile.cpp:Close' },
  { src: 'function:Engine/src/Himii/Asset/PackFile.cpp:HasEntry', tgt: 'function:Engine/src/Himii/Asset/PackFile.cpp:NormalizeRelativePath' },
  { src: 'function:Engine/src/Himii/Asset/PackFile.cpp:ReadEntry', tgt: 'function:Engine/src/Himii/Asset/PackFile.cpp:NormalizeRelativePath' },
  { src: 'function:Engine/src/Himii/Asset/PackFile.cpp:Write', tgt: 'function:Engine/src/Himii/Asset/PackFile.cpp:NormalizeRelativePath' },
  // FileSystem calls PackFile
  { src: 'function:Engine/src/Himii/Core/FileSystem.cpp:Shutdown', tgt: 'function:Engine/src/Himii/Asset/PackFile.cpp:Close' },
  { src: 'function:Engine/src/Himii/Core/FileSystem.cpp:Init', tgt: 'function:Engine/src/Himii/Asset/PackFile.cpp:Open' },
  { src: 'function:Engine/src/Himii/Core/FileSystem.cpp:Exists', tgt: 'function:Engine/src/Himii/Asset/PackFile.cpp:HasEntry' },
  { src: 'function:Engine/src/Himii/Core/FileSystem.cpp:ReadBytes', tgt: 'function:Engine/src/Himii/Asset/PackFile.cpp:ReadEntry' },
  { src: 'function:Engine/src/Himii/Core/FileSystem.cpp:ReadBytes', tgt: 'function:Engine/src/Himii/Core/FileSystem.cpp:ReadLooseBytes' },
  { src: 'function:Engine/src/Himii/Core/FileSystem.cpp:ReadText', tgt: 'function:Engine/src/Himii/Core/FileSystem.cpp:ReadBytes' },
  { src: 'function:Engine/src/Himii/Core/FileSystem.cpp:MaterializeLooseFile', tgt: 'function:Engine/src/Himii/Core/FileSystem.cpp:ReadBytes' },
  { src: 'function:Engine/src/Himii/Core/FileSystem.cpp:Exists', tgt: 'function:Engine/src/Himii/Core/FileSystem.cpp:NormalizeRelativePath' },
  { src: 'function:Engine/src/Himii/Core/FileSystem.cpp:ReadBytes', tgt: 'function:Engine/src/Himii/Core/FileSystem.cpp:NormalizeRelativePath' },
  // Application calls
  { src: 'function:Engine/src/Himii/Core/Application.cpp:Application', tgt: 'function:Engine/src/Himii/Core/Application.cpp:SetEnvironmentVariables' },
  { src: 'function:Engine/src/Himii/Core/FileSystem.cpp:Init', tgt: 'function:Engine/src/Himii/Core/FileSystem.cpp:Shutdown' },
  // FileWatcher calls
  { src: 'function:Engine/src/Himii/Core/FileWatcher.cpp:Watch', tgt: 'function:Engine/src/Himii/Core/FileWatcher.cpp:ScanDirectory' },
  { src: 'function:Engine/src/Himii/Core/FileWatcher.cpp:Update', tgt: 'function:Engine/src/Himii/Core/FileWatcher.cpp:ScanDirectory' },
  // EditorSettings calls
  { src: 'function:Engine/src/Himii/Editor/EditorSettings.cpp:Load', tgt: 'function:Engine/src/Himii/Editor/EditorSettings.cpp:GetSettingsPath' },
  { src: 'function:Engine/src/Himii/Editor/EditorSettings.cpp:Save', tgt: 'function:Engine/src/Himii/Editor/EditorSettings.cpp:GetSettingsPath' },
  // SpriteSheetUtility internal calls
  { src: 'function:Engine/src/Himii/Asset/SpriteSheetUtility.cpp:PixelRectToWorldQuadUVs', tgt: 'function:Engine/src/Himii/Asset/SpriteSheetUtility.cpp:ComputeGpuUvEdgesFromPixelRect' },
  { src: 'function:Engine/src/Himii/Asset/SpriteSheetUtility.cpp:PixelRectToImGuiImageUVCorners', tgt: 'function:Engine/src/Himii/Asset/SpriteSheetUtility.cpp:ComputeGpuUvEdgesFromPixelRect' },
  { src: 'function:Engine/src/Himii/Asset/SpriteSheetUtility.cpp:ComputeSpriteVisualTransform', tgt: 'function:Engine/src/Himii/Asset/SpriteSheetUtility.cpp:BuildSpriteRenderTransform' },
  { src: 'function:Engine/src/Himii/Asset/SpriteSheetUtility.cpp:AtlasGridCoordsToWorldQuadUVs', tgt: 'function:Engine/src/Himii/Asset/SpriteSheetUtility.cpp:PixelRectToWorldQuadUVs' },
  { src: 'function:Engine/src/Himii/Asset/SpriteSheetUtility.cpp:AtlasGridCoordsToWorldQuadUVs', tgt: 'function:Engine/src/Himii/Asset/SpriteSheetUtility.cpp:AtlasGridCoordsToPixelRect' },
  { src: 'function:Engine/src/Himii/Asset/SpriteSheetUtility.cpp:PixelRectToImGuiQuadUVsForScreenCorners', tgt: 'function:Engine/src/Himii/Asset/SpriteSheetUtility.cpp:PixelRectToImGuiImageUv' },
  { src: 'function:Engine/src/Himii/Asset/SpriteSheetUtility.cpp:PixelRectToUVs', tgt: 'function:Engine/src/Himii/Asset/SpriteSheetUtility.cpp:PixelRectToWorldQuadUVs' },
  // TextureImportSerializer internal calls
  { src: 'function:Engine/src/Himii/Asset/TextureImportSerializer.cpp:Deserialize', tgt: 'function:Engine/src/Himii/Asset/TextureImportSerializer.cpp:ReadPixelRect' },
  { src: 'function:Engine/src/Himii/Asset/TextureImportSerializer.cpp:Serialize', tgt: 'function:Engine/src/Himii/Asset/TextureImportSerializer.cpp:GetMetaPath' },
];

// Build a set of all node ids for validation
const nodeIds = new Set(nodes.map(n => n.id));

for (const call of importantCalls) {
  // Only emit if both nodes exist
  if (nodeIds.has(call.src) && nodeIds.has(call.tgt)) {
    edges.push({
      source: call.src,
      target: call.tgt,
      type: 'calls',
      direction: 'forward',
      weight: 0.8
    });
  }
}

// ---- Log stats ----
console.log(`Total nodes: ${nodes.length}`);
console.log(`Total edges: ${edges.length}`);
console.log(`File nodes: ${nodes.filter(n => n.type === 'file').length}`);
console.log(`Function nodes: ${nodes.filter(n => n.type === 'function').length}`);
console.log(`Class nodes: ${nodes.filter(n => n.type === 'class').length}`);
console.log(`Import edges: ${edges.filter(e => e.type === 'imports').length}`);
console.log(`Contains edges: ${edges.filter(e => e.type === 'contains').length}`);
console.log(`Exports edges: ${edges.filter(e => e.type === 'exports').length}`);
console.log(`Calls edges: ${edges.filter(e => e.type === 'calls').length}`);

// ---- Split into parts ----
const totalNodes = nodes.length;
const totalEdges = edges.length;
const needsSplit = totalNodes > 60 || totalEdges > 120;

if (!needsSplit) {
  writeFileSync('D:/Himii-Engine/.understand-anything/intermediate/batch-10.json', JSON.stringify({ nodes, edges }, null, 2));
  console.log('Wrote single file: batch-10.json');
} else {
  const parts = Math.ceil(Math.max(totalNodes / 60, totalEdges / 120));
  console.log(`Splitting into ${parts} parts`);

  // Sort files alphabetically
  const filePaths = [...new Set(nodes.map(n => n.filePath).filter(Boolean))].sort();
  const filesPerPart = Math.ceil(filePaths.length / parts);

  for (let k = 0; k < parts; k++) {
    const partFilePaths = new Set(filePaths.slice(k * filesPerPart, (k + 1) * filesPerPart));

    const partNodes = nodes.filter(n => {
      if (!n.filePath) return false;
      return partFilePaths.has(n.filePath);
    });

    const partNodeIds = new Set(partNodes.map(n => n.id));

    const partEdges = edges.filter(e => {
      return partNodeIds.has(e.source);
    });

    const partNum = k + 1;
    const filename = `batch-10-part-${partNum}.json`;
    writeFileSync(`D:/Himii-Engine/.understand-anything/intermediate/${filename}`, JSON.stringify({ nodes: partNodes, edges: partEdges }, null, 2));
    console.log(`Wrote ${filename}: ${partNodes.length} nodes, ${partEdges.length} edges`);
  }
}
