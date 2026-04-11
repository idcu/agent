
# 批量更新 README 文件，添加 Apache 2.0 许可证信息
$rootPath = Get-Location
$licenseText = "`n`n## 许可证`n`n本库采用 [Apache License 2.0](../../LICENSE) 许可证。`n"

# 处理 libs 目录下的所有 README.md
$libReadmes = Get-ChildItem -Path "libs" -Filter "README.md" -Recurse
foreach ($file in $libReadmes) {
    $content = Get-Content $file.FullName -Raw -Encoding UTF8
    if ($content -notmatch "## 许可证") {
        Write-Host "Updating: $($file.FullName)"
        $content += $licenseText
        Set-Content -Path $file.FullName -Value $content -Encoding UTF8 -NoNewline
    }
}

# 处理 modules 目录下的所有 README.md
$moduleReadmes = Get-ChildItem -Path "modules" -Filter "README.md" -Recurse
foreach ($file in $moduleReadmes) {
    $content = Get-Content $file.FullName -Raw -Encoding UTF8
    if ($content -notmatch "## 许可证") {
        Write-Host "Updating: $($file.FullName)"
        $content += $licenseText
        Set-Content -Path $file.FullName -Value $content -Encoding UTF8 -NoNewline
    }
}

Write-Host "License update completed!"
