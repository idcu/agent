# IDCU Agent 模块创建脚本 (PowerShell)
param(
    [Parameter(Mandatory=$true)]
    [string]$ModuleName,
    
    [Parameter(Mandatory=$true)]
    [string]$ModuleDir,
    
    [Parameter(Mandatory=$true)]
    [string]$TargetDir,
    
    [Parameter(Mandatory=$true)]
    [string]$FullTargetDir,
    
    [Parameter(Mandatory=$true)]
    [string]$TemplateDir
)

$ErrorActionPreference = "Stop"

# 转换为大写
$ModuleDirUpper = $ModuleDir.ToUpper()

Write-Host "Processing template files..."

# 模板替换函数
function Replace-Template {
    param(
        [string]$SourcePath,
        [string]$DestPath
    )
    
    $content = Get-Content -Path $SourcePath -Raw -Encoding UTF8
    $content = $content -replace '@MODULE_NAME@', $ModuleName
    $content = $content -replace '@MODULE_DIR@', $ModuleDir
    $content = $content -replace '@MODULE_DIR_UPPER@', $ModuleDirUpper
    $content = $content -replace '@DEPENDENCIES@', ''
    $content = $content -replace '@DEPENDENCIES_CMAKE@', ''
    $content = $content -replace '@DEPENDENCIES_JSON@', ''
    
    Set-Content -Path $DestPath -Value $content -Encoding UTF8 -NoNewline
    Write-Host "  Created: $DestPath"
}

# 生成文件
Replace-Template -SourcePath (Join-Path $TemplateDir "CMakeLists.txt.template") -DestPath (Join-Path $FullTargetDir "CMakeLists.txt")
Replace-Template -SourcePath (Join-Path $TemplateDir "README.md.template") -DestPath (Join-Path $FullTargetDir "README.md")
Replace-Template -SourcePath (Join-Path $TemplateDir "src\module.c.template") -DestPath (Join-Path $FullTargetDir "src\$ModuleDir.c")
Replace-Template -SourcePath (Join-Path $TemplateDir "include\idcu\module\module.h.template") -DestPath (Join-Path $FullTargetDir "include\idcu\$ModuleDir\$ModuleDir.h")
Replace-Template -SourcePath (Join-Path $TemplateDir "tests\CMakeLists.txt.template") -DestPath (Join-Path $FullTargetDir "tests\CMakeLists.txt")
Replace-Template -SourcePath (Join-Path $TemplateDir "tests\test_module.c.template") -DestPath (Join-Path $FullTargetDir "tests\test_$ModuleDir.c")
Replace-Template -SourcePath (Join-Path $TemplateDir "module.json.template") -DestPath (Join-Path $FullTargetDir "module.json")

Write-Host "All files generated successfully!"
