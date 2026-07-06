const fs = require('fs');

const extraction = JSON.parse(fs.readFileSync('D:/Himii-Engine/.understand-anything/tmp/ua-file-extract-results-2.json', 'utf8'));
const input = JSON.parse(fs.readFileSync('D:/Himii-Engine/.understand-anything/tmp/batch-input-2.json', 'utf8'));

const results = extraction.results;
const batchImportData = input.batchImportData;
const neighborMap = input.neighborMap;

function complexity(lines, funcCount) {
  if (lines > 200 || funcCount > 10) return 'complex';
  if (lines > 50 || funcCount > 3) return 'moderate';
  return 'simple';
}

let allNodes = [];
let allEdges = [];

// ============== FILE NODES ==============

const fileDecls = {
  'HimiiEditor/src/EditorLayer.h': {
    summary: '编辑器核心层主类头文件，管理编辑器生命周期、所有子面板的创建与协调、场景编辑操作、Tilemap 绘制系统以及启动画面过渡流程。',
    tags: ['entry-point', 'coordinator', 'panel-manager', 'tilemap']
  },
  'HimiiEditor/src/HimiiApp.cpp': {
    summary: 'Himii 编辑器应用程序入口实现，负责创建应用实例、配置启动画面窗口属性并将 EditorLayer 作为覆盖层推入。',
    tags: ['entry-point', 'application', 'initialization']
  },
  'HimiiEditor/src/panel/AnimationEditorPanel.cpp': {
    summary: '动画编辑器面板实现，提供图集拾取、时间轴帧调整、动画创建/保存和帧预览等完整的 2D 动画编辑工作流。',
    tags: ['animation-editor', 'editor-panel', 'ui', 'asset-editing', 'timeline']
  },
  'HimiiEditor/src/panel/AnimationEditorPanel.h': {
    summary: '动画编辑器面板头文件，声明 AnimationEditorPanel 类及其公共接口，包括上下文设置、动画保存和帧管理方法。',
    tags: ['animation-editor', 'editor-panel', 'header', 'class-definition']
  },
  'HimiiEditor/src/panel/ConsolePanel.cpp': {
    summary: '控制台面板实现，显示引擎运行日志并按日志级别着色输出。',
    tags: ['console', 'editor-panel', 'logging', 'ui']
  },
  'HimiiEditor/src/panel/ConsolePanel.h': {
    summary: '控制台面板头文件，声明 ConsolePanel 类及其 ImGui 渲染接口。',
    tags: ['console', 'editor-panel', 'header']
  },
  'HimiiEditor/src/panel/ContentBrowserPanel.cpp': {
    summary: '内容浏览器面板实现，支持文件树浏览、资源缩略图预览、文件导入、拖放操作以及 C# 脚本创建功能。',
    tags: ['content-browser', 'editor-panel', 'file-management', 'asset-browser', 'ui']
  },
  'HimiiEditor/src/panel/ContentBrowserPanel.h': {
    summary: '内容浏览器面板头文件，声明 ContentBrowserPanel 类，包含文件浏览、刷新和资源导入等公共接口。',
    tags: ['content-browser', 'editor-panel', 'header', 'class-definition']
  },
  'HimiiEditor/src/panel/EditorPreferencesPanel.cpp': {
    summary: '编辑器偏好设置面板实现，提供 IDE 配置（如外部代码编辑器路径）的编辑界面。',
    tags: ['preferences', 'editor-panel', 'settings', 'ide-config']
  },
  'HimiiEditor/src/panel/EditorPreferencesPanel.h': {
    summary: '编辑器偏好设置面板头文件，声明 EditorPreferencesPanel 类。',
    tags: ['preferences', 'editor-panel', 'header']
  },
  'HimiiEditor/src/panel/ParticleEmitterEditorPanel.cpp': {
    summary: '粒子发射器编辑器面板实现，支持粒子资产加载、实时预览、属性编辑和资产保存。',
    tags: ['particle-editor', 'editor-panel', 'asset-editing', 'preview']
  },
  'HimiiEditor/src/panel/ParticleEmitterEditorPanel.h': {
    summary: '粒子发射器编辑器面板头文件，声明 ParticleEmitterEditorPanel 类及其资产管理接口。',
    tags: ['particle-editor', 'editor-panel', 'header']
  },
  'HimiiEditor/src/panel/ProjectSettingsPanel.cpp': {
    summary: '项目设置面板实现，提供项目级配置（如场景路径、脚本程序集路径）的查看和编辑功能。',
    tags: ['project-settings', 'editor-panel', 'configuration']
  },
  'HimiiEditor/src/panel/ProjectSettingsPanel.h': {
    summary: '项目设置面板头文件，声明 ProjectSettingsPanel 类及其同步/应用配置接口。',
    tags: ['project-settings', 'editor-panel', 'header']
  },
  'HimiiEditor/src/panel/SceneHierarchyPanel.cpp': {
    summary: '场景层级面板实现（编辑器核心面板之一），提供场景实体树视图、组件检视器（含 Transform、Sprite、Rigidbody2D、BoxCollider2D、脚本组件等），以及拖放资源分配功能。',
    tags: ['scene-hierarchy', 'editor-panel', 'entity-inspector', 'component-editor', 'drag-and-drop']
  },
  'HimiiEditor/src/panel/SceneHierarchyPanel.h': {
    summary: '场景层级面板头文件，声明 SceneHierarchyPanel 类，包含实体选择、命令历史管理和向其他编辑器面板传递编辑请求的接口。',
    tags: ['scene-hierarchy', 'editor-panel', 'header', 'command-history']
  },
  'HimiiEditor/src/panel/ScriptConsolePanel.cpp': {
    summary: '脚本控制台面板实现，解析 C# 编译器输出中的错误位置信息，并以可点击导航方式显示编译结果。',
    tags: ['script-console', 'editor-panel', 'scripting', 'compilation']
  },
  'HimiiEditor/src/panel/ScriptConsolePanel.h': {
    summary: '脚本控制台面板头文件，声明 ScriptConsolePanel 类。',
    tags: ['script-console', 'editor-panel', 'header']
  },
  'HimiiEditor/src/panel/TextureInspectorPanel.cpp': {
    summary: '纹理检视器面板实现，提供纹理导入设置编辑、Sprite 切片工作流（自动/网格切片）、Sprite 管理（添加/删除），以及实时预览功能。',
    tags: ['texture-inspector', 'editor-panel', 'asset-editing', 'sprite-slicing', 'ui']
  },
  'HimiiEditor/src/panel/TextureInspectorPanel.h': {
    summary: '纹理检视器面板头文件，声明 TextureInspectorPanel 类及其纹理导入、Sprite 管理和元数据保存接口。',
    tags: ['texture-inspector', 'editor-panel', 'header']
  },
  'HimiiEditor/src/panel/TileMapEditorPanel.cpp': {
    summary: 'Tilemap 编辑器面板实现，提供图块地图编辑、图集设置与切片、笔刷绘制、碰撞体编辑、框选工具以及 TileMap/TileSet 资产保存等完整工作流。',
    tags: ['tilemap-editor', 'editor-panel', 'tile-editing', 'atlas', 'ui']
  },
  'HimiiEditor/src/panel/TileMapEditorPanel.h': {
    summary: 'Tilemap 编辑器面板头文件，声明 TileMapEditorPanel 类及其工具系统（笔刷/框选/移动实体）、图块选择和资产保存接口。',
    tags: ['tilemap-editor', 'editor-panel', 'header']
  }
};

