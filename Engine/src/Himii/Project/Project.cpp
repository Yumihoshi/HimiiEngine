#include "Project.h"
#include "Hepch.h"
#include "Himii/Core/Application.h"

#include "ProjectSerializer.h"

namespace Himii
{
    namespace
    {
        std::filesystem::path ResolveScriptCoreAssemblyPath()
        {
            const std::filesystem::path executableDirectory = Application::Get().GetExecutableDir();
            std::filesystem::path scriptCoreAssemblyPath = executableDirectory / "ScriptCore.dll";

            if (std::filesystem::exists(scriptCoreAssemblyPath))
                return scriptCoreAssemblyPath;

            const std::filesystem::path engineDirectory = Application::GetEngineDir();
#ifdef HIMII_DEBUG
            scriptCoreAssemblyPath = engineDirectory / "ScriptCore/bin/Debug/net8.0/ScriptCore.dll";
#else
            scriptCoreAssemblyPath = engineDirectory / "ScriptCore/bin/Release/net8.0/ScriptCore.dll";
#endif
            return scriptCoreAssemblyPath;
        }

        void CopyIfExists(const std::filesystem::path &sourcePath, const std::filesystem::path &destinationPath)
        {
            if (!std::filesystem::exists(sourcePath))
                return;

            std::error_code errorCode;
            std::filesystem::copy_file(sourcePath, destinationPath,
                                       std::filesystem::copy_options::overwrite_existing, errorCode);
            if (errorCode)
                HIMII_CORE_WARNING("Failed to copy {0}: {1}", sourcePath.string(), errorCode.message());
        }

        void CopyScriptingApiSourcesToProject(const std::filesystem::path &projectDir)
        {
            const std::filesystem::path engineDirectory = Application::GetEngineDir();
            const std::filesystem::path apiSourceDirectory = engineDirectory / "Packaging/ScriptingApi/Himii";
            if (!std::filesystem::exists(apiSourceDirectory))
            {
                HIMII_CORE_WARNING("Scripting API sources not found at {0}", apiSourceDirectory.string());
                return;
            }

            const std::filesystem::path destinationDirectory = projectDir / "assets" / "scripts" / "Himii";
            std::error_code errorCode;
            std::filesystem::create_directories(destinationDirectory, errorCode);
            if (errorCode)
            {
                HIMII_CORE_WARNING("Failed to create {0}: {1}", destinationDirectory.string(), errorCode.message());
                return;
            }

            for (const auto &entry : std::filesystem::directory_iterator(apiSourceDirectory))
            {
                if (!entry.is_regular_file() || entry.path().extension() != ".cs")
                    continue;

                CopyIfExists(entry.path(), destinationDirectory / entry.path().filename());
            }
        }
    }

    void Project::SyncScriptCoreToProjectDirectory(const std::filesystem::path &projectDir)
    {
        const std::filesystem::path scriptCoreAssemblyPath = ResolveScriptCoreAssemblyPath();
        if (!std::filesystem::exists(scriptCoreAssemblyPath))
        {
            HIMII_CORE_WARNING("ScriptCore.dll not found, IDE may not resolve Himii API types.");
            return;
        }

        CopyIfExists(scriptCoreAssemblyPath, projectDir / "ScriptCore.dll");

        std::filesystem::path programDatabasePath = scriptCoreAssemblyPath;
        programDatabasePath.replace_extension(".pdb");
        CopyIfExists(programDatabasePath, projectDir / "ScriptCore.pdb");

        std::filesystem::path documentationPath = scriptCoreAssemblyPath;
        documentationPath.replace_extension(".xml");
        CopyIfExists(documentationPath, projectDir / "ScriptCore.xml");

        CopyScriptingApiSourcesToProject(projectDir);
    }

    Ref<Project> Project::New()
    {
        s_ActiveProject = CreateRef<Project>();
        s_ActiveProject->m_AssetManager = CreateRef<AssetManager>();
        s_ActiveProject->m_AssetManager->DeserializeAssetRegistry();
        return s_ActiveProject;
    }

    Ref<Project> Project::Load(const std::filesystem::path &path)
    {
        Ref<Project> project = CreateRef<Project>();
        project->m_AssetManager = CreateRef<AssetManager>();
        ProjectSerializer serializer(project);
        if (serializer.Deserialize(path))
        {
            project->m_ProjectDirectory = path.parent_path();
            s_ActiveProject = project;
            s_ActiveProject->m_AssetManager->DeserializeAssetRegistry();
            return s_ActiveProject;
        }

        return nullptr;
    }

    bool Project::SaveActive(const std::filesystem::path &path)
    {
        ProjectSerializer serializer(s_ActiveProject);
        if (serializer.Serialize(path))
        {
            s_ActiveProject->m_ProjectDirectory = path.parent_path();
            if (s_ActiveProject->m_AssetManager)
                s_ActiveProject->m_AssetManager->SerializeAssetRegistry();
            return true;
        }

        return false;
    }

