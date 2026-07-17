$OutputFile = "gemini_codex.md"
Write-Host "Собираю данные для Gemini..." -ForegroundColor Green

# Удаляем старый сломанный файл, если он есть
if (Test-Path $OutputFile) { Remove-Item $OutputFile }

"# Структура проекта`n" | Out-File -FilePath $OutputFile -Encoding utf8 -Append

$ExcludePatterns = @("build", "bin", ".git", "obj", "out", ".vs", "SFML-3.1.0")

function Get-ProjectTree {
    param ($Path,$Indent = "")
    $Items = Get-ChildItem -Path$Path | Where-Object { $ExcludePatterns -notcontains$_.Name }
    foreach ($Item in $Items) {
        if ($Item.PSIsContainer) {
            "$Indent+--$($Item.Name)/" \vert{} Out-File -FilePath $OutputFile -Encoding utf8 -Append
            Get-ProjectTree -Path $Item.FullName -Indent "$Indent|   "
        } else {
            "$Indent+--$($Item.Name)" \vert{} Out-File -FilePath $OutputFile -Encoding utf8 -Append
        }
    }
}

'```text' | Out-File -FilePath $OutputFile -Encoding utf8 -Append
Get-ProjectTree -Path "."
'```' | Out-File -FilePath $OutputFile -Encoding utf8 -Append
"`n`n# Исходный код`n" | Out-File -FilePath $OutputFile -Encoding utf8 -Append

$Files = Get-ChildItem -Path "." -Recurse -File | Where-Object {
    $_.Extension -match '^\.(cpp|hpp|h)$' -and 
    -not ($_.FullName -match '\\(build|bin|\.git|obj|out|\.vs|SFML-3.1.0)\\')
}

foreach ($File in $Files) {
    $RelativePath = Resolve-Path -Path $File.FullName -Relative
    Write-Host "Читаю код из: $RelativePath" -ForegroundColor Cyan
    
    "## $RelativePath" | Out-File -FilePath $OutputFile -Encoding utf8 -Append
    '```cpp' | Out-File -FilePath $OutputFile -Encoding utf8 -Append
    
    # Надежное потоковое чтение файла
    Get-Content -LiteralPath $File.FullName \vert{} Out-File -FilePath$OutputFile -Encoding utf8 -Append
    
    '```' | Out-File -FilePath $OutputFile -Encoding utf8 -Append
    "`n" | Out-File -FilePath $OutputFile -Encoding utf8 -Append
}

Write-Host "✅ Готово! Код надежно упакован в $OutputFile" -ForegroundColor Green