for (const r of results) {
  const decl = fileDecls[r.path] || { summary: '代码文件: ' + r.path, tags: ['code'] };
  allNodes.push({
    id: 'file:' + r.path,
    type: 'file',
    name: r.path.split('/').pop(),
    filePath: r.path,
    summary: decl.summary,
    tags: decl.tags,
    complexity: complexity(r.totalLines, (r.functions || []).length)
  });
}

// ============== CLASS NODES ==============

const classDecls = {
  'EditorLayer': {
    summary: '编辑器核心层类，负责管理编辑器完整生命周期：启动画面过渡、面板系统协调、场景编辑、Tilemap 绘制、脚本编译和项目序列化。',
    tags: ['entry-point', 'coordinator', 'lifecycle-management']
  },
  'HimiiApp': {
    summary: '编辑器应用程序类，继承自 Application，负责创建启动窗口并将 EditorLayer 推入层栈。',
    tags: ['application', 'entry-point', 'initialization']
  },
  'AnimationEditorPanel': {
    summary: '动画编辑器面板类，提供动画资产（AnimationAsset）的创建、编辑、保存功能，支持图集帧和时间轴操作。',
    tags: ['animation-editor', 'editor-panel', 'asset-editing']
  },
  'ConsolePanel': {
    summary: '引擎控制台面板类，收集并带颜色分类显示运行时日志消息。',
    tags: ['console', 'editor-panel', 'logging']
  },
  'ContentBrowserPanel': {
    summary: '内容浏览器面板类，管理项目资产目录的文件浏览、导入、C# 脚本创建和缩略图显示。',
    tags: ['content-browser', 'editor-panel', 'file-management']
  },
  'EditorPreferencesPanel': {
    summary: '编辑器偏好设置面板类，用于配置 IDE 路径等编辑器全局偏好。',
    tags: ['preferences', 'editor-panel', 'settings']
  },
  'ParticleEmitterEditorPanel': {
    summary: '粒子发射器编辑器面板类，管理粒子系统资产的加载、实时预览、属性编辑和保存。',
    tags: ['particle-editor', 'editor-panel', 'asset-editing']
  },
  'ProjectSettingsPanel': {
    summary: '项目设置面板类，提供项目级配置的查看界面及配置同步功能。',
    tags: ['project-settings', 'editor-panel', 'configuration']
  },
  'SceneHierarchyPanel': {
    summary: '场景层级面板类，管理场景实体树的显示、实体选择状态、组件检视器渲染以及向其他面板的编辑请求转发。',
    tags: ['scene-hierarchy', 'editor-panel', 'entity-inspector']
  },
  'ScriptConsolePanel': {
    summary: '脚本控制台面板类，解析并展示 C# 脚本编译输出，支持错误位置的点击导航。',
    tags: ['script-console', 'editor-panel', 'compilation']
  },
  'TextureInspectorPanel': {
    summary: '纹理检视器面板类，管理纹理资产的导入设置、Sprite 切片工作流和 Sprite 增删操作。',
    tags: ['texture-inspector', 'editor-panel', 'sprite-management']
  },
  'TileMapEditorPanel': {
    summary: 'Tilemap 编辑器面板类，提供完整的 Tilemap 编辑工具链（笔刷、框选、碰撞编辑和图集切片），管理 TileMap/TileSet 资产的创建与保存。',
    tags: ['tilemap-editor', 'editor-panel', 'tool-system']
  }
};

