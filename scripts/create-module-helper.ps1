# IDCU Agent 模块创建辅助脚本 (PowerShell)

function Create-Module {
    $scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
    $projectRoot = Join-Path $scriptDir ".."
    $templateDir = Join-Path $projectRoot "templates\module"

    # 获取环境变量
    $moduleName = $env:MODULE_NAME
    $moduleDir = $env:MODULE_DIR
    $moduleDirUpper = $env:MODULE_DIR_UPPER
    $targetDir = $env:TARGET_DIR
    $fullTargetDir = $env:FULL_TARGET_DIR
    $dependencies = $env:DEPENDENCIES
    $dependenciesCmake = $env:DEPENDENCIES_CMAKE
    $dependenciesJson = $env:DEPENDENCIES_JSON

    # 确保变量不为空
    if (-not $moduleName -or -not $moduleDir) {
        Write-Error "环境变量未正确设置"
        exit 1
    }

    # 模板替换函数
    function Replace-Template {
        param(
            [string]$SourcePath,
            [string]$DestPath
        )
        
        $content = Get-Content -Path $SourcePath -Raw -Encoding UTF8
        $content = $content -replace '@MODULE_NAME@', $moduleName
        $content = $content -replace '@MODULE_DIR@', $moduleDir
        $content = $content -replace '@MODULE_DIR_UPPER@', $moduleDirUpper.ToUpper()
        $content = $content -replace '@DEPENDENCIES@', $dependencies
        $content = $content -replace '@DEPENDENCIES_CMAKE@', $dependenciesCmake
        $content = $content -replace '@DEPENDENCIES_JSON@', $dependenciesJson
        
        Set-Content -Path $DestPath -Value $content -Encoding UTF8 -NoNewline
    }

    # 生成文件
    Replace-Template -SourcePath (Join-Path $templateDir "CMakeLists.txt.template") -DestPath (Join-Path $fullTargetDir "CMakeLists.txt")
    Replace-Template -SourcePath (Join-Path $templateDir "README.md.template") -DestPath (Join-Path $fullTargetDir "README.md")
    Replace-Template -SourcePath (Join-Path $templateDir "src\module.c.template") -DestPath (Join-Path $fullTargetDir "src\$moduleDir.c")
    Replace-Template -SourcePath (Join-Path $templateDir "include\idcu\module\module.h.template") -DestPath (Join-Path $fullTargetDir "include\idcu\$moduleDir\$moduleDir.h")
    Replace-Template -SourcePath (Join-Path $templateDir "tests\CMakeLists.txt.template") -DestPath (Join-Path $fullTargetDir "tests\CMakeLists.txt")
    Replace-Template -SourcePath (Join-Path $templateDir "tests\test_module.c.template") -DestPath (Join-Path $fullTargetDir "tests\test_$moduleDir.c")
    Replace-Template -SourcePath (Join-Path $templateDir "module.json.template") -DestPath (Join-Path $fullTargetDir "module.json")
}

# 导出函数
Export-ModuleMember -Function Create-Module