    void Project::CreateProjectFiles(const std::string &name, const std::filesystem::path &projectDir)
    {
        if (!std::filesystem::exists(projectDir))
            std::filesystem::create_directories(projectDir);

        // 1. 创建标准目录结构
        std::filesystem::create_directories(projectDir / "assets" / "scenes");
        std::filesystem::create_directories(projectDir / "assets" / "scripts");
        std::filesystem::create_directories(projectDir / "assets" / "textures");

        SyncScriptCoreToProjectDirectory(projectDir);

        // 2. 自动生成 GameAssembly.csproj（引用项目根目录下的 ScriptCore.dll，便于 IDE 解析）
        std::stringstream ss;
        ss << R"(<Project Sdk="Microsoft.NET.Sdk">
  <PropertyGroup>
    <TargetFramework>net8.0</TargetFramework>
    <ImplicitUsings>enable</ImplicitUsings>
    <Nullable>enable</Nullable>
    <OutputPath>bin\$(Configuration)</OutputPath>
    <AppendTargetFrameworkToOutputPath>false</AppendTargetFrameworkToOutputPath>
    <EnableDefaultCompileItems>false</EnableDefaultCompileItems>
  </PropertyGroup>

  <ItemGroup>
    <Using Include="Himii" />
  </ItemGroup>

  <ItemGroup>
    <Compile Include="assets\scripts\**\*.cs" />
  </ItemGroup>

  <ItemGroup>
    <Reference Include="ScriptCore">
      <HintPath>ScriptCore.dll</HintPath>
      <Private>false</Private>
    </Reference>
  </ItemGroup>
</Project>
)";

        std::ofstream csprojFile(projectDir / "GameAssembly.csproj");
        csprojFile << ss.str();
        csprojFile.close();

        const std::filesystem::path defaultScriptPath = projectDir / "assets" / "scripts" / "SampleScript.cs";
        if (!std::filesystem::exists(defaultScriptPath))
        {
            std::ofstream scriptFile(defaultScriptPath);
            if (scriptFile.is_open())
            {
                scriptFile << "public class SampleScript : Entity\n";
                scriptFile << "{\n";
                scriptFile << "    [SerializeField]\n";
                scriptFile << "    private float moveSpeed = 5.0f;\n\n";
                scriptFile << "    public override void OnUpdate(float timestep)\n";
                scriptFile << "    {\n";
                scriptFile << "        // var delta = Time.DeltaTime;\n";
                scriptFile << "    }\n";
                scriptFile << "}\n";
            }
        }

        // 3. (可选) 生成一个空的默认场景，防止 StartScene 报错
        // 这一步比较复杂，需要 SceneSerializer 支持保存空场景，暂时跳过

        HIMII_CORE_INFO("Created new project structure at {0}", projectDir.string());

        std::stringstream ss1;

        // SLN 文件头
        ss1 << "Microsoft Visual Studio Solution File, Format Version 12.00\n";
        ss1 << "# Visual Studio Version 17\n";
        ss1 << "VisualStudioVersion = 17.0.31903.59\n";
        ss1 << "MinimumVisualStudioVersion = 10.0.40219.1\n";

        // 1. 定义 GameAssembly 项目
        // GUID 可以是随机生成的，这里为了演示暂时硬编码 (但在实际引擎中最好动态生成)
        std::string gameAssemblyGUID = "{52962852-2567-41C2-B358-132717009043}";
        ss1 << "Project(\"{9A19103F-16F7-4668-BE54-9A1E7A4F7556}\") = \"GameAssembly\", \"GameAssembly.csproj\", \""
           << gameAssemblyGUID << "\"\n";
        ss1 << "EndProject\n";

        // 3. 定义全局配置 (Debug/Release)
        ss1 << "Global\n";
        ss1 << "\tGlobalSection(SolutionConfigurationPlatforms) = preSolution\n";
        ss1 << "\t\tDebug|Any CPU = Debug|Any CPU\n";
        ss1 << "\t\tRelease|Any CPU = Release|Any CPU\n";
        ss1 << "\tEndGlobalSection\n";

        // 4. 关联项目配置
        ss1 << "\tGlobalSection(ProjectConfigurationPlatforms) = postSolution\n";
        // GameAssembly
        ss1 << "\t\t" << gameAssemblyGUID << ".Debug|Any CPU.ActiveCfg = Debug|Any CPU\n";
        ss1 << "\t\t" << gameAssemblyGUID << ".Debug|Any CPU.Build.0 = Debug|Any CPU\n";
        ss1 << "\t\t" << gameAssemblyGUID << ".Release|Any CPU.ActiveCfg = Release|Any CPU\n";
        ss1 << "\t\t" << gameAssemblyGUID << ".Release|Any CPU.Build.0 = Release|Any CPU\n";
        
        ss1 << "EndGlobal\n";

        // 写入 .sln 文件 (文件名通常和项目名一致，例如 Sandbox.sln)
        std::ofstream slnFile(projectDir / (name + ".sln"));
        if (slnFile.is_open())
        {
            slnFile << ss1.str();
            slnFile.close();
        }
    }

} // namespace Himii