for (const r of results) {
  for (const cls of (r.classes || [])) {
    const methCount = (cls.methods || []).length;
    const propCount = (cls.properties || []).length;
    const clsLen = cls.endLine - cls.startLine + 1;
    // Skip trivial forward declarations
    if (methCount === 0 && propCount === 0 && clsLen < 3) continue;

    const decl = classDecls[cls.name] || {
      summary: cls.name + ' 类，定义于 ' + r.path + '。',
      tags: ['class-definition']
    };

    allNodes.push({
      id: 'class:' + r.path + ':' + cls.name,
      type: 'class',
      name: cls.name,
      filePath: r.path,
      lineRange: [cls.startLine, cls.endLine],
      summary: decl.summary,
      tags: decl.tags,
      complexity: clsLen > 100 ? 'complex' : clsLen > 40 ? 'moderate' : 'simple'
    });
  }
}

// ============== FUNCTION NODES (10+ line .cpp functions) ==============

function makeFuncSummary(name, path, len) {
  if (name === 'OnImGuiRender') return { summary: 'ImGui 渲染入口，绘制面板的全部 UI 布局和交互元素。', tags: ['ui-render', 'entry-point', 'imgui'] };
  if (name.startsWith('Draw')) return { summary: 'UI 绘制辅助方法，渲染面板的特定子区域。', tags: ['ui-render', 'helper'] };
  if (name.startsWith('UI_')) return { summary: 'ImGui UI 子区域渲染方法。', tags: ['ui-render', 'panel-section'] };
  if (name === 'SaveAnimation' || name === 'SaveAnimationAs' || name === 'SaveActiveAnimationAsset' || name === 'SaveAsset' || name === 'SaveActiveTextureMeta')
    return { summary: '资产序列化并保存到文件的方法。', tags: ['serialization', 'asset-saving'] };
  if (name.startsWith('Save'))
    return { summary: '资产保存方法，将编辑结果序列化写入文件。', tags: ['serialization', 'asset-saving'] };
  if (name === 'SetContext' || name === 'Open') return { summary: '设置面板的当前编辑上下文或打开指定资产。', tags: ['context-management', 'initialization'] };
  if (name === 'Refresh') return { summary: '刷新面板数据或目录结构。', tags: ['refresh', 'ui-update'] };
  if (name === 'DrawComponents') return { summary: '绘制实体组件检视器，按组件类型渲染编辑控件，是场景层级面板的核心编辑界面。', tags: ['component-inspector', 'entity-editing', 'core-logic'] };
  if (name === 'DrawEntityNode') return { summary: '在场景层级树中绘制单个实体节点，支持拖放选择和上下文菜单。', tags: ['entity-tree', 'ui', 'selection'] };
  if (name === 'DrawVec3Control') return { summary: '绘制三维向量编辑控件（XYZ 三通道），用于 Transform 位置/旋转/缩放编辑。', tags: ['vector-control', 'ui-widget', 'transform'] };
  if (name === 'DrawTextureAssignControl') return { summary: '绘制纹理拖放分配控件，支持从内容浏览器拖放纹理到组件字段。', tags: ['drag-and-drop', 'texture-assignment', 'ui-widget'] };
  if (name === 'DrawAtlasPickerRegion') return { summary: '绘制图集拾取器区域，可视化图集网格并支持点击选取单元。', tags: ['atlas-picker', 'ui', 'sprite-selection'] };
  if (name === 'DrawTimelineRegion') return { summary: '绘制动画时间轴区域，显示帧序列并支持拖放排序。', tags: ['timeline', 'ui', 'frame-editing'] };
  if (name === 'DrawRightPreviewPanel' || name === 'DrawAtlasPreviewPanel')
    return { summary: '绘制右侧预览面板，可视化显示纹理/图集内容和交互式操作区。', tags: ['preview', 'ui', 'visualization'] };
  if (name === 'DrawSliceSettings') return { summary: '绘制 Sprite 切片设置界面，支持自动切片和网格切片模式的参数配置。', tags: ['sprite-slicing', 'ui', 'settings'] };
  if (name === 'DrawImportSettings') return { summary: '绘制纹理导入设置界面，管理纹理过滤/包裹模式等导入参数。', tags: ['import-settings', 'ui', 'texture'] };
  if (name === 'CreateCSharpScript' || name === 'CreateNewAnimation')
    return { summary: '创建新资产（C# 脚本或动画）的文件生成方法。', tags: ['asset-creation', 'file-generation'] };
  if (name === 'ImportSingleFile' || name === 'ImportFilesFromPaths' || name === 'ImportTextureAsAtlas')
    return { summary: '文件/资源导入方法，将外部文件复制到项目资产目录。', tags: ['file-import', 'asset-management'] };
  if (name === 'AppendFrameFromPayload') return { summary: '从拖放负载数据向当前动画追加帧。', tags: ['drag-and-drop', 'frame-management', 'animation'] };
  if (name === 'ResolveFramePreview')
    return { summary: '解析并获取动画帧的预览纹理，处理图集帧和帧列表两种模式。', tags: ['frame-preview', 'texture-resolve', 'animation'] };
  if (name === 'ApplyAtlasCellPick' || name === 'ApplyToolAtTile' || name === 'ApplyBoxTool')
    return { summary: '应用图集单元拾取或地图编辑工具到指定位置。', tags: ['tool-application', 'editing'] };
  if (name === 'SliceAtlasGrid' || name === 'FillAtlasGridAllCells')
    return { summary: '图集网格切片方法，将图集纹理按行列切分为子区域。', tags: ['atlas-slicing', 'grid-processing'] };
  if (name === 'UI_Properties' || name === 'PropRow')
    return { summary: '绘制属性编辑 UI，显示和编辑当前选择资产的属性面板。', tags: ['property-editor', 'ui', 'inspector'] };
  if (name === 'UI_AtlasSetup' || name === 'UI_AtlasSource')
    return { summary: '绘制图集设置 UI，配置图集纹理的列数/行数等参数。', tags: ['atlas-config', 'ui'] };
  if (name === 'UI_Toolbar') return { summary: '绘制编辑器工具栏，提供工具选择按钮。', tags: ['toolbar', 'ui', 'tool-selection'] };
  if (name === 'UI_Collision') return { summary: '绘制碰撞体编辑 UI，配置 Tilemap 图块的碰撞形状。', tags: ['collision-editing', 'ui', 'physics'] };
  if (name === 'UI_Overview') return { summary: '绘制资产概览信息 UI，显示当前编辑资产的基本属性。', tags: ['overview', 'ui', 'asset-info'] };
  if (name === 'LoadAssets' || name === 'LoadAsset' || name === 'ReloadImportData')
    return { summary: '加载/重载编辑所需的纹理和资产数据。', tags: ['asset-loading', 'data-reload'] };
  if (name === 'SyncFromProject' || name === 'ApplyToProject' || name === 'SyncUIToImportData' || name === 'SyncPendingEditsToMemory')
    return { summary: '同步方法，在 UI 和底层数据模型之间同步状态。', tags: ['data-sync', 'state-management'] };
  if (name === 'AddNewSprite') return { summary: '向当前纹理添加新的 Sprite 定义。', tags: ['sprite-management', 'asset-editing'] };
  if (name === 'DeleteSelectedSprite') return { summary: '删除当前选中的 Sprite 定义。', tags: ['sprite-management', 'asset-editing'] };
  if (name === 'FindSpriteIndexAtPixel') return { summary: '根据像素坐标查找对应的 Sprite 索引，用于点击选择 Sprite。', tags: ['sprite-lookup', 'hit-testing'] };
  if (name === 'FindTileIDAtAtlasCell') return { summary: '在图集中查找指定行列位置的 Tile ID。', tags: ['tile-lookup', 'atlas'] };
  if (name === 'MoveSelectedTimelineFrame') return { summary: '在时间轴中移动选中的帧。', tags: ['timeline', 'frame-manipulation'] };
  if (name === 'SelectAnimationClip') return { summary: '选择指定的动画剪辑作为当前编辑目标。', tags: ['animation', 'selection'] };
  if (name === 'DrawFramePreview' || name === 'DrawSelectedSpriteThumbnail' || name === 'DrawSelectedTileThumbnail')
    return { summary: '绘制帧/精灵/图块的缩略图预览。', tags: ['thumbnail', 'preview', 'ui'] };
  if (name === 'GetOrLoadImageThumbnail')
    return { summary: '获取或加载图片资源缩略图，带缓存机制。', tags: ['thumbnail', 'image-cache', 'asset-loading'] };
  if (name === 'DrawAspectFitThumbnail') return { summary: '以保持宽高比的方式绘制纹理缩略图。', tags: ['thumbnail', 'ui-render'] };
  if (name === 'TruncateTextToWidth') return { summary: '按像素宽度截断文本，用于在有限空间内显示文件名。', tags: ['text-processing', 'ui-helper'] };
  if (name === 'IsOnPathToCurrentDirectory') return { summary: '判断给定路径是否为当前目录的祖先路径。', tags: ['path-utility', 'validation'] };
  if (name === 'DrawContentDetailBar') return { summary: '绘制内容浏览器的底部详情栏，显示选中项信息。', tags: ['ui', 'detail-bar'] };
  if (name === 'DrawTree') return { summary: '递归绘制文件系统目录树节点。', tags: ['tree-view', 'ui', 'recursion'] };
  if (name === 'ResolveUniqueDestination') return { summary: '为导入文件解析不重复的目标路径，自动处理命名冲突。', tags: ['file-resolution', 'path-utility'] };
  if (name === 'DrawIDEConfig') return { summary: '绘制 IDE 配置 UI（外部代码编辑器路径设置）。', tags: ['ide-config', 'ui', 'settings'] };
  if (name === 'RenderPreview' || name === 'UpdatePreview')
    return { summary: '渲染/更新粒子系统的实时预览。', tags: ['particle-preview', 'rendering'] };
  if (name === 'TryParseCompilerLocation')
    return { summary: '解析 C# 编译器输出行，提取文件路径和行号用于错误导航。', tags: ['parsing', 'compiler-output', 'error-navigation'] };
  if (name === 'GetLogColor') return { summary: '根据日志级别返回对应的显示颜色。', tags: ['logging', 'color-mapping'] };
  if (name === 'SetTextureHandle' || name === 'GetTextureFromHandle')
    return { summary: '纹理句柄管理方法。', tags: ['texture-management', 'handle'] };
  // fallback for helpers
  return { summary: '面板辅助方法，定义于 ' + path + '。', tags: ['helper', 'utility'] };
}

for (const r of results) {
  if (!r.path.endsWith('.cpp')) continue;
  for (const func of (r.functions || [])) {
    const funcLen = func.endLine - func.startLine + 1;
    if (funcLen < 10) continue;
    const meta = makeFuncSummary(func.name, r.path, funcLen);
    allNodes.push({
      id: 'function:' + r.path + ':' + func.name,
      type: 'function',
      name: func.name,
      filePath: r.path,
      lineRange: [func.startLine, func.endLine],
      summary: meta.summary,
      tags: meta.tags,
      complexity: funcLen > 50 ? 'complex' : funcLen > 20 ? 'moderate' : 'simple'
    });
  }
}

// ============== EDGES ==============

// 1. Import edges (1:1 from batchImportData)
for (const [filePath, imports] of Object.entries(batchImportData)) {
  for (const targetPath of imports) {
    allEdges.push({
      source: 'file:' + filePath,
      target: 'file:' + targetPath,
      type: 'imports',
      direction: 'forward',
      weight: 0.7
    });
  }
}

// 2. Contains edges for every function/class node
for (const node of allNodes) {
  if (node.type === 'function' || node.type === 'class') {
    allEdges.push({
      source: 'file:' + node.filePath,
      target: node.id,
      type: 'contains',
      direction: 'forward',
      weight: 1.0
    });
  }
}

// 2b. Exports edges for exported functions/classes (in addition to contains)
for (const r of results) {
  const exportedNames = new Set((r.exports || []).map(exp => exp.name));
  for (const node of allNodes) {
    if ((node.type === 'function' || node.type === 'class') && node.filePath === r.path) {
      if (exportedNames.has(node.name)) {
        allEdges.push({
          source: 'file:' + r.path,
          target: node.id,
          type: 'exports',
          direction: 'forward',
          weight: 0.8
        });
      }
    }
  }
}

// 3. Calls edges from call graphs (project-internal only)
// Collect all known names for fast lookup
const knownSymbols = new Set();
const knownFiles = new Set();
for (const n of allNodes) {
  knownSymbols.add(n.name);
  if (n.filePath) knownFiles.add(n.filePath);
}

// Build calls edges from the extracted call graph
for (const r of results) {
  const cg = r.callGraph;
  if (!cg) continue;
  for (const call of cg) {
    const calleeName = call.callee;
    // Only emit if callee is a project-internal function
    // Check if we have a function node for this callee somewhere in our nodes
    const calleeNodes = allNodes.filter(n => n.type === 'function' && n.name === calleeName);
    for (const cn of calleeNodes) {
      // Don't self-reference
      if (cn.filePath === r.path) {
        // Internal calls within same file - find the caller node
        const callerNode = allNodes.find(n => n.type === 'function' && n.filePath === r.path && n.name === call.caller);
        if (callerNode && callerNode.id !== cn.id) {
          allEdges.push({
            source: callerNode.id,
            target: cn.id,
            type: 'calls',
            direction: 'forward',
            weight: 0.8
          });
        }
      }
    }
  }
}

// Deduplicate edges
const edgeSet = new Set();
const dedupedEdges = [];
for (const e of allEdges) {
  const key = e.source + '|' + e.target + '|' + e.type;
  if (!edgeSet.has(key)) {
    edgeSet.add(key);
    dedupedEdges.push(e);
  }
}
allEdges = dedupedEdges;

console.log('Nodes:', allNodes.length);
console.log('Edges:', allEdges.length);
const exportCount = Object.values(batchImportData).reduce((s, a) => s + a.length, 0);
console.log('Import edges (expected):', exportCount);

// Determine split
const parts = Math.ceil(Math.max(allNodes.length / 60, allEdges.length / 120));
console.log('Parts:', parts);

// Sort paths alphabetically
const sortedPaths = results.map(r => r.path).sort();
const filesPerPart = Math.ceil(sortedPaths.length / parts);
console.log('Files per part:', filesPerPart);

// Write full data
fs.writeFileSync('D:/Himii-Engine/.understand-anything/tmp/ua-batch2-all-graph.json',
  JSON.stringify({ nodes: allNodes, edges: allEdges, sortedPaths, parts, filesPerPart }, null, 2));
console.log('Done.');
